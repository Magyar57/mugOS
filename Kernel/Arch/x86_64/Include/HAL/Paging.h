#ifndef __PAGING_H__
#define __PAGING_H__

#include "Memory/Memory.h"

// Flags for the VMM_map/Paging_map method
#define PAGE_READ					0x00000000 // Protection flag: page is readable (default)
#define PAGE_WRITE					0x00000001 // Protection flag: page is writable
#define PAGE_EXEC					0x00000002 // Protection flag: page is executable
#define PAGE_KERNEL					0x00000000
#define PAGE_USER					0x00000004
#define PAGE_CACHE_DISABLED			0x00000008 // Map flag: cache is disabled
#define PAGE_CACHE_WRITETHROUGH		0x00000010 // Map flag: cache is write-through
#define PAGE_CACHE_WRITEBACK		0x00000000 // Map flag: cache is write-back (default)

/// @brief Initializes the tables (allocate, and map everything needed).
/// @note `Paging_map` becomes callable after tables have been prepared
void Paging_initializeTables();

/// @brief Actually loads CR3 with our root page table. Call `Paging_initializeTables` first !
void Paging_enable();

void Paging_unmap(virtual_address_t virt, uint64_t n_pages);

/// @brief Map `n_pages` pages from physical address `phys` to virtual `virt`.
/// See the `PAGE_`-prefixed macros for flag informations
void Paging_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

#endif
