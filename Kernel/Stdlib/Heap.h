#ifndef __HEAP_H__
#define __HEAP_H__

#include <stddef.h>

void* Heap_malloc(size_t size);
void* Heap_calloc(size_t size);
void Heap_free(void* ptr);
void* Heap_realloc(void* ptr, size_t new_size);
// void* Heap_reallocarray(void* ptr, size_t n, size_t size); // unimplemented

#endif
