#ifndef __PAGING_H__
#define __PAGING_H__

#include "Memory/Memory.h"

// THIS IS A STUB, DOESN'T WORK YET
void Paging_initialize(physical_address_t kernelPhys, virtual_address_t kernelVirt, size_t kSize, size_t virtualOffset);

#endif
