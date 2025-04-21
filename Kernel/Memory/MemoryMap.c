#include <limine.h>
#include "Logging.h"
#include "Panic.h"

#include "Memory/MemoryMap.h"
#define MODULE "Memory map"

// Used in limine's memory map, when parsing, to mark an entry as "removed"
#define REMOVED_ENTRY_LIMINE 574213

struct MemoryMap g_memoryMap;

// Parse the map to initialize some `mmap` members
static void parseLimineMemoryMap(struct MemoryMap* mmap, struct limine_memmap_response* limine_mmap){
	mmap->totalMemory = 0;
	bool firstNotFound = true;
	mmap->firstUsablePage = 0;
	mmap->lastUsablePage = 0;
	uint64_t temp;

	for (uint64_t i=0 ; i<limine_mmap->entry_count ; i++){
		struct limine_memmap_entry* cur = limine_mmap->entries[i];

		switch (cur->type){
		case LIMINE_MEMMAP_USABLE:
			mmap->totalMemory += cur->length;
			mmap->lastUsablePage = cur->base + cur->length - PAGE_SIZE; // Last page of the region
			if (firstNotFound){
				mmap->firstUsablePage = cur->base;
				firstNotFound = false;
			}
			break;
		case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			// These are potentially usable memory.
			// Limine does not guarantee no overlap with other entries
			// Note: we check for overlap with the next entry, because
			// checking for the privous entry was done in the previous iteration
			temp = cur->length;
			if ( (i < limine_mmap->entry_count-1) && (cur->base+cur->length >= limine_mmap->entries[i+1]->base) )
				temp = limine_mmap->entries[i+1]->base - cur->base;
			mmap->totalMemory += temp;
			break;
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			mmap->totalMemory += cur->length;
			break;
		case LIMINE_MEMMAP_RESERVED:
		case LIMINE_MEMMAP_ACPI_NVS:
		case LIMINE_MEMMAP_BAD_MEMORY:
		case LIMINE_MEMMAP_FRAMEBUFFER:
		case REMOVED_ENTRY_LIMINE:
			break;
		default:
			log(PANIC, MODULE, "Unsupported Limine memory type %d", cur->type);
			panic();
		}
	}

	if (mmap->totalMemory == 0){
		log(PANIC, MODULE, "No usable physical memory available !!");
		panic();
	}
}

/// @brief Process Limine's memory map by squashing sequential entries of same type
/// @returns The number of entries in the processed memory map
static int processLimineMemoryMap(struct limine_memmap_response* limine_mmmap){
	if (limine_mmmap->entry_count == 0) return 0;
	int n_entries = 1; // last entry

	for (uint64_t i=0 ; i<limine_mmmap->entry_count-1 ; i++){
		struct limine_memmap_entry* cur = limine_mmmap->entries[i];
		struct limine_memmap_entry* next = limine_mmmap->entries[i+1];
		n_entries++;

		// If two consecutives entries are juxtaposed, merge them
		if (cur->type == next->type && cur->base+cur->length == next->base){
			cur->length += next->length;
			next->type = REMOVED_ENTRY_LIMINE;
			n_entries--; // remove this entry, that we counted and removed
		}
	}

	return n_entries;
}

static inline enum MemoryType getMemoryType(uint64_t limineMemoryType){
	switch (limineMemoryType){
	case LIMINE_MEMMAP_USABLE:
		return MEMORY_USABLE;
	case LIMINE_MEMMAP_RESERVED:
		return MEMORY_RESERVED;
	case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
		return MEMORY_ACPI_RECLAIMABLE;
	case LIMINE_MEMMAP_ACPI_NVS:
		return MEMORY_ACPI_NVS;
	case LIMINE_MEMMAP_BAD_MEMORY:
		return MEMORY_RESERVED;
	case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
		return MEMORY_BOOTLOADER_RECLAIMABLE;
	case LIMINE_MEMMAP_KERNEL_AND_MODULES: // or LIMINE_MEMMAP_EXECUTABLE_AND_MODULES
		return MEMORY_KERNEL;
	case LIMINE_MEMMAP_FRAMEBUFFER:
		return MEMORY_FRAMEBUFFER;
	default:
		log(PANIC, MODULE, "Unsupported limine memory type %d !!", limineMemoryType);
		panic();
		unreachable();
	}
}

static void printMemoryMap(struct MemoryMap* memmap){
	static const char* memoryTypeNames[] = {
		"MEMORY_USABLE",
		"MEMORY_RESERVED",
		"MEMORY_KERNEL",
		"MEMORY_FRAMEBUFFER",
		"MEMORY_ACPI_NVS",
		"MEMORY_ACPI_RECLAIMABLE",
		"MEMORY_BOOTLOADER_RECLAIMABLE",
	};

	log(INFO, MODULE, "@Phys start        @Phys end                       Type");
	for (int i=0 ; i<memmap->size ; i++){
		struct MemoryMapEntry* cur = memmap->entries + i;

		int magnitude = getMagnitude(cur->length);
		magnitude = (magnitude > 6) ? 6 : magnitude; // clamp magnitude to the max prefix available on 64 bits
		uint64_t divisor = (1llu << (10*magnitude)); // 1024**magnitude = 2**(10*magnitude)

		log(INFO, MODULE, "%#.16llx %#.16llx (%4d %s) - %s ",
			cur->address, cur->address + cur->length - 1, cur->length/divisor, SIZE_UNITS[magnitude],
			memoryTypeNames[cur->type]);
	}
}

void MMap_initialize(struct MemoryMap* memmap, struct limine_memmap_response* limine_mmap){
	parseLimineMemoryMap(memmap, limine_mmap);
	memmap->size = processLimineMemoryMap(limine_mmap);

	if (memmap->size >= MMAP_MAX_ENTRIES){
		log(PANIC, MODULE, "Memory map is too big to fit in anticipated buffer !");
		log(PANIC, MODULE, "Please recompile with MMAP_MAX_ENTRIES > %d", memmap->size);
		log(PANIC, MODULE, "Note: current value is %d", MMAP_MAX_ENTRIES);
		panic();
	}

	// Copy limine's memory map into mugOS's memory map

	int memap_index = 0; // in mugOS memmap
	for(uint64_t i=0 ; i<limine_mmap->entry_count ; i++){
		struct limine_memmap_entry* cur = limine_mmap->entries[i];

		if (limine_mmap->entries[i]->type == REMOVED_ENTRY_LIMINE)
			continue;

		// Copy entry
		g_memoryMap.entries[memap_index].address = cur->base;
		g_memoryMap.entries[memap_index].length = cur->length;
		g_memoryMap.entries[memap_index].type = getMemoryType(cur->type);
		memap_index++;
	}

	printMemoryMap(memmap);
}

// ================ Display number in powers of two ================

int getMagnitude(uint64_t byte_number){
    int i = 0;
    while (byte_number >= 1024){
        byte_number /= 1024;
        i++;
    }
    return i;
}
