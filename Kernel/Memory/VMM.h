#ifndef __VMM_H__
#define __VMM_H__

#include "Memory/Memory.h"
#include "HAL/Paging.h"

// VMM.h: Virtual Memory Manager

void VMM_initialize();

// Higher half direct map
void VMM_setHHDM(uint64_t offset);
virtual_address_t VMM_HHDM_physToVirt(physical_address_t addr);
physical_address_t VMM_HHDM_virtToPhys(virtual_address_t addr);

// Paging structure are mapped in their own virtual memory region
virtual_address_t VMM_pagingStructures_physToVirt(physical_address_t addr);

void VMM_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

/// @brief Store a mapping to be applied at paging intialization.
/// Call when VMM_intialize hasn't been called already.
/// @warning Can only be called so many times !
void VMM_premap(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags);

#endif
