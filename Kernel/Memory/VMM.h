#ifndef __VMM_H__
#define __VMM_H__

#include "Memory/Memory.h"
#include "HAL/Paging.h"

// VMM.h: Virtual Memory Manager

// Virtual addresses in this region are garanteed to be kernel-only
// They can be used safely with VMM_map
#define VMM_KERNEL_MEMORY		toCanonical(1ul << (ADDRESS_SIZE-1))

void VMM_init();

// ================ Map memory ================

/// @brief Map physical memory to virtual memory
/// @param phys The starting address of the physical memory region
/// @param virt The starting address of the virtual memory region
/// @param n_pages Number of pages to map
/// @param flags Properties to give to the mapping ; see PAGE_* macros in `HAL/Paging.h`
void VMM_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

/// @brief Store a mapping to be applied at paging intialization ; use instead of VMM_map
/// when VMM_intialize hasn't been called already. See VMM_map for arguments description
/// @warning Can only be called so many times !
void VMM_premap(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

virtual_address_t VMM_mapInHHDM(physical_address_t addr);
virtual_address_t VMM_mapInHeap(physical_address_t addr, uint64_t n_pages, int flags);

// ================ Unmap memory ================

void VMM_unmap(virtual_address_t addr, uint64_t n_pages);

// ================ Physical -> Virtual ================

/// @brief Get the physical address from any virtual address
physical_address_t VMM_toPhysical(virtual_address_t addr);

// ================ Virtual -> Physical ================

// /// @brief Get virtual address from HHDM-mapped physical address
virtual_address_t VMM_toHHDM(physical_address_t addr);

/// @brief Get virtual address from heap-structures-region-mapped physical address
virtual_address_t VMM_toHeap(physical_address_t addr);

// ================ Misc ================

void VMM_setHHDM(uint64_t offset);

#endif
