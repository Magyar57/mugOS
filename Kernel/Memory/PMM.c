#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "string.h"
#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "Boot/LimineRequests.h"

#include "PMM.h"
#define MODULE "Physical Memory Manager"

// Used in limine's memory map, when parsing, to mark an entry as "removed"
#define REMOVED_ENTRY_LIMINE 574213

enum MemoryType {
	MEMORY_USABLE,
	MEMORY_RESERVED,
	MEMORY_KERNEL, // ktext & kdata
	MEMORY_FRAMEBUFFER,
	MEMORY_ACPI_NVS,
	MEMORY_ACPI_RECLAIMABLE,
	MEMORY_BOOTLOADER_RECLAIMABLE,
};

struct MemoryMap {
	int size;
	struct MemoryMapEntry {
		physical_address_t address;
		uint64_t length;
		enum MemoryType type;
	} *entries;
};

struct BitmapAllocator {
	uint64_t nBlocks; // (= #bits)
	physical_address_t start;

	uint64_t* bitmap;
	uint64_t bitmapLength; // bitmap[bitmapLength]
};

struct PhysicalMemoryData {
	uint64_t totalMemory;
	uint64_t freeMemory;

	struct MemoryMap memMap;
	struct BitmapAllocator allocator;
};

static struct PhysicalMemoryData m_pmData;

// ================ Memory allocator ================

static void clearBits(struct BitmapAllocator* allocator, uint64_t start_bit, uint64_t end_bit){
	uint64_t start_index, end_index;
	uint64_t start_mask, end_mask;

	// These check shouldn't be necessary, but better safe than sorry
	if (start_bit >= end_bit) return;
	if (start_bit >= allocator->nBlocks) return;
	if (end_bit > allocator->nBlocks)
		end_bit = allocator->nBlocks;

	// Example on uint8, clearing from start_bit=2 to end_bit=19
	// 11000000 00000000 00011111
	// First and last are what start_mask and end_mask are for
	// The middle is handled faster by a for loop

	// Compute masks
	uint64_t index_in_first_uint64 = start_bit % 64;
	uint64_t index_in_last_uint64 = end_bit % 64;
	start_mask = (index_in_first_uint64 == 0) ? 0x0000000000000000 :
		~((1llu << (64 - index_in_first_uint64)) - 1); // e.g. 3 => 0b00000111
	end_mask = (index_in_last_uint64 == 0) ? 0xffffffffffffffff :
		~(((1llu << index_in_last_uint64) - 1) << (64 - index_in_last_uint64)); // e.g. 3 => 0b11100000

	// Apply masks

	// Indexes in the uint64_t* bitmap
	start_index = start_bit / 64;
	end_index = end_bit / 64;

	// Special case: we need to apply both masks in the same uint64
	if (start_index == end_index){
		allocator->bitmap[start_index] &= start_mask | end_mask;
		return;
	}

	allocator->bitmap[start_index] &= start_mask;
	for(uint64_t i=start_index+1 ; i<end_index ; i++)
		allocator->bitmap[i] = 0x0000000000000000;
	if (index_in_last_uint64 != 0)
		allocator->bitmap[end_index] &= end_mask;
}

static void setBits(struct BitmapAllocator* allocator, uint64_t start_bit, uint64_t end_bit){
	uint64_t start_index, end_index;
	uint64_t start_mask, end_mask;

	// These check shouldn't be necessary, but better safe than sorry
	if (start_bit >= end_bit) return;
	if (start_bit >= allocator->nBlocks) return;
	if (end_bit > allocator->nBlocks)
		end_bit = allocator->nBlocks;

	// Example on uint8, setting from start_bit=2 to end_bit=19
	// 00111111 11111111 11100000
	// First and last are what start_mask and end_mask are for
	// The middle is handled faster by a for loop

	// Compute masks
	uint64_t index_in_first_uint64 = start_bit % 64;
	uint64_t index_in_last_uint64 = end_bit % 64;
	start_mask = (index_in_first_uint64 == 0) ? 0xffffffffffffffff :
		(1llu << (64 - index_in_first_uint64)) - 1; // e.g. 3 => 0b11111000
	end_mask = (index_in_last_uint64 == 0) ? 0x0000000000000000 :
		((1llu << index_in_last_uint64) - 1) << (64 - index_in_last_uint64); // e.g. 3 => 0b00011111

	// Apply masks

	// Indexes in the uint64_t* bitmap
	start_index = start_bit / 64;
	end_index = end_bit / 64;

	// Special case: we need to apply both masks in the same uint64
	if (start_index == end_index){
		allocator->bitmap[start_index] |= start_mask & end_mask;
		return;
	}

	allocator->bitmap[start_index] |= start_mask;
	for(uint64_t i=start_index+1 ; i<end_index ; i++)
		allocator->bitmap[i] = 0xffffffffffffffff;
	if (index_in_last_uint64 != 0)
		allocator->bitmap[end_index] |= end_mask;
}

static const char* SIZE_UNITS[] = {
	"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB",
	"ZiB" // not representible on uint64 (max is ~16 EiB)
};

// Get the number of pages needed to store `size` bytes
static inline uint64_t getSizeAsPages(size_t n_bytes){
	// Note: Adding PAGE_SIZE-1 rounds the integer division up
	return (n_bytes + PAGE_SIZE-1) / PAGE_SIZE;
}

static uint64_t countFreeBlocks(struct BitmapAllocator* allocator){
	uint64_t n_bits = 0;
	uint64_t cur;

	for (uint64_t i=0 ; i<allocator->bitmapLength-1 ; i++){
		cur = allocator->bitmap[i];

		// Compute how many bits are free in the current uint64
		for (int j=0 ; j<64 ; j++){
			if ((cur & 0x8000000000000000) == 0)
				n_bits++;
			cur <<= 1;
		}
	}

	// Last uint64 is special to parse: we need to ignore the last bits
	cur = allocator->bitmap[allocator->bitmapLength-1];
	int n_bits_remaining = allocator->nBlocks % 64;
	for (int j=0 ; j<n_bits_remaining ; j++){
		if ((cur & 0x8000000000000000) == 0)
			n_bits++;
		cur <<= 1;
	}

	return n_bits;
}

static inline physical_address_t allocate_FirstFit(struct BitmapAllocator* allocator, uint64_t n_pages){
	// Search for n_blocks consecutive free bits in the bitmap
	uint64_t n_bits = 0; // consecutive free bits in the current sequence
	uint64_t cur;

	if (n_pages == 0)
		return (physical_address_t) NULL;

	uint64_t i;
	int j;
	for (i=0 ; i<allocator->bitmapLength-1 ; i++){
		cur = allocator->bitmap[i];
		for (j=0 ; j<64 ; j++){
			if ((cur & 0x8000000000000000) == 0)
				n_bits++;
			else
				n_bits = 0;

			if (n_bits == n_pages)
				goto found;

			cur <<= 1;
		}
	}

	// Last uint64 is special to parse: we need to ignore the last bits
	cur = allocator->bitmap[allocator->bitmapLength-1];
	int n_bits_remaining = allocator->nBlocks % 64;
	for (j=0 ; j<n_bits_remaining ; j++){
		if ((cur & 0x8000000000000000) == 0)
			n_bits++;
		else
			n_bits = 0;

		if (n_bits == n_pages)
			goto found;

		cur <<= 1;
	}

	return (physical_address_t) NULL;

	found:
	uint64_t end_idx = i*64 + j + 1;
	uint64_t start_idx = end_idx - n_pages;
	setBits(allocator, start_idx, end_idx);
	m_pmData.freeMemory -= n_pages * PAGE_SIZE;
	return allocator->start + start_idx * PAGE_SIZE;
}

physical_address_t PMM_allocate(uint64_t n_pages){
	return allocate_FirstFit(&m_pmData.allocator, n_pages);
}

void PMM_free(physical_address_t addr, uint64_t n_pages){
	// Note: we don't check that the freed address is invalid
	// We assume that the kernel code that called this doesn't mess up its
	// addresses and sizes
	// And bounds are checked by the clearBits method

	// Free the memory in the bitmap
	uint64_t start_bit = (addr - m_pmData.allocator.start) >> PAGE_SHIFT;
	uint64_t end_bit = start_bit + n_pages;
	clearBits(&m_pmData.allocator, start_bit, end_bit);
	m_pmData.freeMemory += n_pages * PAGE_SIZE;
}

// ================ Memory map ================

/// @returns n so that `1024^(n+1) >= byte_number > 1024^n`
/// @note Example: `512 B` => `0`, `1023 B` => `0`, `1024 B` => `1`, `1025 B` => `1`, `1048576` => `3`...
static int getMagnitude(uint64_t byte_number){
    int i = 0;
    while (byte_number >= 1024){
        byte_number /= 1024;
        i++;
    }
    return i;
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

void PMM_printMemoryUsage(){
	int magnitude_totalMem = getMagnitude(m_pmData.totalMemory);
	magnitude_totalMem = (magnitude_totalMem > 6) ? 6 : magnitude_totalMem;
	int divisor_totalMem = (1llu << (10*magnitude_totalMem));

	uint64_t used = m_pmData.totalMemory - m_pmData.freeMemory;
	int magnitude_usedMem = getMagnitude(used);
	magnitude_usedMem = (magnitude_usedMem > 6) ? 6 : magnitude_usedMem;
	int divisor_usedMem = (1llu << (10*magnitude_usedMem));

	log(INFO, MODULE, "Memory usage: %llu %s / %llu %s",
		used/divisor_usedMem, SIZE_UNITS[magnitude_usedMem],
		m_pmData.totalMemory/divisor_totalMem, SIZE_UNITS[magnitude_totalMem]
	);
}

// ================ Initialization ================

// Returns the number of entries in the processed memory map
static int processLimineMemoryMap(struct limine_memmap_response* memmap){
	if (memmap->entry_count == 0) return 0;
	int n_entries = 1; // last entry

	for (uint64_t i=0 ; i<memmap->entry_count-1 ; i++){
		struct limine_memmap_entry* cur = memmap->entries[i];
		struct limine_memmap_entry* next = memmap->entries[i+1];
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

static void parseLimineMemoryMap(struct limine_memmap_response* memmap,
							physical_address_t* totalMemory_out,
							physical_address_t* firstFreeMemory_out,
							physical_address_t* lastFreeMemory_out){
	bool firstNotFound = true;
	*firstFreeMemory_out = 0;
	*lastFreeMemory_out = 0;
	uint64_t temp;

	for (uint64_t i=0 ; i<memmap->entry_count ; i++){
		struct limine_memmap_entry* cur = memmap->entries[i];

		switch (cur->type){
		case LIMINE_MEMMAP_USABLE:
			*totalMemory_out += cur->length;
			*lastFreeMemory_out = cur->base + cur->length - PAGE_SIZE; // Last page of the region
			if (firstNotFound){
				*firstFreeMemory_out = cur->base;
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
			if ( (i < memmap->entry_count-1) && (cur->base+cur->length >= memmap->entries[i+1]->base) )
				temp = memmap->entries[i+1]->base - cur->base;
			*totalMemory_out += temp;
			break;
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			*totalMemory_out += cur->length;
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

	if (*totalMemory_out == 0){
		log(PANIC, MODULE, "No usable physical memory available !!");
		panic();
	}
}

// One time memory allocation, using first fit algorithm. Guaranteed to return a value
static physical_address_t earlyAllocate(struct limine_memmap_response* memmap, uint64_t n_pages){
	uint64_t needed = n_pages * PAGE_SIZE;

	// Search the mem map
	for (uint64_t i=0 ; i<memmap->entry_count ; i++){
		struct limine_memmap_entry* cur = memmap->entries[i];

		if (cur->type == LIMINE_MEMMAP_USABLE && cur->length >= needed)
			return cur->base;
	}

	log(PANIC, MODULE, "earlyAllocate out of memory !!");
	panic();
	unreachable();
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

static void initMemoryMap(struct limine_memmap_response* limine_memmap, struct MemoryMap* memmap){
	int memap_index = 0; // in mugOS memmap

	for(uint64_t i=0 ; i<limine_memmap->entry_count ; i++){
		struct limine_memmap_entry* cur = limine_memmap->entries[i];

		if (limine_memmap->entries[i]->type == REMOVED_ENTRY_LIMINE)
			continue;

		// Copy entry
		memmap->entries[memap_index].address = cur->base;
		memmap->entries[memap_index].length = cur->length;
		memmap->entries[memap_index].type = getMemoryType(cur->type);
		memap_index++;
	}
}

static inline uint64_t roundUp(uint64_t value, unsigned int discretization){
	if (discretization == 0) return value;
	return ((value + discretization -1) / discretization) * discretization;
}

static void initBitmap(struct PhysicalMemoryData* data, physical_address_t allocated, uint64_t nPages){
	// Set attributes (note: nBlocks was set already)
	data->allocator.bitmapLength = (data->allocator.nBlocks + 63) / 64;

	// Initialized bitmap with all regions used/reserved
	memset(data->allocator.bitmap, 0xff, data->allocator.bitmapLength*sizeof(uint64_t));

	// Set the usable memory as free
	int start_bit, end_bit;
	uint64_t freeMemory = 0; // count free memory for verification
	for (int i=0 ; i<data->memMap.size ; i++){
		struct MemoryMapEntry* cur = &data->memMap.entries[i];
		if (cur->type != MEMORY_USABLE)
			continue;

		freeMemory += cur->length;

		start_bit = (cur->address - data->allocator.start) >> PAGE_SHIFT;
		end_bit = (cur->address - data->allocator.start + cur->length) >> PAGE_SHIFT;
		clearBits(&data->allocator, start_bit, end_bit);
	}

	assert(freeMemory == PAGE_SIZE * countFreeBlocks(&data->allocator));

	// From now we can mark as used the memory that we earlyAllocated
	start_bit = allocated >> PAGE_SHIFT;
	end_bit = start_bit + nPages;
	setBits(&data->allocator, start_bit, end_bit);

	// Count free memory
	data->freeMemory = PAGE_SIZE * countFreeBlocks(&data->allocator);
}

// TODO move where it belongs (where?)
static inline virtual_address_t physicalToVirtual(physical_address_t addr){
	const uint64_t virtualOffset = hhdmReq.response->offset;
	return (uint64_t)addr + virtualOffset;
}

void PMM_initialize(){
	physical_address_t allocated;
	physical_address_t lastFreePage; // as an address
	size_t bitmapSize, memmapSize; // sizes are in bytes

	// Limine's memory map: parse values and process (squash together consecutive entries of same type)
	parseLimineMemoryMap(memmapReq.response, &m_pmData.totalMemory, &m_pmData.allocator.start, &lastFreePage);
	m_pmData.memMap.size = processLimineMemoryMap(memmapReq.response);

	// Compute allocator size
	// Note: we add 1 because from @start to the n-th page, there is n-1 pages
	m_pmData.allocator.nBlocks = ((lastFreePage - m_pmData.allocator.start) >> PAGE_SHIFT) + 1;

	// Compute the memory that we need to allocate
	bitmapSize = (m_pmData.allocator.nBlocks + 7) / 8; // Note: +7 rounds the division up
	bitmapSize = roundUp(bitmapSize, sizeof(struct MemoryMapEntry*)); // Align the next structure's size
	memmapSize = m_pmData.memMap.size * sizeof(struct MemoryMapEntry*);

	// Allocate
	uint64_t n_pages = getSizeAsPages(bitmapSize + memmapSize);
	allocated = earlyAllocate(memmapReq.response, n_pages);
	// Split the memory we got
	void* addr = (void*) physicalToVirtual(allocated);
	m_pmData.allocator.bitmap = (uint64_t*) addr;
	m_pmData.memMap.entries = (struct MemoryMapEntry*) (addr + bitmapSize);

	// Initializations
	memset(m_pmData.memMap.entries, 0, memmapSize);
	initMemoryMap(memmapReq.response, &m_pmData.memMap);
	initBitmap(&m_pmData, allocated, n_pages);

	// Ok, print stuff
	printMemoryMap(&m_pmData.memMap);
	PMM_printMemoryUsage();

	log(SUCCESS, MODULE, "Initialization success (managing %d pages)", m_pmData.allocator.nBlocks);
}
