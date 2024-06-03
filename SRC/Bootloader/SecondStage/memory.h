#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "stdint.h"

void far* memcpy(const void far* dst, const void far* src, size_t size);
void far* memset(void far* ptr, int value, size_t size);
int memcmp(const void far* ptr1, const void far* ptr2, size_t size);

#endif
