#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "Preprocessor.h"
#include "Memory/Memory.h"

#ifdef KERNEL
#include "Memory/PMM.h" // PMM_allocatePages
#include "Memory/VMM.h" // VMM_Heap_physToVirt / virtToPhys
#else
// #include <sys/mman.h> // mmap
#error "Implement a mmap-like system call"
#endif

// Heap.c: Heap implementation, inspired by OpenBSD's sbrk-free implementation
// - All allocations are done with mmap
// - No intialization needed (it's done dynamically)
// - Fast O(1) allocations and deallocations
// - Quite big metadata
// - Metadata is kept away from user blocks, to avoid under/overflow issues
//
// Internal structures:
// - mmap-ed regions are refered to as Chunks: big chunks are whole, small chunks
//   are divided into allocatable blocks, managed with a bitmap.
// - 1 Chunk <=> 1 ChunkInfo structure, describing it. (e.g address, size, small chunks' bitmap)
// - A `Hashmap<void*, ChunkInfo*>` keeps track of what addresses are allocated to which
//   ChunkInfo, allowing to retrieve informations
// - A page-sized 'Chungus' structure manages free ChunkInfo to be allocated (with a bitmap)
//
// Heap:
// - A Heap structure stores all needed data structures
// - A doubly-linked list of Chungus ('chunguses') is kept in Heap information
// - Doubly-linked lists of (partially-)free Chunks, called smallbuckets, are kept in Heap information
// - A cache for big chunks allows to reuse mmaped regions, avoiding costly mmap syscalls

// Small buckets store blocks in chunks, managed with bitmaps
#define MIN_BLOCK_SIZE				64
#define SMALLBUCKETS_OFFSET			6 // = log2(MIN_BLOCK_SIZE)
#define N_SMALLBUCKET				6 // log2(BIGBUCKET_THRESHOLD) - log2(MIN_BLOCK_SIZE)
#define getSmallbucketSize(order)	(1 << (order+SMALLBUCKETS_OFFSET))

// Large buckets are used when allocations are too big for Chunks bitmaps
#define BIGBUCKET_THRESHOLD			PAGE_SIZE // WARN: most functions make this assumption
#define N_BIGBLOCKS_CACHED			16
#define BIGBLOCKS_CACHE_MAXSIZE		16*PAGE_SIZE

// Chungus are page-sized structures that store ChunkInfo structs
#define CHUNGUS_N_CHUNKS			((PAGE_SIZE - sizeof(struct Chungus)) / sizeof(struct ChunkInfo))
#define CHUNGUS_BITMAP_SIZE			2 // 2*8*sizeof(uint64_t) = 128 ChunkInfo = 5120 B
#define getChungus(chunkInfoAddr)	(struct Chungus*) getPage(chunkInfoAddr)
#define getChunkInfoIndex(chunk)	((getOffset(chunk) - sizeof(struct Chungus)) / sizeof(struct ChunkInfo))

struct ChunkInfo {
	void* base;				// Base address
	size_t size;			// Size of the allocated region
	uint64_t bitmap;		// Allocation bitmap. It must fit in one uint64_t

	struct ChunkInfo* next; // Doubly-linked list: next node
	struct ChunkInfo* prev; // Doubly-linked list: previous node
};

struct ChunkBucket {
	// Note: buckets are simply a doubly-linked list of partial ChunkInfo
	struct ChunkInfo* head;
	struct ChunkInfo* tail;
};

struct Chungus {
	uint64_t bitmap[CHUNGUS_BITMAP_SIZE];

	struct Chungus* next;
	struct Chungus* prev;

	struct ChunkInfo chunks[];
};

struct ChungusList {
	struct Chungus* head;
	struct Chungus* tail;
};

compile_assert(sizeof(struct Chungus) + CHUNGUS_N_CHUNKS*sizeof(struct Chungus) <= PAGE_SIZE);

typedef void* key_t;
typedef struct ChunkInfo* value_t;

typedef struct HashmapEntry {
	key_t key;
	value_t value;
} entry_t;

typedef struct Hashmap {
	entry_t* entries;
	long totalEntries;
	long freeEntries;
} hashmap_t;

struct Heap {
	hashmap_t allocMap;									// Map of allocated blocks -> size
	struct ChungusList partialChunguses;				// Pools of allocatable BlockInfo
	struct ChunkBucket smallBuckets[N_SMALLBUCKET];		// Buckets of free chunks (doubly-linked lists)
	struct ChunkInfo freeCache[N_BIGBLOCKS_CACHED];		// Cache for freed large blocks (we reuse some)
};

static struct Heap m_heap;

static void* allocatePages(int n, bool clear);
static void freePages(void* pages, int n);

// ================ Doubly-linked lists ================
// Note: code is dupplicated, to simplify code structure

static void Bucket_pushFront(struct ChunkBucket* bucket, struct ChunkInfo* chunk){
	assert(bucket && chunk);
	chunk->prev = chunk;

	// Note: we don't need to check bucket->tail because it is NULL <=> bucket->head is NULL
	if (bucket->head){
		chunk->next = bucket->head;
		bucket->head->prev = chunk;
		bucket->tail->next = chunk;
	}
	// Bucket is empty
	else {
		chunk->next = chunk;
		bucket->tail = chunk;
	}

	// Change the head AT THE LAST MOMENT to not loose access to the previous one
	bucket->head = chunk;
}

static void Bucket_pop(struct ChunkBucket* bucket, struct ChunkInfo* chunk){
	assert(bucket && chunk);
	assert(bucket->head); // cannot call pop if linked list is empty

	if (chunk == bucket->head && chunk == bucket->tail){
		bucket->head = NULL;
		bucket->tail = NULL;
		chunk->next = NULL;
		chunk->prev = NULL;
		return;
	}

	// Block is ONLY head
	if (chunk == bucket->head){
		bucket->head = chunk->next;
		bucket->tail->prev = chunk->next;
		chunk->next->prev = chunk->next; // loop head
		chunk->next = NULL;
		chunk->prev = NULL;
		return;
	}

	// Block is ONLY tail
	if (chunk == bucket->tail){
		bucket->tail = chunk->prev;
		chunk->prev->next = bucket->head;
		chunk->next = NULL;
		chunk->prev = NULL;
		return;
	}

	chunk->next->prev = chunk->prev;
	chunk->prev->next = chunk->next;

	chunk->next = NULL;
	chunk->prev = NULL;
}

static void ChungusList_pushFront(struct ChungusList* chunguses, struct Chungus* chungus){
	assert(chunguses && chungus);
	chungus->prev = chungus;

	// Note: we don't need to check chunguses->tail because it is NULL <=> chunguses->head is NULL
	if (chunguses->head){
		chungus->next = chunguses->head;
		chunguses->head->prev = chungus;
		chunguses->tail->next = chungus;
	}
	// List is empty
	else {
		chungus->next = chungus;
		chunguses->tail = chungus;
	}

	// Change the head AT THE LAST MOMENT to not loose access to the previous one
	chunguses->head = chungus;
}

static void Chunguses_pop(struct ChungusList* chunguses, struct Chungus* chungus){
	assert(chunguses && chungus);
	assert(chunguses->head); // cannot call pop if linked list is empty

	if (chungus == chunguses->head && chungus == chunguses->tail){
		chunguses->head = NULL;
		chunguses->tail = NULL;
		chungus->next = NULL;
		chungus->prev = NULL;
		return;
	}

	// Block is ONLY head
	if (chungus == chunguses->head){
		chunguses->head = chungus->next;
		chunguses->tail->prev = chungus->next;
		chungus->next->prev = chungus->next; // loop head
		chungus->next = NULL;
		chungus->prev = NULL;
		return;
	}

	// Block is ONLY tail
	if (chungus == chunguses->tail){
		chunguses->tail = chungus->prev;
		chungus->prev->next = chunguses->head;
		chungus->next = NULL;
		chungus->prev = NULL;
		return;
	}

	chungus->next->prev = chungus->prev;
	chungus->prev->next = chungus->next;

	chungus->next = NULL;
	chungus->prev = NULL;
}

// ================ Hashmap ================

#define HASHMAP_DEFAULT_SIZE		PAGE_SIZE/sizeof(entry_t)
// Max size of hashmap, in bytes
#define HASHMAP_MAX_SIZE			(long) (SIZE_MAX / (2*sizeof(entry_t)))
// Grow hashmap if we're below 25% of free space (75% full)
// Do NOT EVER set it to 0%, algorithms rely on assumption that map is partially empty
#define hashmapTooFull(map)			(map->freeEntries*4 < map->totalEntries)

static inline uint64_t hash(void* key){
	// Hashing pointers, maybe test later
	uint64_t sum;
	uintptr_t actual_ptr_value;

	actual_ptr_value = (uintptr_t)key >> PAGE_SHIFT;
	sum = actual_ptr_value;
	sum = (sum << 7) - sum + (actual_ptr_value >> 16);

	// Use these two lines only on 64 bits (we only support 64 bits CPU anyway)
	sum = (sum << 7) - sum + (actual_ptr_value >> 32);
	sum = (sum << 7) - sum + (actual_ptr_value >> 48);

	return sum;
}

static bool Hashmap_grow(hashmap_t* map){
	assert(map);
	void* p;
	entry_t* new_values;

	if (map->totalEntries > HASHMAP_MAX_SIZE)
		return false;

	// Double the capacity
	long new_total = (map->totalEntries == 0) ? HASHMAP_DEFAULT_SIZE : 2*map->totalEntries;
	int n_pages = roundToPage(new_total * sizeof(entry_t));
	uint64_t mask = new_total - 1;

	new_values = allocatePages(n_pages, true);
	if (!new_values) return false;

	// Copy the entries from old to new array
	for (long i=0 ; i<map->totalEntries ; i++){
		p = map->entries[i].key;
		if (p == NULL)
			continue;
		uint64_t j = hash(p) & mask;
		// Insert
		while(new_values[j].key != NULL){
			j = (j-1) & mask; // j--;
		}
		new_values[j] = map->entries[i]; // memcpy the value_t
	}

	// Free the previous mmap-ed 'values' array region
	if (map->totalEntries > 0){
		size_t old_size = roundToPage(map->totalEntries*sizeof(entry_t));
		freePages(map->entries, roundToPage(old_size));
	}
	map->entries = new_values;
	map->freeEntries += new_total - map->totalEntries;
	map->totalEntries = new_total;
	return true;
}

static bool Hashmap_insert(hashmap_t* map, key_t ptr, value_t value){
	assert(map);
	if (ptr == NULL) return false; // never insert NULL

	// Lazily check for growth need
	if (hashmapTooFull(map) || map->totalEntries == 0){
		if (!Hashmap_grow(map))
			return false;
	}

	const uint64_t mask = map->totalEntries - 1;

	ptr = (void*) getPage(ptr);
	uint64_t i = hash(ptr) & mask;

	while(map->entries[i].key != NULL){
		i = (i-1) & mask; // i--;
	}

	map->entries[i].key = ptr;
	map->entries[i].value = value;
	map->freeEntries--;
	return true;
}

static entry_t* Hashmap_find(hashmap_t* map, key_t ptr){
	assert(map);
	if (map->entries == NULL) return NULL;
	const uint64_t mask = map->totalEntries - 1;
	uint64_t i;

	ptr = (void*) getPage(ptr);
	i = hash(ptr) & mask;

	// Iterate over 'collisions' bucket
	while(map->entries[i].key != NULL){
		if (map->entries[i].key == ptr)
			return map->entries + i;

		i = (i-1) & mask; // i--;
	}

	return NULL;
}

/// @brief Delete the `entry` from the hashmap.
/// @warning The address MUST be taken from the hashmap values (returned by Hashmap_find) !
static void Hashmap_delete(hashmap_t* map, entry_t* entry){
	assert(map);
	assert(!(map->totalEntries & (map->totalEntries-1))); // assert that total is a power of two
	if (entry == NULL) return;
	if (map->totalEntries == 0) return;

	const uint64_t mask = map->totalEntries - 1;
	uint64_t cur_index, prev_index, target_index;

	cur_index = entry - map->entries; // pointer arithmetic to get index of value
	assert(cur_index < (uint64_t)map->totalEntries);

	// Remove first element
	map->entries[cur_index].key = NULL;
	map->entries[cur_index].value = 0;
	prev_index = cur_index;

	// Handle collision bucket (move back elements that needs it)
	do {
		cur_index = (cur_index-1) & mask;

		// The index at which cur element should be (if we exclude collisions)
		target_index = hash(map->entries[cur_index].key) & mask;
		if (cur_index <= target_index && target_index < prev_index) // cur <= target < prev
			continue;
		if (target_index < prev_index && prev_index < cur_index) // target < prev < cur
			continue;
		if (prev_index < cur_index && cur_index <= target_index) // prev < cur <= target
			continue;

		// Move cur to prev
		map->entries[prev_index] = map->entries[cur_index];
		map->entries[cur_index].key = NULL;
		map->entries[cur_index].value = 0; // or memset if it's a struct
		prev_index = cur_index;

	} while (map->entries[cur_index].key != NULL);

	map->freeEntries++;
}

// ================ Blocks ================

static void* allocateBlock(struct ChunkInfo* chunk){
	// Should not happen if 'chunk' correctly comes from a bucket
	assert(chunk->bitmap != 0xffffffffffffffff);

	// Search for first available block
	// First clear bit is at index #leading_set_bits
	int first_free = __builtin_clzll(~chunk->bitmap);

	// Mark block as allocated
	chunk->bitmap |= (0x8000000000000000 >> first_free);

	return chunk->base + first_free*chunk->size; // n-th block
}

static void freeBlock(struct ChunkInfo* chunk, void* ptr){
	int bitmap_offset = getOffset(ptr) / chunk->size;

	uint64_t mask = ~(0x8000000000000000 >> bitmap_offset);
	chunk->bitmap &= mask;
}

// ================ Chunks ================

static bool allocateChunk(struct ChunkInfo* chunk, size_t size){
	assert(chunk);
	int n_pages = roundToPage(size);

	chunk->base = allocatePages(n_pages, false);
	if (!chunk->base) return false;

	chunk->size = size;
	chunk->next = NULL;
	chunk->prev = NULL;

	// Mark ChunkInfo as allocated in its Chungus
	struct Chungus* chungus = getChungus(chunk);
	unsigned int bitmapIndex = getChunkInfoIndex(chunk);
	int bitmapMajorIndex = bitmapIndex / 64;
	int bitmapMinorIndex = bitmapIndex % 64;
	chungus->bitmap[bitmapMajorIndex] |= 0x8000000000000000 >> bitmapMinorIndex;

	// Empty mask:
	// order 0 => blocks of   64 => 64 bits bitmap => 0b0000000000000000
	// order 1 => blocks of  128 => 32 bits bitmap => 0x00000000ffffffff
	// ...
	// order 4 => blocks of 1024 =>  4 bits bitmap => 0x0fffffffffffffff
	// order 5 => blocks of 2048 =>  2 bits bitmap => 0x3fffffffffffffff
	if (size < BIGBUCKET_THRESHOLD){
		uint64_t mask = (0x8000000000000000 >> (PAGE_SIZE/size-1)) - 1;
		chunk->bitmap = mask;
	}

	return true;
}

static void freeChunk(struct ChunkInfo* chunk){
	freePages(chunk->base, roundToPage(chunk->size));
	chunk->base = NULL;
	chunk->size = 0;
}

static inline bool isChunkFull(struct ChunkInfo* chunk){
	return (chunk->bitmap == 0xffffffffffffffff);
}

static inline bool isChunkEmpty(struct ChunkInfo* chunk){
	// Chunk is empty if its bitmap is equal to its empty mask ; see allocateChunk
	uint64_t empty_mask = (0x8000000000000000 >> (PAGE_SIZE/chunk->size-1)) - 1;
	return (chunk->bitmap == empty_mask);
}

static bool shouldFreeChunk(struct ChunkBucket* bucket, struct ChunkInfo* chunk){
	// We can free chunk if we got enough (>=target) free blocks in the bucket
	const int target_n_blocks = 3*PAGE_SIZE / (2*chunk->size); // threshold: 1.5 fully empty chunks
	int n_free_blocks = 0;
	struct ChunkInfo* cur = bucket->head;

	do {
		if (cur != chunk){
			int n_bits_to_zero = 64 - __builtin_popcountll(chunk->bitmap);
			n_free_blocks += n_bits_to_zero;
		}
		if (n_free_blocks >= target_n_blocks)
			return true;
		cur = cur->next;
	} while(cur->prev != cur);

	return false;
}

// ================ Chungus ================

static struct Chungus* allocateChungus(){
	struct Chungus* chungus = allocatePages(1, false);
	if (!chungus) return NULL;

	for (int i=0 ; i<CHUNGUS_BITMAP_SIZE-1 ; i++)
		chungus->bitmap[i] = 0;
	// Mark the invalid ChunkInfos as used, so that we can never allocate them
	constexpr int index = CHUNGUS_N_CHUNKS % 64;
	uint64_t mask = (index == 0) ? 0x0 :
		(1ull << (64 - index)) - 1; // e.g. 3 => 0b00011111
	chungus->bitmap[CHUNGUS_BITMAP_SIZE-1] = mask;

	chungus->next = NULL;
	chungus->prev = NULL;

	return chungus;
}

static inline void freeChungus(struct Chungus* chungus){
	freePages(chungus, 1);
}

static inline bool isChungusFull(struct Chungus* chungus){
	for (int i=0 ; i<CHUNGUS_BITMAP_SIZE ; i++){
		if (chungus->bitmap[i] != 0xffffffffffffffff)
			return false;
	}

	return true;
}

static inline bool isChungusEmpty(struct Chungus* chungus){
	for (int i=0 ; i<CHUNGUS_BITMAP_SIZE-1 ; i++){
		if (chungus->bitmap[i] != 0x0000000000000000)
			return false;
	}

	// Last is special, we need to mask the last bits: those are always set,
	// since their corresponding BlockInfo is not in the managed page
	constexpr int index = CHUNGUS_N_CHUNKS % 64;
	uint64_t mask = (index == 0) ? 0x0 :
		~((1ull << (64 - index)) - 1); // e.g. 3 => 0b11100000

	bool last_fully_free = ((chungus->bitmap[CHUNGUS_BITMAP_SIZE-1] & mask) == 0x0000000000000000);
	return last_fully_free;
}

static struct ChunkInfo* findFreeChunkInChungus_chungus(struct Chungus* chungus){
	// Note: This is an internal function for findFreeChunk_chungusList

	for (int i=0 ; i<CHUNGUS_BITMAP_SIZE ; i++){
		if (chungus->bitmap[i] == 0xffffffffffffffff)
			continue;

		// Search for first available ChunkInfo
		// First clear bit is at index #leading_set_bits
		int first_free_bit = __builtin_clzll(~chungus->bitmap[i]);
		return chungus->chunks + 64*i + first_free_bit;
	}

	return NULL;
}

static struct ChunkInfo* findFreeChunk_chungusList(struct ChungusList* chunguses){
	assert(chunguses);
	struct Chungus* curChungus = chunguses->head;
	struct ChunkInfo* res;

	// Note: This is an internal function for getFreeChunk

	if (!curChungus)
		return NULL;

	// Iterate over all chunguses
	do {
		res = findFreeChunkInChungus_chungus(curChungus);
		if (res)
			return res;

		curChungus = curChungus->next;
	} while (curChungus->next != curChungus);

	return NULL;
}

/// @brief Searches a free chunk from the `chunguses`, allocating a new chungus if needed
/// @returns Address of the initialized chunk, NULL on failure
static struct ChunkInfo* getFreeChunk(struct ChungusList* chunguses){
	struct ChunkInfo* chunk;
	struct Chungus* chungus;

	chunk = findFreeChunk_chungusList(chunguses);
	// We got a free chunk in a chungus !
	if (chunk){
		chungus = getChungus(chunk);
		if (isChungusFull(chungus))
			Chunguses_pop(chunguses, chungus);
		return chunk;
	}

	// No available ChunkInfo to allocate, aka chungus list is empty. Allocate new chungus
	chungus = allocateChungus();
	if (!chungus) return NULL;
	ChungusList_pushFront(chunguses, chungus);

	// Note: no need to check whether the chungus is full here :)
	chunk = findFreeChunk_chungusList(chunguses);
	assert(chunk); // should not be able to fail
	return chunk;
}

static inline int countFreeChunkInChungus(struct Chungus* chungus){
	int free_chunks = 0;

	// Note: we don't care about the lasts bits of the bitmap always beeing set because invalid,
	// since we count clear bits and not set ones
	for (int i=0 ; i<CHUNGUS_BITMAP_SIZE ; i++){
		int n_bits_to_zero = 64 - __builtin_popcountll(chungus->bitmap[i]);
		free_chunks += n_bits_to_zero;
	}

	return free_chunks;
}

static bool shouldFreeChungus(struct ChungusList* chunguses, struct Chungus* chungus){
	// We can free a chungus if we got enough (>=target) free BlockInfo in the chunguses
	const int target_n_chunks = 3*PAGE_SIZE / (2*CHUNGUS_N_CHUNKS); // threshold: 1.5 fully empty chungus
	int n_free_chunks = 0;
	struct Chungus* cur = chunguses->head;

	do {
		if (cur != chungus)
			n_free_chunks += countFreeChunkInChungus(chungus);
		if (n_free_chunks >= target_n_chunks)
			return true;
		cur = cur->next;
	} while(cur->prev != cur);

	return false;
}

// ================ Allocations / freeing ================

static void* allocatePages(int n, bool clear){
	assert(n > 0);
	void* res;
	size_t size = n*PAGE_SIZE;

	#ifdef KERNEL
	physical_address_t addr = PMM_allocatePages(n);
	if (addr == 0) return NULL;
	res = (void*) VMM_toHeap(addr);
	#else
	res = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (res == NULL) return NULL;
	#endif

	if (clear)
		memset(res, 0, size);
	return res;
}

static void freePages(void* pages, int n){
	#ifdef KERNEL
	physical_address_t addr = VMM_toPhysical((virtual_address_t) pages);
	PMM_freePages(addr, n);
	#else
	munmap(pages, n*PAGE_SIZE);
	#endif
}

static int getSmallbucketOrder(size_t size){
	assert(size >= MIN_BLOCK_SIZE);

	// int power_of_two = 63 - __builtin_clzll(size); // floor(log2(size))
	int power_of_two = 64 - __builtin_clzll(size - 1); // ceil(log2(size))

	return power_of_two - SMALLBUCKETS_OFFSET;
}

static void* allocateSmallbucket(size_t size){
	assert(size < BIGBUCKET_THRESHOLD);
	void* res;

	size = max(size, MIN_BLOCK_SIZE);
	int order = getSmallbucketOrder(size);

	struct ChunkBucket* bucket = m_heap.smallBuckets + order;
	struct ChunkInfo* chunk = bucket->head;

	// We need to allocate a Chunk
	if (!chunk){
		chunk = getFreeChunk(&m_heap.partialChunguses);
		if (!chunk) return NULL;
		if (!allocateChunk(chunk, size))
			return NULL;
		if (!Hashmap_insert(&m_heap.allocMap, chunk->base, chunk)){
			freeChunk(chunk);
			return NULL;
		}
		Bucket_pushFront(bucket, chunk);
	}

	// Allocate a block in our chunk
	res = allocateBlock(chunk);
	if (isChunkFull(chunk))
		Bucket_pop(bucket, chunk);

	return res;
}

static void freeSmallbucket(struct HashmapEntry* entry, void* ptr){
	struct ChunkInfo* chunk = entry->value;
	int order = getSmallbucketOrder(chunk->size);
	struct ChunkBucket* bucket = m_heap.smallBuckets + order;

	bool add_to_bucket = isChunkFull(chunk);
	freeBlock(chunk, ptr);

	// Bucket was full, add it back to the allocatable buckets
	if (add_to_bucket){
		Bucket_pushFront(bucket, chunk);
		return;
	}

	// Emptyied the chunk, handle it
	if (isChunkEmpty(chunk) && shouldFreeChunk(bucket, chunk)){
		// Free chunk & update the hashmap
		Bucket_pop(bucket, chunk);
		freeChunk(chunk);
		Hashmap_delete(&m_heap.allocMap, entry);

		// Update the chungus
		struct Chungus* chungus = getChungus(chunk);
		bool was_full = isChungusFull(chungus);
		unsigned int bitmapIndex = getChunkInfoIndex(chunk);
		int bitmapMajorIndex = bitmapIndex / 64;
		int bitmapMinorIndex = bitmapIndex % 64;
		chungus->bitmap[bitmapMajorIndex] &= ~(0x8000000000000000 >> bitmapMinorIndex);

		if (was_full){
			ChungusList_pushFront(&m_heap.partialChunguses, chungus);
			return;
		}

		// Emptyied the chungus, handle it
		if (isChungusEmpty(chungus) && shouldFreeChungus(&m_heap.partialChunguses, chungus)){
			Chunguses_pop(&m_heap.partialChunguses, chungus);
			freeChungus(chungus);
		}
	}
}

static void* allocateLargebucket(size_t size){
	void* res;
	bool success;
	struct ChunkInfo* chunk;

	// First, try to reuse a cached big block
	for (int i=0 ; i<N_BIGBLOCKS_CACHED ; i++){
		chunk = &m_heap.freeCache[i];
		if (chunk->base == NULL || chunk->size < size)
			continue;
		// Fits ! :)
		success = Hashmap_insert(&m_heap.allocMap, chunk->base, chunk);
		if (!success) return NULL;
		res = chunk->base;
		chunk->base = NULL;
		return res;
	}

	chunk = getFreeChunk(&m_heap.partialChunguses);
	if (chunk == NULL) return NULL;
	success = allocateChunk(chunk, size);
	if (!success) return NULL;

	success = Hashmap_insert(&m_heap.allocMap, chunk->base, chunk);
	if (!success){
		freeChunk(chunk);
		return NULL;
	}

	return chunk->base;
}

static void freeLargebucket(struct HashmapEntry* entry){
	struct ChunkInfo* chunk;

	// Try to cache
	if (entry->value->size < BIGBLOCKS_CACHE_MAXSIZE){
		for (int i=0 ; i<N_BIGBLOCKS_CACHED ; i++){
			chunk = &m_heap.freeCache[i];
			if (chunk->base != NULL)
				continue;
			// Free slot found
			*chunk = *entry->value; // copy
			Hashmap_delete(&m_heap.allocMap, entry);
			return;
		}
	}

	// Otherwise, free it
	chunk = entry->value;
	freeChunk(chunk);
	Hashmap_delete(&m_heap.allocMap, entry);
}

// ================ Public interface ================

void* Heap_malloc(size_t size){
	if (size < BIGBUCKET_THRESHOLD)
		return allocateSmallbucket(size);
	else
		return allocateLargebucket(size);
}

void* Heap_calloc(size_t size){
	void* res = Heap_malloc(size);
	memset(res, 0, size);
	return res;
}

void Heap_free(void* ptr){
	struct HashmapEntry* res = Hashmap_find(&m_heap.allocMap, ptr);
	if (!res){
		fprintf(stderr, "Bogus pointer or double free detected !!\n");
		abort();
	}

	if (res->value->size < BIGBUCKET_THRESHOLD)
		freeSmallbucket(res, ptr);
	else
		return freeLargebucket(res);
}

void* Heap_realloc(void* ptr, size_t new_size){
	void* new_ptr;
	size_t old_size;
	if (ptr == NULL)
		return Heap_malloc(new_size);

	if (new_size == 0){
		Heap_free(ptr);
		return NULL;
	}

	struct HashmapEntry* map_entry = Hashmap_find(&m_heap.allocMap, ptr);
	if (!map_entry){
		fprintf(stderr, "Bogus pointer passed to realloc !!\n");
		abort();
	}
	old_size = map_entry->value->size;
	if (old_size >= new_size)
		return ptr;

	// Here we must realloc

	// Small allocation
	if (new_size < BIGBUCKET_THRESHOLD){
		new_ptr = allocateSmallbucket(new_size);
		if (new_ptr == NULL) return NULL;
		memcpy(new_ptr, ptr, old_size);
		freeSmallbucket(map_entry, ptr);
		return new_ptr;
	}

	// Large allocation
	// Note: We don't try to mmap next to the current region's end, because
	// when freeing we'd need to unmap both regions, so we'd need a way to know
	// how these regions were allocated. It's not worth the hastle
	new_ptr = allocateLargebucket(new_size);
	if (new_ptr == NULL) return NULL;
	memcpy(new_ptr, ptr, old_size);
	if (old_size < BIGBUCKET_THRESHOLD)
		freeSmallbucket(map_entry, ptr);
	else
		freeLargebucket(map_entry);

	return new_ptr;
}

void* Heap_reallocarray(void* ptr, size_t n, size_t size); // unimplemented
