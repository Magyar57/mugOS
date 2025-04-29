#include "HAL/Paging.h"

#include "Memory/VMM.h"

uint64_t m_kernelMemoryOffset = 0xffffffffffffffff; // init with invalid offset

void VMM_initialize(){
	Paging_initialize();
}

void VMM_setKernelMemoryOffset(uint64_t offset){
	m_kernelMemoryOffset = offset;
}

uint64_t VMM_getKernelMemoryOffset(){
	return m_kernelMemoryOffset;
}

virtual_address_t VMM_physicalToVirtual(physical_address_t addr){
	return addr + m_kernelMemoryOffset;
}

physical_address_t VMM_virtualToPhysical(virtual_address_t addr){
	return addr - m_kernelMemoryOffset;
}
