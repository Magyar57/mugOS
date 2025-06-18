#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "assert.h"
#include "mugOS/List.h"
#include "Preprocessor.h"
#include "Memory/Memory.h"

#ifdef KERNEL
#include "Logging.h"
#include "Panic.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#else
#error "Implement a mmap-like system call"
#endif

#include "SlabAllocator.h"
#define MODULE "Slab allocator"

#define OBJ_ROUND						8 // in bytes
#define roundMultiple(val, power_of_2)	((val + power_of_2-1) & ~(power_of_2-1))

#define KMALLOC_N_CACHES				8 // log2(KMALLOC_MAX_CACHE_SIZE) - log2(KMALLOC_MIN_CACHE_SIZE) + 1
#define KMALLOC_CACHES_OFFSET			5 // = log2(KMALLOC_MIN_CACHE_SIZE)

#define SLAB_OFFSLAB_THRESHOLD			512 // Above, struct Slab is allocated in kmalloc caches
#define SLAB_ENDLIST_MARKER				0xC0C0A123
#define SLAB_TARGET_N_OBJ				16
#define getSlabFreelistSize(n_obj)		(n_obj*sizeof(uint32_t))

#define REAP_TARGET						16 // Perfect target for reaping memory, in #pages

// Slab management structures
struct Slab {
	struct Cache* owner;
	void* payload; // Actual objects. Points to the first obj
	int n_allocated; // current #allocated objects

	lnode_t slab_lnode;

	// Index of the first free element in freeObjects
	uint32_t firstFree;
	// Keeps track of free object in the slab with an array of variable size,
	// of length = #objects in the slab. Ends with 'SLAB_ENDLIST_MARKER'
	uint32_t* freeObjects;
};

// Object cache, with optional constructors and destructors
typedef struct Cache {
	char name[24];
	bool offslab;		// whether the slabs structures are kept on or off-slab
	size_t objSize;
	long n_pages;		// number of pages each slab spans
	int n_objects;		// objects per slab
	ctor_t constructor;	// function pointer to object's constructor

	list_t full_slabs;
	list_t partial_slabs;
	list_t empty_slabs;

	lnode_t cache_lnode;
} cache_t;

typedef void* key_t;
typedef struct Slab* value_t;

typedef struct HashmapEntry {
	key_t key;
	value_t value;
} entry_t;

typedef struct Hashmap {
	entry_t* entries;
	long totalEntries;
	long freeEntries;
} hashmap_t;

// Cache for allocating caches structs
static struct Cache m_cacheCache = {
	.name = "caches",
	.offslab = false,
	.objSize = sizeof(struct Cache),
	.n_pages = 1,
	.n_objects = (1*PAGE_SIZE - sizeof(struct Slab)) / (128 + getSlabFreelistSize(1)), // 30
	.constructor = NULL,

	.full_slabs = LIST_STATIC_INIT(m_cacheCache.full_slabs),
	.partial_slabs = LIST_STATIC_INIT(m_cacheCache.partial_slabs),
	.empty_slabs = LIST_STATIC_INIT(m_cacheCache.empty_slabs),
};
compile_assert(sizeof(cache_t) % OBJ_ROUND == 0);

static struct Cache m_kmallocCaches[KMALLOC_N_CACHES] = {
	{ .objSize =   32 },
	{ .objSize =   64 },
	{ .objSize =  128 },
	{ .objSize =  256 },
	{ .objSize =  512 },
	{ .objSize = 1024 },
	{ .objSize = 2048 },
	{ .objSize = 4096 },
};

static hashmap_t m_hashmap;
static list_t m_caches = LIST_STATIC_INIT(m_caches);

static void* allocatePages(long n, bool clear);
static void freePages(void* pages, long n);

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
	long n_pages = roundToPage(new_total * sizeof(entry_t));
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

// ================ Slabs ================

static struct Slab* allocateSlab(long n_pages, int n_objects, bool offslab){
	struct Slab* slab;
	const int free_list_size = getSlabFreelistSize(n_objects);

	if (offslab){
		slab = kmalloc(sizeof(struct Slab) + free_list_size);
		if (slab == NULL) return NULL;
		slab->payload = allocatePages(n_pages, false);
		if (slab->payload == NULL){
			kfree(slab);
			return NULL;
		}
	}
	else {
		slab = allocatePages(n_pages, false);
		if (slab == NULL) return NULL;
		slab->payload = (void*)(slab+1) + free_list_size;
	}

	slab->owner = NULL; // set by caller
	slab->n_allocated = 0;
	slab->firstFree = 0;
	slab->freeObjects = (void*)(slab+1);

	// Initialize free list
	for (int i=0 ; i<n_objects-1 ; i++)
		slab->freeObjects[i] = i+1;
	slab->freeObjects[n_objects-1] = SLAB_ENDLIST_MARKER;

	return slab;
}

static void freeSlab(struct Slab* slab, long n_pages, bool is_offslab){
	if (is_offslab){
		freePages(slab->payload, n_pages);
		kfree(slab);
	}
	else {
		freePages(slab, n_pages);
	}
}

static inline bool isSlabFull(struct Slab* slab){
	return (slab->firstFree == SLAB_ENDLIST_MARKER);
}

static inline bool isSlabEmpty(struct Slab* slab){
	return (slab->n_allocated == 0);
}

static void* allocateObject(struct Slab* slab){
	if (slab->firstFree == SLAB_ENDLIST_MARKER)
		return NULL;

	void* obj_ptr = slab->payload + slab->firstFree*slab->owner->objSize;
	slab->firstFree = slab->freeObjects[slab->firstFree];

	slab->n_allocated++;
	return obj_ptr;
}

static void freeObject(struct Slab* slab, void* obj_ptr){
	if (obj_ptr == NULL)
		return;

	uint32_t obj_index = (obj_ptr - slab->payload) / slab->owner->objSize;
	slab->freeObjects[obj_index] = slab->firstFree;
	slab->firstFree = obj_index;

	slab->n_allocated--;
}

// ================ Cache ================

static void initCache(struct Cache* cache, const char* name, size_t objsize, ctor_t ctor){
	bool offslab;
	long n_pages;
	int n_objects;
	assert(cache);

	objsize = roundMultiple(objsize, OBJ_ROUND);
	offslab = (objsize > SLAB_OFFSLAB_THRESHOLD);
	n_pages = (SLAB_TARGET_N_OBJ*objsize + PAGE_SIZE-1) / PAGE_SIZE;

	n_objects = (offslab) ? n_pages*PAGE_SIZE / objsize :
		(n_pages*PAGE_SIZE - sizeof(struct Slab)) / (objsize + getSlabFreelistSize(1));

	strncpy(cache->name, name, sizeof(cache->name)-1);
	cache->name[sizeof(cache->name)-1] = '\0';
	cache->offslab = offslab;
	cache->objSize = objsize;
	cache->n_pages = n_pages;
	cache->n_objects = n_objects;
	cache->constructor = ctor;

	List_init(&cache->full_slabs);
	List_init(&cache->partial_slabs);
	List_init(&cache->empty_slabs);
}

static bool growCache(cache_t* cache){
	assert(cache);
	bool success;

	struct Slab* new_slab = allocateSlab(cache->n_pages, cache->n_objects, cache->offslab);
	if (new_slab == NULL) return false;

	// Add entry for all pages to the hashmap
	void* obj_base = new_slab->payload;
	for (int i=0 ; i<cache->n_pages ; i++){
		success = Hashmap_insert(&m_hashmap, obj_base + i*PAGE_SIZE, new_slab);
		// On failure, undo all hashmap insertions
		if (!success){
			for (int j=0 ; j<i ; j++){
				entry_t* entry = Hashmap_find(&m_hashmap, obj_base + j*PAGE_SIZE);
				Hashmap_delete(&m_hashmap, entry);
			}
			return false;
		}
	}

	// Initializes all new objects
	if (cache->constructor != NULL){
		for (int i=0 ; i<cache->n_objects ; i++){
			void* cur_obj = new_slab->payload + i*cache->objSize;
			cache->constructor(cur_obj);
		}
	}

	new_slab->owner = cache;
	List_pushFront(&cache->empty_slabs, &new_slab->slab_lnode);
	return true;
}

static void freeCacheInSlab(struct Cache* cache, struct Slab* slab, void* ptr){
	bool was_full = false;

	was_full = isSlabFull(slab);
	freeObject(slab, ptr);

	if (was_full){
		List_pop(&cache->full_slabs, &slab->slab_lnode);
		List_pushFront(&cache->partial_slabs, &slab->slab_lnode);
	}
	else if (isSlabEmpty(slab)){
		List_pop(&cache->partial_slabs, &slab->slab_lnode);
		List_pushFront(&cache->empty_slabs, &slab->slab_lnode);
	}
}

static void removeCacheSlab(cache_t* cache, list_t* list, struct Slab* to_remove){
	entry_t* entry;
	long n_pages = cache->n_pages;
	bool offslab = cache->offslab;

	// Remove the slab from our structures
	List_pop(list, &to_remove->slab_lnode);
	for (int i=0 ; i<cache->n_pages ; i++){
		entry = Hashmap_find(&m_hashmap, to_remove->payload + i*PAGE_SIZE);
		if (entry != NULL) // Shouldn't happen
			Hashmap_delete(&m_hashmap, entry);
	}

	// Finally, we can free the pages
	freeSlab(to_remove, n_pages, offslab);
}

static void shrinkCache(cache_t* cache, int n_slabs){
	for (int i=0 ; i<n_slabs ; i++){
		struct Slab* to_remove = List_getObject(cache->empty_slabs.head, struct Slab, slab_lnode);
		removeCacheSlab(cache, &cache->empty_slabs, to_remove);
	}
}

static int getReapablePages(struct Cache* cache){
	int n_pages = 0;

	lnode_t* node;
	List_foreach(&cache->empty_slabs, node){
		n_pages += cache->n_pages;
	}

	if (cache->constructor != NULL)
		n_pages /= 2;

	return n_pages;
}

// ================ Misc ================

static void* allocatePages(long n, bool clear){
	if (n <= 0) return NULL;
	void* res;
	size_t size = n*PAGE_SIZE;

	physical_address_t addr = PMM_allocatePages(n);
	if (addr == 0) return NULL;
	res = (void*) VMM_mapInHeap(addr, n, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);

	if (clear)
		memset(res, 0, size);
	return res;
}

static void freePages(void* pages, long n){
	physical_address_t addr = VMM_toPhysical((virtual_address_t) pages);
	VMM_unmap((virtual_address_t)pages, n);
	PMM_freePages(addr, n);
}

static int getKmallocCache(size_t size){
	if (size <= KMALLOC_MIN_CACHE_SIZE)
		return 0;

	int power_of_two = 64 - __builtin_clzll(size - 1); // ceil(log2(size))
	return power_of_two - KMALLOC_CACHES_OFFSET;
}

// ================ Public cache API and kmalloc ================

void SlabAllocator_initialize(){
	char name[32];
	size_t size;

	// Note: m_caches and m_cacheCache.full/partial/empty_list are initialized
	// at compile time

	for (int i=0 ;i<KMALLOC_N_CACHES ; i++){
		size = m_kmallocCaches[i].objSize;
		snprintf(name, sizeof(name), "kmalloc-%lu", size);
		initCache(m_kmallocCaches+i, name, size, NULL);
		List_pushFront(&m_caches, &m_kmallocCaches[i].cache_lnode);
	}
}

void SlabAllocator_reapAndTear(){
	struct Cache* cur, *cache_to_reap = NULL;
	int cur_n_pages, to_reap_n_pages = 0;

	if (List_isEmpty(&m_caches))
		return;

	lnode_t* node;
	List_foreach(&m_caches, node){
		cur = List_getObject(node, struct Cache, cache_lnode);
		cur_n_pages = getReapablePages(cur);
		if (cur_n_pages > to_reap_n_pages){
			to_reap_n_pages = cur_n_pages;
			cache_to_reap = cur;
		}
	}

	if (cache_to_reap == NULL)
		return;

	if (to_reap_n_pages > REAP_TARGET)
		to_reap_n_pages /= 2;

	int n_slabs = to_reap_n_pages / cache_to_reap->n_pages;
	shrinkCache(cache_to_reap, n_slabs);
}

cache_t* Cache_create(const char* name, size_t objsize, ctor_t ctor){
	if (objsize == 0)
		return NULL;

	struct Cache* cache = Cache_malloc(&m_cacheCache);
	if (cache == NULL) return NULL;

	initCache(cache, name, objsize, ctor);
	List_pushFront(&m_caches, &cache->cache_lnode);
	return cache;
}

void Cache_destroy(cache_t* cache){
	if (cache == NULL) return;
	struct Slab* slab_to_free;

	// Delete all slabs
	while (!List_isEmpty(&cache->empty_slabs)){
		slab_to_free = List_getObject(cache->empty_slabs.head, struct Slab, slab_lnode);
		removeCacheSlab(cache, &cache->empty_slabs, slab_to_free);
	}
	while (!List_isEmpty(&cache->partial_slabs)){
		slab_to_free = List_getObject(cache->partial_slabs.head, struct Slab, slab_lnode);
		removeCacheSlab(cache, &cache->partial_slabs, slab_to_free);
	}
	while (!List_isEmpty(&cache->full_slabs)){
		slab_to_free = List_getObject(cache->full_slabs.head, struct Slab, slab_lnode);
		removeCacheSlab(cache, &cache->full_slabs, slab_to_free);
	}

	List_pop(&m_caches, &cache->cache_lnode);
	Cache_free(&m_cacheCache, cache);
}

void* Cache_malloc(cache_t* cache){
	struct Slab* slab;
	bool was_empty = false;

	if (cache == NULL) return NULL;

	// Allocate from partial slabs
	if (!List_isEmpty(&cache->partial_slabs)){
		slab = List_getObject(cache->partial_slabs.head, struct Slab, slab_lnode);
	}
	// Allocate from free slabs. Grow if necessary
	else {
		if (List_isEmpty(&cache->empty_slabs) && !growCache(cache))
			return NULL;
		was_empty = true;
		slab = List_getObject(cache->empty_slabs.head, struct Slab, slab_lnode);
	}

	void* res = allocateObject(slab);

	if (was_empty){
		List_pop(&cache->empty_slabs, &slab->slab_lnode);
		List_pushFront(&cache->partial_slabs, &slab->slab_lnode);
	}
	else if (isSlabFull(slab)){
		List_pop(&cache->partial_slabs, &slab->slab_lnode);
		List_pushFront(&cache->full_slabs, &slab->slab_lnode);
	}

	return res;
}

void Cache_free(cache_t* cache, void* ptr){
	assert(cache);

	entry_t* entry = Hashmap_find(&m_hashmap, ptr);
	if (entry == NULL) return;

	struct Slab* slab = entry->value;
	assert(slab->owner == cache);

	freeCacheInSlab(cache, slab, ptr);
}

#ifdef KERNEL

void* kmalloc(size_t size){
	void* res;

	if (size > KMALLOC_MAX_CACHE_SIZE)
		return NULL;

	// Get in which 'size' cache to allocate
	int index = getKmallocCache(size);
	cache_t* cache = m_kmallocCaches + index;

	res = Cache_malloc(cache);

	return res;
}

void kfree(void* ptr){
	if (ptr == NULL)
		return;

	entry_t* entry = Hashmap_find(&m_hashmap, ptr);
	if (entry == NULL){
		log(PANIC, MODULE, "Bogus pointer passed to kfree !");
		panic();
	}

	struct Slab* slab = entry->value;
	struct Cache* cache = entry->value->owner;

	// Get in which 'size' cache it was allocated
	int index = getKmallocCache(cache->objSize);
	struct Cache* kmalloc_cache = m_kmallocCaches + index;
	assert(cache == kmalloc_cache);

	freeCacheInSlab(kmalloc_cache, slab, ptr);
}

void* kcalloc(size_t size){
	void* res = kmalloc(size);
	memset(res, 0, size);
	return res;
}

void* krealloc(void* ptr, size_t new_size){
	void* new_ptr;
	size_t old_size;
	if (ptr == NULL)
		return kmalloc(new_size);

	if (new_size == 0){
		kfree(ptr);
		return NULL;
	}

	struct HashmapEntry* map_entry = Hashmap_find(&m_hashmap, ptr);
	if (!map_entry){
		log(PANIC, MODULE, "Bogus pointer passed to realloc !");
		panic();
	}
	old_size = map_entry->value->owner->objSize;
	if (old_size >= new_size)
		return ptr;

	// Here we must realloc

	new_ptr = kmalloc(new_size);
	if (new_ptr == NULL) return NULL;
	memcpy(new_ptr, ptr, old_size);
	kfree(ptr);

	return new_ptr;
}

#endif
