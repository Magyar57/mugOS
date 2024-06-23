#ifndef __STRING_H__
#define __STRING_H__

#include "stdio.h"

// Find a char "chr" in the string
// @return <z> pointer to the chr. NULL if not found
const char* strchr(const char* str, char chr);

// Copies the string src into a destination dst. The memory regions must NOT overlap
char* strcpy(char* dst, const char* src);

// strlen - Calculate the length of a string
// @return The number of bytes in the string pointed to by str
size_t strlen(const char* str);

void* memcpy(const void* dst, const void* src, size_t size);

void* memset(void* ptr, int value, size_t size);

int memcmp(const void* ptr1, const void* ptr2, size_t size);

#endif
