#include "assert.h"
#include "Panic.h"
#include "Logging.h"
#include "HAL/Paging.h"
#include "Preprocessor.h"

#include "Memory/VMM.h"
#define MODULE "Virtual memory manager"

// HHDM: Higher Half Direct mapping. Used only before initialization.
// Maps kernel code/data, framebuffers too
#define HHDM_BULLSHIT_VALUE 0xabcdeff00ffedcba
static uint64_t m_hhdmOffset = HHDM_BULLSHIT_VALUE;

// Special region mappings. They are mapped with an offset, which are the bits
// 0x0000f00000000000 ; these are reserved for this purpose.

// PSDM: Paging Structures Direct Mapping
static constexpr uint64_t m_psdmOffset = 0xffffe00000000000;
static constexpr uint64_t m_psdmSize = 0x0000100000000000; // 0xffffe00000000000 - 0xfffff00000000000
// HSDM: Heap Structures Direct Mapping
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

void VMM_initialize(){
	// Initialize the paging structures
	Paging_initializeTables();

	// Apply pre-mappings
	for (int i=0 ; i<m_nPremappings ; i++){
		VMM_map(m_premappings[i].phys, m_premappings[i].virt,
			m_premappings[i].n_pages, m_premappings[i].flags);
	}

	Paging_enable();
	m_pagingInitialized = true;
}

// ================ Map memory ================

void VMM_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags){
	return Paging_map(phys, virt, n_pages, flags);
}

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

virtual_address_t VMM_mapInHHDM(physical_address_t addr){
	assert(m_hhdmOffset != HHDM_BULLSHIT_VALUE);
	return addr + m_hhdmOffset;
}

virtual_address_t VMM_mapInHeap(physical_address_t addr, uint64_t n_pages, int flags){
	if (addr >= m_hsdmSize){
		log(PANIC, MODULE,
			"Heap structure at %p is higher the maximum supported (%p)", addr, toCanonical(m_psdmSize));
		panic();
	}

	virtual_address_t mapped_addr = addr + m_hsdmOffset;
	VMM_map(addr, mapped_addr, n_pages, flags);
	return mapped_addr;
}

virtual_address_t VMM_mapInPaging(physical_address_t addr, uint64_t n_pages, int flags){
	if (!m_pagingInitialized){
		return VMM_mapInHHDM(addr); // mapped in the bootloader
	}

	if (addr >= m_psdmSize){
		log(PANIC, MODULE,
			"Paging structure at %p is higher the maximum supported (%p)", addr, toCanonical(m_psdmSize));
		panic();
	}

	virtual_address_t mapped_addr = addr + m_psdmOffset;
	VMM_map(addr, mapped_addr, n_pages, flags);
	return mapped_addr;
}

// ================ Unmap memory ================

void VMM_unmap(virtual_address_t addr){
	log(PANIC, MODULE, "VMM_unmap unimplemented !! %p", addr);
	panic();
}

// ================ Physical -> Virtual ================

physical_address_t VMM_toPhysical(virtual_address_t addr){
	// Special regions: direct mappings, easy
	if (addr & (0xfffff00000000000)){
		return addr & ~0xfffff00000000000;
	}

	// Not special regions (mapped with VMM_map)
	// We need to find address in paging mappings
	log(PANIC, MODULE, "Calling VMM_toPhysical on VMM_map-ed region is unimplemented");
	panic();

	unreachable();
}

// ================ Virtual -> Physical ================

virtual_address_t VMM_toHHDM(physical_address_t addr){
	assert(m_hhdmOffset != HHDM_BULLSHIT_VALUE);
	return addr + m_hhdmOffset;
}

virtual_address_t VMM_toHeap(physical_address_t addr){
	return addr + m_hsdmOffset;
}

virtual_address_t VMM_toPaging(physical_address_t addr){
	if (!m_pagingInitialized){
		return VMM_toHHDM(addr); // mapped in the bootloader
	}

	return m_psdmOffset + addr;
}

// ================ Misc ================

void VMM_setHHDM(uint64_t offset){
	m_hhdmOffset = offset;
}
