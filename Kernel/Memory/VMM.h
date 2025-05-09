#ifndef __VMM_H__
#define __VMM_H__

#include "Memory/Memory.h"

// VMM.h: Virtual Memory Manager

void VMM_initialize();
void VMM_setKernelMemoryOffset(uint64_t offset);
uint64_t VMM_getKernelMemoryOffset();
virtual_address_t VMM_physicalToVirtual(physical_address_t addr);
physical_address_t VMM_virtualToPhysical(virtual_address_t addr);

#endif
