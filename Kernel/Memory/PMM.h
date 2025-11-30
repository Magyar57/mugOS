#ifndef __PMM_H__
#define __PMM_H__

#include "Memory/Memory.h"

// PMM.h: Physical Memory Manager

void PMM_init();

/// @brief Allocate `n_pages` contiguous physical pages
/// @return The start address of the allocated block
paddr_t PMM_allocatePages(uint64_t n_pages);

/// @brief Free pages allocated by `PMM_allocatePages`
/// @param addr Address returned by `PMM_allocatePages`
/// @param n_pages Number of pages that were allocated (passed to `PMM_allocatePages`)
void PMM_freePages(paddr_t addr, uint64_t n_pages);

/// @brief Pretty-print the physical memory usage.
void PMM_printMemoryUsage();

/// @brief Print the number of pages in use. Useful for debugging
void PMM_printPagesUsage();

#endif
