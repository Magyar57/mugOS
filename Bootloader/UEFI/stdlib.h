#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stdint.h>
#include "EFI/EFI.h"

// Stdlib.h: standard library functions
// Note: since UEFI provides so many functions, we don't need that much
// of a big stdlib, so everything has been put here

size_t strlen(const char* str);
int memcmp(const void* ptr1, const void* ptr2, size_t size);

void utoa(unsigned long long number, char* out, size_t n, uint8_t radix);
void itoa(long long input, char* out, size_t n, uint8_t radix);
EFI_STATUS puts_noCRLF(CHAR16* string);
EFI_STATUS puts(CHAR16* string);
void putNumber_noCRLF(long long number, uint8_t radix);
void putNumber(long long number, uint8_t radix);
void putNumberUnsigned_noCRLF(unsigned long long number, uint8_t radix);
void putNumberUnsigned(unsigned long long number, uint8_t radix);

#endif
