#include <limine.h>
#include "assert.h"
#include "Logging.h"
#include "Panic.h"

#include "Memory/MemoryMap.h"
#define MODULE "Memory map"

// Used in limine's memory map, when parsing, to mark an entry as "removed"
#define REMOVED_ENTRY_LIMINE 574213

struct MemoryMap g_memoryMap;

// Parse the map to initialize some `mmap` members
static void parseLimineMemoryMap(struct MemoryMap* mmap, struct limine_memmap_response* limine_mmap){
	mmap->totalUsableMemory = 0;
	bool firstNotFound = true;
	mmap->firstUsablePage = 0;
	mmap->lastUsablePage = 0;
	uint64_t temp;
	struct limine_memmap_entry* cur;

	for (uint64_t i=0 ; i<limine_mmap->entry_count ; i++){
		cur = limine_mmap->entries[i];

		switch (cur->type){
		case LIMINE_MEMMAP_USABLE:
			mmap->totalUsableMemory += cur->length;
			mmap->lastUsablePage = cur->base + cur->length - PAGE_SIZE; // Last page of the region
			// First entry: do not add the first page (at physical address 0) in usable regions
			if (cur->base == 0){
				mmap->totalUsableMemory -= PAGE_SIZE;
				if (firstNotFound){
					mmap->firstUsablePage = PAGE_SIZE;
					firstNotFound = false;
				}
			}
			if (firstNotFound){
				mmap->firstUsablePage = cur->base;
				firstNotFound = false;
			}
			break;
		case LIMINE_MEMMAP_RESERVED:
			break;
		case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			// These are potentially usable memory.
			// Limine does not guarantee no overlap with other entries
			// Note: we check for overlap with the next entry, because
			// checking for the privous entry was done in the previous iteration
			temp = cur->length;
			if ( (i < limine_mmap->entry_count-1) && (cur->base+cur->length >= limine_mmap->entries[i+1]->base) )
				temp = limine_mmap->entries[i+1]->base - cur->base;
			mmap->totalUsableMemory += temp;
			mmap->lastUsablePage = cur->base + cur->length - PAGE_SIZE; // Last page of the region
			if (firstNotFound){
				mmap->firstUsablePage = cur->base;
				firstNotFound = false;
			}
			break;
		case LIMINE_MEMMAP_ACPI_NVS:
			break;
		case LIMINE_MEMMAP_BAD_MEMORY:
			break;
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			mmap->totalUsableMemory += cur->length;
			mmap->lastUsablePage = cur->base + cur->length - PAGE_SIZE; // Last page of the region
			if (firstNotFound){
				mmap->firstUsablePage = cur->base;
				firstNotFound = false;
			}
			break;
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			mmap->kernelAddress = cur->base;
			mmap->kernelSize = cur->length;
			break;
		case LIMINE_MEMMAP_FRAMEBUFFER:
			break;
		case REMOVED_ENTRY_LIMINE:
			break;
		default:
			log(PANIC, MODULE, "Unsupported Limine memory type %d", cur->type);
			panic();
		}
	}
}

/// @brief Process Limine's memory map by squashing sequential entries of same type
/// @returns The number of entries in the processed memory map
static int processLimineMemoryMap(struct limine_memmap_response* limine_memap){
	if (limine_memap->entry_count == 0) return 0;

	struct limine_memmap_entry* cur = limine_memap->entries[0];
	struct limine_memmap_entry* next;

	int n_entries = limine_memap->entry_count;
	if (cur->type == LIMINE_MEMMAP_USABLE && cur->base == 0 && cur->length > PAGE_SIZE)
		n_entries++; // we'll put the first page in its own 'unusable' region

	for (uint64_t i=0 ; i<limine_memap->entry_count-1 ; i++){
		cur = limine_memap->entries[i];
		next = limine_memap->entries[i+1];

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
	}
}

static void copyIntoMemoryMap(struct MemoryMap* mmap, struct limine_memmap_response *limine_mmap){
	int memap_index = 0; // in mugOS mmap

	// If first physical page is usable, move it in its own reserved region
	if (limine_mmap->entries[0]->type == LIMINE_MEMMAP_USABLE && limine_mmap->entries[0]->base == 0){
		mmap->entries[memap_index].address = 0;
		mmap->entries[memap_index].length = PAGE_SIZE;
		mmap->entries[memap_index].type = MEMORY_RESERVED;
		memap_index++;
		// Modify the first entry so that it gets copied properly
		limine_mmap->entries[0]->base = PAGE_SIZE;
		limine_mmap->entries[0]->length -= PAGE_SIZE;
	}

	// Copy the entries
	for(uint64_t i=0 ; i<limine_mmap->entry_count ; i++){
		struct limine_memmap_entry* cur = limine_mmap->entries[i];

		if (limine_mmap->entries[i]->type == REMOVED_ENTRY_LIMINE)
			continue;

		mmap->entries[memap_index].address = cur->base;
		mmap->entries[memap_index].length = cur->length;
		mmap->entries[memap_index].type = getMemoryType(cur->type);
		memap_index++;
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

		log(INFO, MODULE, "%#.16llx %#.16llx (%4d %s) - %s",
			cur->address, cur->address + cur->length - 1, cur->length/divisor, SIZE_UNITS[magnitude],
			memoryTypeNames[cur->type]);
	}
}

void MMap_init(struct MemoryMap* memmap, void* firmware_mmap){
	struct limine_memmap_response* limine_mmap = firmware_mmap;

	// Parse Limine's memory map, to set: totalUsableMemory, firstUsablePage, lastUsablePage
	parseLimineMemoryMap(memmap, limine_mmap);
	if (memmap->totalUsableMemory == 0){
		log(PANIC, MODULE, "No usable physical memory available !!");
		panic();
	}

	// Process entries, and get final number of entries
	memmap->size = processLimineMemoryMap(limine_mmap);
	if (memmap->size >= MMAP_MAX_ENTRIES){
		log(PANIC, MODULE, "Memory map is too big to fit in buffer ! "
			"Recompile kernel with MMAP_MAX_ENTRIES > %d (current MMAP_MAX_ENTRIES=%d)",
			memmap->size, MMAP_MAX_ENTRIES);
		panic();
	}

	// Copy limine's memory map into mugOS's memory map
	copyIntoMemoryMap(memmap, limine_mmap);

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
