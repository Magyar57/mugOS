// #include <stdint.h>

#include "Memory/VMM.h"

uint64_t m_kernelMemoryOffset = 0xffffffffffffffff; // init with invalid offset

void VMM_setKernelMemoryOffset(uint64_t offset){
	m_kernelMemoryOffset = offset;
}

virtual_address_t VMM_physicalToVirtual(physical_address_t addr){
	return addr + m_kernelMemoryOffset;
}
