#ifndef __STDLIB_H__
#define __STDLIB_H__

#include "Heap.h"

#define EXIT_FAILURE -1
#define EXIT_SUCCESS 0

void abort();

static inline void* malloc(size_t size){
	return Heap_malloc(size);
}

static inline void* calloc(size_t size){
	return Heap_calloc(size);
}

static inline void free(void* ptr){
	return Heap_free(ptr);
}

static inline void* realloc(void* ptr, size_t new_size){
	return Heap_realloc(ptr, new_size);
}

#endif
