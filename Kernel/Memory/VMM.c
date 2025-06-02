#include "assert.h"
#include "Panic.h"
#include "Logging.h"
#include "HAL/Paging.h"

#include "Memory/VMM.h"
#define MODULE "Virtual memory manager"

static uint64_t m_hhdmOffset = 0xffffffffffffffff; // init with invalid offset

// PSDM: Paging Structure Direct Mapping. They are mapped with an offset:
static constexpr uint64_t m_psdmOffset = 0xffffe00000000000;
static constexpr uint64_t m_psdmSize = 0x0000100000000000; // 0xffffe00000000000 - 0xfffff00000000000
// HDM: Heap Direct Mapping
static constexpr uint64_t m_hsdmOffset = 0xffffd00000000000;
static constexpr uint64_t m_hsdmSize = 0x0000100000000000; // 0xffffd00000000000 - 0xffffe00000000000

static bool m_pagingInitialized = false;

struct Mapping {
	physical_address_t phys;
	physical_address_t virt;
	uint64_t n_pages;
	int flags;
};

#define MAX_PREMAPPINGS 2
static struct Mapping m_premappings[MAX_PREMAPPINGS];
static int m_nPremappings = 0;

void VMM_premap(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags){
	if (m_nPremappings >= MAX_PREMAPPINGS){
		log(PANIC, MODULE, "Cannot pre-map more than %d times !!", MAX_PREMAPPINGS);
		panic();
	}

	m_premappings[m_nPremappings].phys = phys;
	m_premappings[m_nPremappings].virt = virt;
	m_premappings[m_nPremappings].n_pages = n_pages;
	m_premappings[m_nPremappings].flags = flags;
	m_nPremappings++;
}

void VMM_initialize(){
	// Initialize the paging structures
	Paging_initializeTables();

	// Apply pre-mappings
	for (int i=0 ; i<m_nPremappings ; i++){
		VMM_map(m_premappings[i].phys, m_premappings[i].virt,
			m_premappings[i].n_pages, m_premappings[i].flags);
	}

	// Paging structures
	VMM_map(0x0, m_psdmOffset, roundToPage(m_psdmSize), PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
	// Kernel heap
	VMM_map(0x0, m_hsdmOffset, roundToPage(m_hsdmSize), PAGE_READ|PAGE_WRITE|PAGE_KERNEL);

	Paging_enable();
	m_pagingInitialized = true;
}

void VMM_setHHDM(uint64_t offset){
	m_hhdmOffset = offset;
}

virtual_address_t VMM_HHDM_physToVirt(physical_address_t addr){
	return addr + m_hhdmOffset;
}

physical_address_t VMM_HHDM_virtToPhys(virtual_address_t addr){
	return addr - m_hhdmOffset;
}

virtual_address_t VMM_pagingStructures_physToVirt(physical_address_t addr){
	if (!m_pagingInitialized){
		return VMM_HHDM_physToVirt(addr); // mapped in the bootloader
	}

	if (addr >= m_psdmSize){
		log(PANIC, MODULE, "Paging structure at %p is higher the maximum supported (%p)", addr, toCanonical(m_psdmSize));
		panic();
	}

	return m_psdmOffset + addr;
}

virtual_address_t VMM_Heap_physToVirt(physical_address_t addr){
	return addr + m_hsdmOffset;
}

physical_address_t VMM_Heap_virtToPhys(virtual_address_t addr){
	return addr - m_hsdmOffset;
}

void VMM_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags){
	return Paging_map(phys, virt, n_pages, flags);
}
