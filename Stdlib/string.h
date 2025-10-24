#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>

/// @brief Finds a char "chr" in the string
/// @returns A pointer to the first occurrence of `chr` in the C string `str`. NULL if not found
char* strchr(const char* str, int chr);

/// @brief Copies the string `src` into a destination `dst`. The memory regions must NOT overlap
char* strcpy(char* dst, const char* src);

/// @brief Copies the string `src` into a destination `dst`, for maximum size of size.
//         Null character not copied if src is too long. The memory regions must NOT overlap.
char* strncpy(char* dst, const char* src, size_t size);

/// @brief Computes the length of a string
/// @returns The number of bytes in the string pointed to by `str`
size_t strlen(const char* str);

/// @brief Compares the strings s1 and s2
/// @returns 0 if string are equal, negative value if s1 is less than s2, positive value otherwise
int strncmp(const char* s1, const char* s2, size_t n);

/// @brief Copies `size` bytes from memory area `src` to `dest`. The memory regions must NOT overlap
/// @returns A pointer to `dest`
void* memcpy(void* dst, const void* src, size_t size);

/// @brief Fills the first `size` bytes of the memory pointed by `ptr` with the constant byte `value`
/// @returns A pointer to `ptr`
void* memset(void* ptr, int value, size_t size);

/// @brief Compares the first `size` bytes of the memory area `ptr1` and `ptr2`
/// @returns An integer less than, equal to or greater than zero if the first `size` bytes of `ptr1` is found
///          to be, respectively, less than, match, or greater than the first `size` bytes of `ptr2`
int memcmp(const void* ptr1, const void* ptr2, size_t size);

/// @brief Copies `size` bytes from memory area `src` to `dest`. The memory regions may overlap
/// @returns A pointer to `dest`
void* memmove(void* dest, const void* src, size_t size);

#endif
