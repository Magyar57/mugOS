#ifndef __VMM_H__
#define __VMM_H__

#include "Memory/Memory.h"
#include "HAL/Paging.h"

// VMM.h: Virtual Memory Manager

void VMM_initialize();

/// @brief Map physical memory to virtual memory
/// @param phys The starting address of the physical memory region
/// @param virt The starting address of the virtual memory region
/// @param n_pages Number of pages to map
/// @param flags Properties to give to the mapping ; see PAGE_* macros in `HAL/Paging.h`
void VMM_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

/// @brief Store a mapping to be applied at paging intialization ; use instead of VMM_map
// when VMM_intialize hasn't been called already. See VMM_map for arguments description
/// @warning Can only be called so many times !
void VMM_premap(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

// Mappings - translates physical <-> virtual

physical_address_t VMM_toPhysical(virtual_address_t addr);

virtual_address_t VMM_toHHDM(physical_address_t addr); // DON'T USE THIS
virtual_address_t VMM_toHeap(physical_address_t addr);
virtual_address_t VMM_toPaging(physical_address_t addr);

void VMM_setHHDM(uint64_t offset);

#endif
