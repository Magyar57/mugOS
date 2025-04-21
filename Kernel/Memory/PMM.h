#ifndef __PMM_H__
#define __PMM_H__

#include "Memory/Memory.h"

// PMM.h: Physical Memory Manager

void PMM_initialize();
void* PMM_allocate(uint64_t n_pages);
void PMM_free(physical_address_t addr, uint64_t n_pages);
void PMM_printMemoryUsage();

#endif
