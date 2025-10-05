#include "assert.h"
#include "Preprocessor.h"
#include "Panic.h"
#include "Logging.h"
#include "HAL/Memory/Paging.h"

#include "Memory/VMM.h"
#define MODULE "Virtual memory manager"

// HHDM: Higher Half Direct mapping. Used only before initialization.
// Maps kernel code/data, framebuffers too
#define HHDM_BULLSHIT_VALUE 0xabcdeff00ffedcba
static uint64_t m_hhdmOffset = HHDM_BULLSHIT_VALUE;

// Special region mappings. They are mapped with an offset, which are the bits
// 0x0000f00000000000 ; these are reserved for this purpose.

// HSDM: Heap Structures Direct Mapping
static constexpr uint64_t m_hsdmOffset = 0xffffd00000000000;
static constexpr uint64_t m_hsdmSize = 0x0000100000000000; // 0xffffd00000000000 - 0xffffe00000000000

static bool m_pagingInitialized = false;

struct Mapping {
	paddr_t phys;
	paddr_t virt;
	uint64_t n_pages;
	int flags;
};

#define MAX_PREMAPPINGS 2
static struct Mapping m_premappings[MAX_PREMAPPINGS];
static int m_nPremappings = 0;

void VMM_init(){
	// Initialize the paging structures
	Paging_initTables();

	// Apply pre-mappings
	for (int i=0 ; i<m_nPremappings ; i++){
		VMM_map(m_premappings[i].phys, m_premappings[i].virt,
			m_premappings[i].n_pages, m_premappings[i].flags);
	}

	Paging_enable();
	m_pagingInitialized = true;
}

// ================ Map memory ================

void VMM_map(paddr_t phys, vaddr_t virt, uint64_t n_pages, int flags){
	return Paging_map(phys, virt, n_pages, flags);
}

void VMM_premap(paddr_t phys, vaddr_t virt, uint64_t n_pages, int flags){
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

vaddr_t VMM_mapInHHDM(paddr_t addr){
	assert(m_hhdmOffset != HHDM_BULLSHIT_VALUE);
	return addr + m_hhdmOffset;
}

vaddr_t VMM_mapInHeap(paddr_t addr, uint64_t n_pages, int flags){
	if (addr >= m_hsdmSize){
		log(PANIC, MODULE,
			"Heap structure at %p is higher the maximum supported (%p)", addr, toCanonical(m_hsdmSize));
		panic();
	}

	vaddr_t mapped_addr = addr + m_hsdmOffset;
	VMM_map(addr, mapped_addr, n_pages, flags);
	return mapped_addr;
}

// ================ Unmap memory ================

void VMM_unmap(vaddr_t addr, uint64_t n_pages){
	return Paging_unmap(addr, n_pages);
}

// ================ Physical -> Virtual ================

paddr_t VMM_toPhysical(vaddr_t addr){
	// Special regions: direct mappings, easy
	if (addr & (0xfffff00000000000)){
		return addr & ~0xfffff00000000000;
	}

	// Not special regions (mapped with VMM_map)
	// We need to find address in paging mappings
	log(PANIC, MODULE, "Calling VMM_toPhysical on VMM_map-ed region is unimplemented");
	panic();
}

// ================ Virtual -> Physical ================

vaddr_t VMM_toHHDM(paddr_t addr){
	assert(m_hhdmOffset != HHDM_BULLSHIT_VALUE);
	return addr + m_hhdmOffset;
}

vaddr_t VMM_toHeap(paddr_t addr){
	return addr + m_hsdmOffset;
}

// ================ Misc ================

void VMM_setHHDM(uint64_t offset){
	m_hhdmOffset = offset;
}
