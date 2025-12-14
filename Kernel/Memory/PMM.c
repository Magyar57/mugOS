#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "string.h"
#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "Boot/LimineRequests.h"
#include "Memory/MemoryMap.h"
#include "Memory/VMM.h"

#include "PMM.h"
#define MODULE "Physical Memory Manager"

struct BitmapAllocator {
	uint64_t nBlocks; // (= #bits)
	paddr_t start;
	uint64_t allocatableBlocks; // #blocks that can be allocated (<= nBlocks)
	uint64_t allocatedBlocks; // #allocated blocks at a given time

	uint64_t* bitmap;
	uint64_t bitmapLength; // bitmap[bitmapLength]
};

static struct BitmapAllocator m_bitmapAllocator;

// ================ Memory allocator ================

static void clearBits(struct BitmapAllocator* allocator, uint64_t start_bit, uint64_t end_bit){
	uint64_t start_index, end_index;
	uint64_t start_mask, end_mask;

	// These check shouldn't be necessary, but better safe than sorry
	if (start_bit >= end_bit) return;
	if (start_bit >= allocator->nBlocks) return;
	if (end_bit > allocator->nBlocks)
		end_bit = allocator->nBlocks;

	// From now on we can update free blocks
	allocator->allocatedBlocks -= end_bit - start_bit;

	// Example on uint8, clearing from start_bit=2 to end_bit=19
	// 11000000 00000000 00011111
	// First and last are what start_mask and end_mask are for
	// The middle is handled faster by a for loop

	// Compute masks
	uint64_t index_in_first_uint64 = start_bit % 64;
	uint64_t index_in_last_uint64 = end_bit % 64;
	start_mask = (index_in_first_uint64 == 0) ? 0x0000000000000000 :
		~((1llu << (64 - index_in_first_uint64)) - 1); // e.g. 3 => 0b11100000
	end_mask = (index_in_last_uint64 == 0) ? 0xffffffffffffffff :
		~(((1llu << index_in_last_uint64) - 1) << (64 - index_in_last_uint64)); // e.g. 3 => 0b00000111

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
	for (uint64_t i=start_index+1 ; i<end_index ; i++)
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

	// From now on we can update free blocks
	allocator->allocatedBlocks += end_bit - start_bit;

	// Example on uint8, setting from start_bit=2 to end_bit=19
	// 00111111 11111111 11100000
	// First and last are what start_mask and end_mask are for
	// The middle is handled faster by a for loop

	// Compute masks
	uint64_t index_in_first_uint64 = start_bit % 64;
	uint64_t index_in_last_uint64 = end_bit % 64;
	start_mask = (index_in_first_uint64 == 0) ? 0xffffffffffffffff :
		(1llu << (64 - index_in_first_uint64)) - 1; // e.g. 3 => 0b00011111
	end_mask = (index_in_last_uint64 == 0) ? 0x0000000000000000 :
		((1llu << index_in_last_uint64) - 1) << (64 - index_in_last_uint64); // e.g. 3 => 0b11111000

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
	for (uint64_t i=start_index+1 ; i<end_index ; i++)
		allocator->bitmap[i] = 0xffffffffffffffff;
	if (index_in_last_uint64 != 0)
		allocator->bitmap[end_index] |= end_mask;
}

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
		// Note: __builtin_popcountll counts the number of bits to 1 in the argument
		n_bits += 64 - __builtin_popcountll(cur);
	}

	// Last uint64 is special to parse: we need to ignore the last bits
	cur = allocator->bitmap[allocator->bitmapLength-1];
	int n_bits_remaining = allocator->nBlocks % 64;
	if (n_bits_remaining == 0)
		n_bits_remaining = 64;

	for (int j=0 ; j<n_bits_remaining ; j++){
		if ((cur & 0x8000000000000000) == 0)
			n_bits++;
		cur <<= 1;
	}

	return n_bits;
}

static bool isFullyAllocated(struct BitmapAllocator* allocator, uint64_t start_bit, uint64_t end_bit){
	uint64_t cur;

	if (start_bit >= end_bit) return false;
	if (start_bit >= allocator->nBlocks) return false;
	if (end_bit > allocator->nBlocks) return false;

	uint64_t start_index = start_bit / 64;
	uint64_t end_index = end_bit / 64;
	int index_in_first_uint64 = start_bit % 64;
	int index_in_last_uint64 = end_bit % 64;

	// Edge case: only one uint64 to check
	if (start_index == end_index){
		cur = allocator->bitmap[start_index] >> (64 - index_in_last_uint64);
		for (uint64_t i=start_bit ; i<end_bit ; i++){
			if ((cur & 1) == 0)
				return false;
			cur >>= 1;
		}
		return true;
	}

	// First uint64
	if (index_in_first_uint64 != 0){
		cur = allocator->bitmap[start_index];
		start_index++;
		for (int j=index_in_first_uint64 ; j<64 ; j++){
			if ((cur & 1) == 0)
				return false;
			cur >>= 1;
		}
	}
	// Middle
	for (uint64_t i=start_index ; i<end_index-1 ; i++){
		cur = allocator->bitmap[i];
		if (cur != 0xffffffffffffffff)
			return false;
	}
	// Last uint64
	cur = allocator->bitmap[end_index];
	for (int j=0 ; j<index_in_last_uint64 ; j++){
		if ((cur & 0x8000000000000000) == 0)
			return false;
		cur <<= 1;
	}

	return true;
}

static inline paddr_t allocate_firstFit(struct BitmapAllocator* allocator, uint64_t n_pages){
	// Search for n_blocks consecutive free bits in the bitmap
	uint64_t n_bits = 0; // consecutive free bits in the current sequence
	uint64_t cur;

	if (n_pages == 0)
		return (paddr_t) NULL;

	uint64_t i;
	int j;
	for (i=0 ; i<allocator->bitmapLength-1 ; i++){
		cur = allocator->bitmap[i];
		for (j=0 ; j<64 ; j++){
			if ((cur & 0x8000000000000000) == 0)
				n_bits++;
			else
				n_bits = 0;

			if (n_bits == n_pages){
				uint64_t end_idx = i*64 + j + 1;
				uint64_t start_idx = end_idx - n_pages;
				setBits(allocator, start_idx, end_idx);
				return allocator->start + start_idx * PAGE_SIZE;
			}

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

		if (n_bits == n_pages){
			uint64_t end_idx = i*64 + j + 1;
			uint64_t start_idx = end_idx - n_pages;
			setBits(allocator, start_idx, end_idx);
			return allocator->start + start_idx * PAGE_SIZE;
		}

		cur <<= 1;
	}

	return (paddr_t) NULL;
}

paddr_t PMM_allocatePages(uint64_t n_pages){
	return allocate_firstFit(&m_bitmapAllocator, n_pages);
}

void PMM_freePages(paddr_t addr, uint64_t n_pages){
	// Note 1: we don't check that the freed address is invalid ; We assume that
	// the kernel code that called this doesn't mess up its addresses and sizes
	// Note 2: Bounds are checked by both isFullyAllocated and clearBits

	uint64_t start_bit = (addr - m_bitmapAllocator.start) / PAGE_SIZE;
	uint64_t end_bit = start_bit + n_pages;

	// Check that we don't free stuff that's already free
	if (!isFullyAllocated(&m_bitmapAllocator, start_bit, end_bit)){
		log(ERROR, MODULE, "PMM_freePages: double free or bogus pointer detected");
		return;
	}

	// Free the memory in the bitmap
	clearBits(&m_bitmapAllocator, start_bit, end_bit);
}

// ================ Functions ================

void PMM_printMemoryUsage(){
	const uint64_t total = m_bitmapAllocator.allocatableBlocks * PAGE_SIZE;
	const uint64_t used = m_bitmapAllocator.allocatedBlocks * PAGE_SIZE;
	uint64_t per_ten_thousand = 10000 * used / total;

	int magnitude_totalMem = getMagnitude(total);
	magnitude_totalMem = (magnitude_totalMem > 6) ? 6 : magnitude_totalMem;
	int divisor_totalMem = (1llu << (10*magnitude_totalMem));
	uint64_t total_atMagnitude = (divisor_totalMem == 0) ? total : total/divisor_totalMem;

	int magnitude_usedMem = getMagnitude(used);
	magnitude_usedMem = (magnitude_usedMem > 6) ? 6 : magnitude_usedMem;
	int divisor_usedMem = (1llu << (10*magnitude_usedMem));
	uint64_t used_atMagnitude = (divisor_usedMem == 0)? used : used/divisor_usedMem;

	log(INFO, MODULE, "Memory usage: %lu %s / %lu %s (%02lu.%02lu%%)",
		used_atMagnitude, SIZE_UNITS[magnitude_usedMem],
		total_atMagnitude, SIZE_UNITS[magnitude_totalMem],
		per_ten_thousand / 100, per_ten_thousand % 100
	);
}

void PMM_printPagesUsage(){
	const uint64_t total = m_bitmapAllocator.allocatableBlocks;
	const uint64_t used = m_bitmapAllocator.allocatedBlocks;

	log(DEBUG, MODULE, "Pages usage: %lu / %lu", used, total);
}

// ================ Initialization ================

/// @brief Get the bitmap start address.
/// In the case where it would be 0, returns the next free page
static paddr_t getBitmapStart(){
	for (int i=0 ; i<g_memoryMap.size ; i++){
		struct MemoryMapEntry* cur = g_memoryMap.entries + i;

		if (cur->type != MEMORY_USABLE)
			continue;

		if (cur->address != 0)
			return cur->address;

		// If the region is 2 pages or more, return it
		if (cur->length > PAGE_SIZE)
			return cur->address + PAGE_SIZE;

		// Region is at address 0, with a length of one free page
		// Continue and return the next free region
	}

	log(PANIC, MODULE, "No free allocatable memory to manage !!");
	panic();
}

static uint64_t getAllocatableBlocks(){
	uint64_t allocatable_bytes = 0;

	for (int i=0 ; i<g_memoryMap.size ; i++){
		switch (g_memoryMap.entries[i].type){
		case MEMORY_USABLE:
		case MEMORY_ACPI_RECLAIMABLE:
		case MEMORY_BOOTLOADER_RECLAIMABLE:
			allocatable_bytes += g_memoryMap.entries[i].length;
			if (g_memoryMap.entries[i].address == 0)
				allocatable_bytes -= PAGE_SIZE; // first page always removed
			break;
		default:
			break;
		}
	}

	return allocatable_bytes / PAGE_SIZE;
}

// One-time-use memory allocation, using first fit algorithm. Guaranteed to return a value
static paddr_t earlyAllocate(struct limine_memmap_response* memmap, uint64_t n_pages){
	uint64_t needed = n_pages * PAGE_SIZE;

	// Ensure one-time-use
	static bool allocated_already = false;
	assert(!allocated_already);
	allocated_already = true;

	// Search the mem map
	for (uint64_t i=0 ; i<memmap->entry_count ; i++){
		struct limine_memmap_entry* cur = memmap->entries[i];

		if (cur->type == LIMINE_MEMMAP_USABLE && cur->length >= needed){
			// Do not allocate address 0
			if (cur->base == 0){
				if (cur->length <= needed)
					continue;

				return cur->base + PAGE_SIZE;
			}
			return cur->base;
		}
	}

	log(PANIC, MODULE, "earlyAllocate out of memory !!");
	panic();
}

static void initBitmap(struct BitmapAllocator* allocator, struct MemoryMap* memmap,
					   paddr_t bitmap_addr_phys, uint64_t nPages){
	// Note: Before calling initBitmap, start, nBlocks and allocatableBlocks needs to be set

	allocator->bitmapLength = (allocator->nBlocks + 63) / 64;

	// Initialized bitmap with all regions used/reserved
	memset(allocator->bitmap, 0xff, allocator->bitmapLength*sizeof(uint64_t));
	allocator->allocatedBlocks = allocator->allocatableBlocks;

	// Set the usable memory as free
	int start_bit, end_bit;
	uint64_t freeMemory = 0; // count free memory for verification
	for (int i=0 ; i<memmap->size ; i++){
		struct MemoryMapEntry* cur = &memmap->entries[i];
		if (cur->type != MEMORY_USABLE)
			continue;

		freeMemory += cur->length;

		start_bit = (cur->address - allocator->start) / PAGE_SIZE;
		end_bit = (cur->address - allocator->start + cur->length) / PAGE_SIZE;
		clearBits(allocator, start_bit, end_bit);
	}

	// Assert that we didn't mess up anything
	uint64_t freeBlocks = countFreeBlocks(allocator);
	assert(allocator->allocatedBlocks == allocator->allocatableBlocks - freeBlocks);
	assert(freeMemory == PAGE_SIZE * freeBlocks);

	// From now, we can mark as used the memory that we earlyAllocated
	start_bit = (bitmap_addr_phys - allocator->start) / PAGE_SIZE;
	end_bit = start_bit + nPages;
	setBits(allocator, start_bit, end_bit);

	// Verify that we removed 'nPages' bits
	assert(freeBlocks - nPages == countFreeBlocks(allocator));
}

void PMM_init(){
	if (g_memoryMap.size == 0){
		log(PANIC, MODULE, "PMM initialization needs the MemoryMap to be initialized first !");
		panic();
	}

	// Compute allocator sizes
	// Note: we add 1 because from @start to the n-th page, there is n-1 pages
	m_bitmapAllocator.start = getBitmapStart();
	m_bitmapAllocator.allocatableBlocks = getAllocatableBlocks();
	m_bitmapAllocator.nBlocks =
		((g_memoryMap.lastUsablePage - m_bitmapAllocator.start) / PAGE_SIZE) + 1;

	// Allocate memory for the bitmap
	size_t bitmapSize = (m_bitmapAllocator.nBlocks + 7) / 8; // Note: +7 rounds the division up
	uint64_t n_pages = getSizeAsPages(bitmapSize);
	paddr_t allocated = earlyAllocate(g_memmapReq.response, n_pages);
	vaddr_t allocated_virt = VMM_toHHDM(allocated);
	VMM_premap(allocated, allocated_virt, n_pages, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
	m_bitmapAllocator.bitmap = (uint64_t*) allocated_virt;

	initBitmap(&m_bitmapAllocator, &g_memoryMap, allocated, n_pages);

	PMM_printMemoryUsage();
	log(SUCCESS, MODULE, "Initialization success (managing %lu pages, %lu allocatable)",
		m_bitmapAllocator.nBlocks, m_bitmapAllocator.allocatableBlocks);
}
