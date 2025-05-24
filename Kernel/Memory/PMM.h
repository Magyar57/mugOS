#ifndef __PMM_H__
#define __PMM_H__

#include "Memory/Memory.h"

// PMM.h: Physical Memory Manager

void PMM_initialize();
physical_address_t PMM_allocatePages(uint64_t n_pages);
void PMM_freePages(physical_address_t addr, uint64_t n_pages);
void PMM_printMemoryUsage();

#endif
