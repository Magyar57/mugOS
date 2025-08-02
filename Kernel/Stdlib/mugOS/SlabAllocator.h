#ifndef __SLAB_ALLOCATOR_H__
#define __SLAB_ALLOCATOR_H__

#include <stddef.h>

// Object cache, with an optional constructor
typedef struct Cache cache_t;

// Object constructor: sets an object to its initialized state
typedef void (*ctor_t)(void* obj);

void SlabAllocator_init();

/// @brief Reap (reclaim) free memory. Use when memory is tight
void SlabAllocator_reapAndTear();

// ================ Cache ================

/// @brief Create a cache
/// @param name The human-readable name of the cache
/// @param objsize The size (in bytes) of objects in this cache
/// @param ctor A function pointer to an object constructor to be called on object creation. Nullable
/// @return A pointer to the allocated cache, `NULL` on error
cache_t* Cache_create(const char* name, size_t objsize, ctor_t ctor);

/// @brief Destroy/delete a cache
void Cache_destroy(cache_t* cache);

/// @brief Allocate an object from a cache
void* Cache_malloc(cache_t* cache);

/// @brief Free an object from a cache
void Cache_free(cache_t* cache, void* ptr);

// ================ kmalloc ================
#ifdef KERNEL

/// @brief Allocate an object of size `size` (up to `KMALLOC_MAX_CACHE_SIZE`)
/// @returns A pointer to valid heap memory, or `NULL` on error
void* kmalloc(size_t size);

/// @brief Free an object (allocated by kmalloc)
void kfree(void* ptr);

/// @brief Allocate an object of size `size`, and zero it
void* kcalloc(size_t size);

/// @brief Realloc a kmalloc pointer
void* krealloc(void* ptr, size_t new_size);

#endif // def KERNEL

#endif
