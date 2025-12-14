#include "string.h"
#include "assert.h"
#include "mugOS/Preprocessor.h"
#include "Panic.h"
#include "Logging.h"
#include "Memory/MemoryMap.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "CPU/CPU.h"
#include "CPU/Registers.h"
#include "Platform/GDT.h"

#include "HAL/Memory/Paging.h"
#define MODULE "Paging"

#define TABLE_SIZE		512	// Number of entries in the tables

#pragma region "Paging structures"

// 64-bit paging structures (each table has 512 entries):
// -> PageMapLevel5 (PML5) [optional]
//    -> PML4
//       -> PageDirectoryPointerTable (PDPT)
//          -> PageDirectoryTable (PDT) or 1GB page
//             -> PageTable or 2MB page
//                -> 4KB page

// 4 levels paging => 48 bits virtual addresses (512^4 * 4096 == 2**48)
// 5 levels paging => 57 bits virtual addresses (512^5 * 4096 == 2**57)

// Indexing (when referencing a 4KB page ; that's the getIndex... maccros):
// Virtual address = [ [ PML4 ] [ Directory Ptr ] [ Directory ] [ Table ] [ Offset ] ]
//            bits = [ 47    39 38             30 29         21 20     12 11       0 ]

#define getIndexPML4(addr) 						( ( (vaddr_t )addr & 0x0000ff8000000000) >> 39 )
#define getIndexPageDirectoryPointerTable(addr)	( ( (vaddr_t )addr & 0x0000007fc0000000) >> 30 )
#define getIndexPageDirectory(addr)				( ( (vaddr_t )addr & 0x000000003fe00000) >> 21 )
#define getIndexPageTable(addr)					( ( (vaddr_t )addr & 0x00000000001ff000) >> 12 )

#define get4KBEntryAddress(addr)				( ( (paddr_t)addr & 0x0000fffffffff000) >> 12 )
#define get2MBEntryAddress(addr)				( ( (paddr_t)addr & 0x0000ffffffe00000) >> 21 )
#define get1GBEntryAddress(addr)				( ( (paddr_t)addr & 0x0000ffffc0000000) >> 30 )
#define getTableEntryAddress(addr) 				get4KBEntryAddress(addr)

#define getEntryAddress(entry_addr)				((entry_addr) << 12)

#define PRIVILEGE_KERNEL 0
#define PRIVILEGE_USER 1

// PML5 table entry, references a PML4 table
struct PML4Descriptor {
	uint64_t unimplemented;
} packed;

// PML4 table entry, references a PageDirectoryPointerTable
struct PDTPDescriptor {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t ignored0 : 1;
	uint64_t reserved0 : 1;
	uint64_t ignored1 : 3;
	uint64_t restart : 1; // Used if using HLAT paging
	paddr_t address : ADDRESS_SIZE-12;
	uint64_t reserved1 : 52-ADDRESS_SIZE;
	uint64_t ignored2 : 11;
	uint64_t executeDisabled : 1;
} packed;

// PDPT entry, that references a page directory
struct PageDirectoryDescriptor {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t ignored0 : 1;
	uint64_t pageSize : 1; // Must be zero (otherwise, this would be a PageDescriptor1GB)
	uint64_t ignored1 : 3;
	uint64_t restart : 1; // Used if using HLAT paging
	paddr_t address : ADDRESS_SIZE-12;
	uint64_t reserved : 52-ADDRESS_SIZE;
	uint64_t ignored2 : 11;
	uint64_t executeDisabled : 1;
} packed;

// Page directory pointer table entry, that references a 1GB page
struct PageDescriptor1GB {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pageSize : 1; // Must be one (otherwise, this would be a PageDirectoryDescriptor)
	uint64_t global : 1;
	uint64_t ignored0 : 2;
	uint64_t restart : 1; // Used if using HLAT paging
	uint64_t pat : 1;
	uint64_t reserved0 : 17;
	paddr_t address : ADDRESS_SIZE-30; // @phys of the 1GB page (ADDRESS_SIZE-30 bits)
	uint64_t reserved1 : 52-ADDRESS_SIZE;
	uint64_t ignored1 : 7;
	uint64_t protectionKey : 4;
	uint64_t executeDisabled : 1;
} packed;

// Page directory pointer table entry, that references either a page directory or a 1GB page
union PageDirectoryPointerTableEntry {
	struct PageDirectoryDescriptor pageDirectory;
	struct PageDescriptor1GB page1GB;
} packed;

// Page directory entry, that references a Page Table
struct PageTableDescriptor {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t ignored0 : 1;
	uint64_t pageSize : 1; // Must be zero (otherwise, this would be a PageDescriptor2MB)
	uint64_t ignored1 : 3;
	uint64_t restart : 1; // Used if using HLAT paging
	paddr_t address : ADDRESS_SIZE-12;
	uint64_t reserved : 52-ADDRESS_SIZE;
	uint64_t ignored2 : 11;
	uint64_t executeDisabled : 1;
} packed;

// Page directory entry, that references a 2MB page
struct PageDescriptor2MB {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pageSize : 1; // Must be one (otherwise, this would be a PageTableDescriptor)
	uint64_t global : 1;
	uint64_t ignored0 : 2;
	uint64_t restart : 1; // Used if using HLAT paging
	uint64_t pat : 1;
	uint64_t reserved0 : 8;
	paddr_t address : ADDRESS_SIZE-21; // @phys of the 2MB page (ADDRESS_SIZE-21 bits)
	uint64_t reserved1 : 52-ADDRESS_SIZE;
	uint64_t ignored1 : 7;
	uint64_t protectionKey : 4;
	uint64_t executeDisabled : 1;
} packed;

// Page directory entry, references either a page table or a 2MB page
union PageDirectoryEntry {
	struct PageTableDescriptor pageTable;
	struct PageDescriptor2MB page2MB;
} packed;

// Page table entry, references a 4KB Page
struct PageDescriptor4KB {
	uint64_t present : 1;
	uint64_t writable : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pat: 1;
	uint64_t global : 1;
	uint64_t ignored0 : 2;
	uint64_t restart : 1; // Used if HLAT set
	paddr_t address : ADDRESS_SIZE-12; // @phys of the 4KB page (ADDRESS_SIZE-12 bits)
	uint64_t reserved : 52-ADDRESS_SIZE;
	uint64_t ignored1 : 7;
	uint64_t protectionKey : 4; // Used if CR4.PKE or CR4.PKS are set
	uint64_t executeDisabled : 1; // Used if IA32_EFER.NXE set
};

compile_assert(sizeof(struct PML4Descriptor) == 8);
compile_assert(sizeof(struct PDTPDescriptor) == 8);
compile_assert(sizeof(union PageDirectoryPointerTableEntry) == 8);
compile_assert(sizeof(union PageDirectoryEntry) == 8);
compile_assert(sizeof(struct PageDescriptor4KB) == 8);

#pragma endregion

static bool m_enabled = false;
static bool m_has1GBPages = false;

aligned(PAGE_SIZE) static struct PDTPDescriptor m_pml4[TABLE_SIZE];
compile_assert(sizeof(m_pml4) == PAGE_SIZE);

// Note:
// When the most restrictive bit applies, we set the most permissive
// rights in the tables, and the correct rights in the pages descriptors.
// -> Concerns bits R/W "writable", U/S "privilege", XD "executeDisabled"
// When the last level met applies, we do the opposite (set caching on)
// -> Concerns bits PWT "writeThrough", PCD "cacheDisabled"

// Paging.asm
bool setPML4(paddr_t pml4);
void flushTLB(void* addr);

// ================ Paging_map ================

// Dynamic dynamic tables.
// Those are used by Paging_map when needing to access/modify tables.
// We have 3 pages permanently map, that are switched dynamically to a table
// when said table needs to be accessed. It avoids mapping permanenlty all tables :)
static constexpr vaddr_t m_dynamicPDPT = 0xffffffffffffd000;
static constexpr vaddr_t m_dynamicPD   = 0xffffffffffffe000;
static constexpr vaddr_t m_dynamicPT   = 0xfffffffffffff000;
// PageTable in which the dynamic tables are mapped in (for dynamically mapping said tables)
static struct PageTableDescriptor* m_dynamicTablesHost = (void*) 0xffffffffffffc000;

static inline void* mapDynamicPDPT(paddr_t addr){
	if (!m_enabled)
		return (void*) VMM_mapInHHDM(addr);

	m_dynamicTablesHost[509].address = getTableEntryAddress(addr);
	flushTLB((void*)m_dynamicPDPT);
	return (void*) m_dynamicPDPT;
}

static inline void* mapDynamicPD(paddr_t addr){
	if (!m_enabled)
		return (void*) VMM_mapInHHDM(addr);

	m_dynamicTablesHost[510].address = getTableEntryAddress(addr);
	flushTLB((void*)m_dynamicPD);
	return (void*) m_dynamicPD;
}

static inline void* mapDynamicPT(paddr_t addr){
	if (!m_enabled)
		return (void*) VMM_mapInHHDM(addr);

	m_dynamicTablesHost[511].address = getTableEntryAddress(addr);
	flushTLB((void*)m_dynamicPT);
	return (void*) m_dynamicPT;
}

static void set4KBPage(struct PageDescriptor4KB* entry, paddr_t addr, int flags){
	assert(!entry->present);

	entry->present = true;
	entry->writable = ((flags & PAGE_WRITE) != 0);
	entry->privilege = ((flags & PAGE_USER) != 0);
	entry->writeThrough = ((flags & PAGE_CACHE_WRITETHROUGH) != 0);
	entry->cacheDisabled = ((flags & PAGE_CACHE_DISABLED) != 0);
	entry->accessed = 0;
	entry->dirty = 0;
	entry->pat = 0;
	entry->global = false;
	entry->restart = false;
	entry->address = get4KBEntryAddress(addr);
	entry->reserved = 0b0000;
	entry->protectionKey = 0b0000;
	entry->executeDisabled = !(flags & PAGE_EXEC);
}

static void set2MBPage(struct PageDescriptor2MB* entry, paddr_t addr, int flags){
	assert(!entry->present);

	entry->present = true;
	entry->writable = ((flags & PAGE_WRITE) != 0);
	entry->privilege = ((flags & PAGE_USER) != 0);
	entry->writeThrough = ((flags & PAGE_CACHE_WRITETHROUGH) != 0);
	entry->cacheDisabled = ((flags & PAGE_CACHE_DISABLED) != 0);
	entry->accessed = 0;
	entry->dirty = 0;
	entry->pageSize = 1;
	entry->global = false;
	entry->restart = false;
	entry->pat = 0;
	entry->reserved0 = 0;
	entry->address = get2MBEntryAddress(addr);
	entry->reserved1 = 0;
	entry->protectionKey = 0b0000;
	entry->executeDisabled = !(flags & PAGE_EXEC);
}

static void set1GBPage(struct PageDescriptor1GB* entry, paddr_t addr, int flags){
	assert(!entry->present);
	assert(m_has1GBPages);

	entry->present = true;
	entry->writable = ((flags & PAGE_WRITE) != 0);
	entry->privilege = ((flags & PAGE_USER) != 0);
	entry->writeThrough = ((flags & PAGE_CACHE_WRITETHROUGH) != 0);
	entry->cacheDisabled = ((flags & PAGE_CACHE_DISABLED) != 0);
	entry->accessed = 0;
	entry->dirty = 0;
	entry->pageSize = 1;
	entry->global = false;
	entry->restart = false;
	entry->pat = 0;
	entry->reserved0 = 0;
	entry->address = get1GBEntryAddress(addr);
	entry->reserved1 = 0;
	entry->protectionKey = 0b0000;
	entry->executeDisabled = !(flags & PAGE_EXEC);
}

static paddr_t allocatePageOrPanic(){
	paddr_t res = PMM_allocatePages(1);
	if (!res){
		log(PANIC, MODULE, "Could not allocate necessary paging structure !");
		panic();
	}

	return res;
}

static inline union PageDirectoryPointerTableEntry* getPDP(struct PDTPDescriptor* entry, bool alloc){
	union PageDirectoryPointerTableEntry* pdp;
	paddr_t page_phys;

	if (entry->present)
		return mapDynamicPDPT(getEntryAddress(entry->address));

	if (!alloc){
		log(PANIC, MODULE, "Tried to retrieve a non-present page directory pointer table");
		panic();
	}

	// Allocate a new table
	page_phys = allocatePageOrPanic();
	pdp = mapDynamicPDPT(page_phys);
	memset(pdp, 0, PAGE_SIZE);

	entry->present = true;
	entry->writable = true;
	entry->privilege = PRIVILEGE_USER;
	entry->writeThrough = false;
	entry->cacheDisabled = false;
	entry->accessed = false;
	entry->reserved0 = 0b0;
	entry->restart = false;
	entry->address = getTableEntryAddress(page_phys);
	entry->reserved1 = 0b0000;
	entry->executeDisabled = false;

	return pdp;
}

static inline union PageDirectoryEntry* getPD(union PageDirectoryPointerTableEntry* entry, bool alloc){
	union PageDirectoryEntry* pd;
	paddr_t page_phys;

	if (entry->pageDirectory.present)
		return mapDynamicPD(getEntryAddress(entry->pageDirectory.address));

	if (!alloc){
		log(PANIC, MODULE, "Tried to retrieve a non-present page directory");
		panic();
	}

	// Allocate a new table
	page_phys = allocatePageOrPanic();
	pd = mapDynamicPD(page_phys);
	memset(pd, 0, PAGE_SIZE);

	entry->pageDirectory.present = true;
	entry->pageDirectory.writable = true;
	entry->pageDirectory.privilege = PRIVILEGE_USER;
	entry->pageDirectory.writeThrough = false;
	entry->pageDirectory.cacheDisabled = false;
	entry->pageDirectory.accessed = false;
	entry->pageDirectory.pageSize = 0;
	entry->pageDirectory.restart = false;
	entry->pageDirectory.address = getTableEntryAddress(page_phys);
	entry->pageDirectory.reserved = 0b0000;
	entry->pageDirectory.executeDisabled = false;

	return pd;
}

static inline struct PageDescriptor4KB* getPT(union PageDirectoryEntry* entry, bool alloc){
	struct PageDescriptor4KB* pt;
	paddr_t page_phys;

	if (entry->pageTable.present)
		return mapDynamicPT(getEntryAddress(entry->pageTable.address));

	if (!alloc){
		log(PANIC, MODULE, "Tried to retrieve a non-present page table");
		panic();
	}

	// Allocate a new table
	page_phys = allocatePageOrPanic();
	pt = mapDynamicPT(page_phys);
	memset(pt, 0, PAGE_SIZE);

	entry->pageTable.present = true;
	entry->pageTable.writable = true;
	entry->pageTable.privilege = PRIVILEGE_USER;
	entry->pageTable.writeThrough = false;
	entry->pageTable.cacheDisabled = false;
	entry->pageTable.accessed = false;
	entry->pageTable.pageSize = 0;
	entry->pageTable.restart = false;
	entry->pageTable.address = getTableEntryAddress(page_phys);
	entry->pageTable.reserved = 0b0000;
	entry->pageTable.executeDisabled = false;

	return pt;
}

void Paging_map(paddr_t phys, vaddr_t virt, uint64_t n_pages, int flags){
	union PageDirectoryPointerTableEntry* cur_pdp;
	union PageDirectoryEntry* cur_pd;
	struct PageDescriptor4KB* cur_pt;
	paddr_t phys_cur = phys;
	vaddr_t virt_cur = virt;

	uint64_t mappable;
	uint64_t pages_remaining = n_pages; // in 4KB pages

	while (pages_remaining > 0){
		uint64_t pml4_index = getIndexPML4(virt_cur);
		uint64_t pdp_index = getIndexPageDirectoryPointerTable(virt_cur);
		uint64_t pd_index = getIndexPageDirectory(virt_cur);
		uint64_t pt_index = getIndexPageTable(virt_cur);

		// Get PDP in the PML4
		cur_pdp = getPDP(m_pml4 + pml4_index, true);

		// Try to map as 1GB pages (if addr is 1GB aligned AND we have more than 1GB to map)
		if (m_has1GBPages && phys_cur % SIZE_1GB == 0 && pages_remaining >= SIZE_1GB/PAGE_SIZE){
			mappable = min(TABLE_SIZE - pdp_index, pages_remaining*SIZE_4KB / SIZE_1GB);
			for (uint64_t i=0 ; i<mappable ; i++){
				set1GBPage(&cur_pdp->page1GB+pdp_index+i, phys_cur, flags);
				flushTLB((void*) virt_cur);
				phys_cur += SIZE_1GB;
				virt_cur += SIZE_1GB;
			}
			pages_remaining -= mappable * SIZE_1GB/PAGE_SIZE;
			continue;
		}

		// Get Page Directory in PDP
		cur_pd = getPD(cur_pdp + pdp_index, true);

		// Try to map as 2MB pages
		if (phys_cur % SIZE_2MB == 0 && pages_remaining >= SIZE_2MB/PAGE_SIZE){
			mappable = min(TABLE_SIZE - pd_index, pages_remaining*SIZE_4KB / SIZE_2MB);
			for (uint64_t i=0 ; i<mappable ; i++){
				set2MBPage(&cur_pd->page2MB+pd_index+i, phys_cur, flags);
				flushTLB((void*) virt_cur);
				phys_cur += SIZE_2MB;
				virt_cur += SIZE_2MB;
			}
			pages_remaining -= mappable * SIZE_2MB/PAGE_SIZE;
			continue;
		}

		// Get Page Table in Page Directory
		cur_pt = getPT(cur_pd + pd_index, true);

		// Map 4KB pages in the page table
		mappable = min(TABLE_SIZE - pt_index, pages_remaining);
		for (uint64_t i=0 ; i<mappable ; i++){
			set4KBPage(cur_pt+pt_index+i, phys_cur, flags);
			flushTLB((void*) virt_cur);
			phys_cur += PAGE_SIZE;
			virt_cur += PAGE_SIZE;
		}
		pages_remaining -= mappable;
	}
}

// ================ Paging_unmap ================

static inline bool tableIsEmpty(void* table){
	struct PageDescriptor4KB* polymorphic_table = table;

	for (int i=0 ; i<TABLE_SIZE ; i++)
		if (polymorphic_table[i].present)
			return false;

	return true;
}

static void freeTable(void* table){
	struct PageDescriptor4KB* polymorphic_table = table;
	polymorphic_table->present = false;

	paddr_t phys = getEntryAddress(polymorphic_table->address);
	PMM_freePages(phys, 1);
}

void Paging_unmap(vaddr_t virt, uint64_t n_pages){
	union PageDirectoryPointerTableEntry* cur_pdp;
	union PageDirectoryEntry* cur_pd;
	struct PageDescriptor4KB* cur_pt;
	vaddr_t virt_cur = virt;

	uint64_t removable;
	uint64_t pages_remaining = n_pages; // in 4KB pages

	while (pages_remaining > 0){
		uint64_t pml4_index = getIndexPML4(virt_cur);
		uint64_t pdp_index = getIndexPageDirectoryPointerTable(virt_cur);
		uint64_t pd_index = getIndexPageDirectory(virt_cur);
		uint64_t pt_index = getIndexPageTable(virt_cur);

		// Get PDP in the PML4
		cur_pdp = getPDP(m_pml4 + pml4_index, false);

		// Mapped as 1GB pages
		if (cur_pdp->page1GB.pageSize == 1){
			removable = min(TABLE_SIZE - pdp_index, pages_remaining*SIZE_4KB / SIZE_1GB);
			for (uint64_t i=0 ; i<removable ; i++){
				assert(cur_pdp[pdp_index+i].page1GB.present == true);
				cur_pdp[pdp_index+i].page1GB.present = false;
				flushTLB((void*) virt_cur);
				virt_cur += SIZE_1GB;
			}
			if (tableIsEmpty(cur_pdp))
				freeTable(m_pml4 + pml4_index);
			pages_remaining -= removable * SIZE_1GB/PAGE_SIZE;
			continue;
		}

		// Get Page Directory in PDP
		cur_pd = getPD(cur_pdp + pdp_index, false);

		// Mapped as 2MB pages
		if (cur_pd->page2MB.pageSize == 1){
			removable = min(TABLE_SIZE - pd_index, pages_remaining*SIZE_4KB / SIZE_2MB);
			for (uint64_t i=0 ; i<removable ; i++){
				assert(cur_pd[pd_index+i].page2MB.present == true);
				cur_pd[pd_index+i].page2MB.present = false;
				flushTLB((void*) virt_cur);
				virt_cur += SIZE_2MB;
			}
			if (tableIsEmpty(cur_pd)){
				freeTable(cur_pdp + pdp_index);
				if (tableIsEmpty(cur_pdp))
					freeTable(m_pml4 + pml4_index);
			}
			pages_remaining -= removable * SIZE_2MB/PAGE_SIZE;
			continue;
		}

		// Get Page Table in Page Directory
		cur_pt = getPT(cur_pd + pd_index, false);

		// Unmap the 4KB pages in the page table
		removable = min(TABLE_SIZE - pt_index, pages_remaining);
		for (uint64_t i=0 ; i<removable ; i++){
			assert(cur_pt[pt_index+i].present == true);
			cur_pt[pt_index+i].present = false;
			flushTLB((void*) virt_cur);
			virt_cur += PAGE_SIZE;
		}
		if (tableIsEmpty(cur_pt)){
			freeTable(cur_pd + pd_index);
			if (tableIsEmpty(cur_pd)){
				freeTable(cur_pdp + pdp_index);
				if (tableIsEmpty(cur_pdp))
					freeTable(m_pml4 + pml4_index);
			}
		}
		pages_remaining -= removable;
	}
}

// ================ Paging initialization ================

static void initializeFeatures(){
	union CR4 cr4;
	cr4.value = Registers_readCR4();
	union MSR_IA32_EFER efer;
	efer.value = Registers_readMSR(MSR_ADDR_IA32_EFER);

	// Assert that the address line is long enough
	if (g_CPU.extFeatures.bits.NumberOfLinearAddressBits < ADDRESS_SIZE){
		log(PANIC, MODULE, "Detected a CPU address size of %d, minimum supported is %d !",
			g_CPU.extFeatures.bits.NumberOfLinearAddressBits, ADDRESS_SIZE);
		panic();
	}

	// Restrain the CPU in kernel mode to execute/access kernel pages
	if (g_CPU.features.bits.SMAP)
		cr4.bits.SMAP = true;
	if (g_CPU.features.bits.SMEP)
		cr4.bits.SMEP = true;

	// Assert that the Execute-disable feature is available, and enable it
	if (!g_CPU.extFeatures.bits.NX)
		panicForMissingFeature("NX (mark pages as non executable)");
	else
		efer.bits.NXE = true;

	// Check if we can use 1GB pages
	m_has1GBPages = g_CPU.extFeatures.bits.PAGES_1GB;

	// Note: whether or not the global (G) bit (<=> PGE feature) is supported, we disable it
	// as we don't use it.
	// Note 2: feature presence is determined by `g_CPU.features.bits.PGE`
	cr4.bits.PGE = false;

	Registers_writeCR4(cr4.value);
	Registers_writeMSR(MSR_ADDR_IA32_EFER, efer.value);
}

static inline void mapKernel(){
	extern uint8_t LOAD_ADDRESS;
	extern uint8_t __text_start, __rodata_start, __data_start, __end;
	paddr_t kernel_phys = g_memoryMap.kernelAddress;
	vaddr_t kernel_virt = (vaddr_t) &LOAD_ADDRESS;

	// Compute size (in #pages) of kernel regions to map
	uint64_t text_size = (&__rodata_start - &__text_start) / PAGE_SIZE;
	uint64_t rodata_size = (&__data_start - &__rodata_start) / PAGE_SIZE;
	uint64_t data_size = (&__end - &__data_start) / PAGE_SIZE;

	// Compute addresses of sections
	paddr_t ktext_phys = (paddr_t)&__text_start - kernel_virt + kernel_phys;
	paddr_t rodata_phys = (paddr_t)&__rodata_start - kernel_virt + kernel_phys;
	paddr_t data_phys = (paddr_t)&__data_start - kernel_virt + kernel_phys;

	// .text section: r-x
	Paging_map(ktext_phys, (vaddr_t) &__text_start, text_size, PAGE_READ|PAGE_EXEC|PAGE_KERNEL);
	// .rodata section: r--
	Paging_map(rodata_phys, (vaddr_t) &__rodata_start, rodata_size, PAGE_READ|PAGE_KERNEL);
	// .data and .bss sections: rw-
	Paging_map(data_phys, (vaddr_t) &__data_start, data_size, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
}

void mapDynamicTables(){
	// Finally, we can prepare mappings for the temporary directories
	Paging_map(0xfffffffffffff000, m_dynamicPDPT, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
	Paging_map(0xfffffffffffff000, m_dynamicPD, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
	Paging_map(0xfffffffffffff000, m_dynamicPT, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);

	// Now get the physical address of the allocated page table for those last 3 pages
	// Note: if in the future, we happen to not have HHDM setup by the bootloader,
	// instead of calling Paging_map, we simply need to map manually the pages here to
	// get the page table physical address
	uint64_t pml4_index = getIndexPML4(m_dynamicPDPT);
	uint64_t pdp_index = getIndexPageDirectoryPointerTable(m_dynamicPDPT);
	uint64_t pd_index = getIndexPageDirectory(m_dynamicPDPT);
	struct PDTPDescriptor* pdpt = (void*) VMM_toHHDM(getEntryAddress(m_pml4[pml4_index].address));
	struct PageTableDescriptor* pd = (void*) VMM_toHHDM(getEntryAddress(pdpt[pdp_index].address));
	paddr_t pt_phys = getEntryAddress(pd[pd_index].address);

	// Now that we have our physical address, map it
	Paging_map(pt_phys, (vaddr_t) m_dynamicTablesHost, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
}

void Paging_initTables(){
	// Clear the PML4 (sets all entries to invalid)
	memset(m_pml4, 0, PAGE_SIZE);

	// Assert that we have the features we need, and enable them
	initializeFeatures();

	mapKernel();
	mapDynamicTables();

	// Map the HHDM & framebuffer
	for (int i=0 ; i<g_memoryMap.size ; i++){
		struct MemoryMapEntry* cur = g_memoryMap.entries + i;
		uint64_t n_pages = (cur->length + PAGE_SIZE-1) / PAGE_SIZE; // round up
		switch (cur->type){
		case MEMORY_USABLE:
			// HHDM (deprecated)
			// Paging_map(cur->address, VMM_toHHDM(cur->address), n_pages,
			// 	PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
			break;
		case MEMORY_RESERVED:
		case MEMORY_KERNEL:
			break;
		case MEMORY_FRAMEBUFFER:
			// Framebuffer is/are mapped in the HHDM
			Paging_map(cur->address, VMM_toHHDM(cur->address), n_pages,
				PAGE_READ|PAGE_WRITE|PAGE_KERNEL|PAGE_CACHE_DISABLED);
			break;
		case MEMORY_ACPI_NVS:
		case MEMORY_ACPI_RECLAIMABLE:
			break;
		case MEMORY_BOOTLOADER_RECLAIMABLE:
			Paging_map(cur->address, VMM_toHHDM(cur->address), n_pages,
				PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
			break;
		default:
			break;
		}
	}
}

void Paging_enable(){
	extern uint8_t LOAD_ADDRESS;
	paddr_t kphys = g_memoryMap.kernelAddress;
	vaddr_t kvirt = (vaddr_t) &LOAD_ADDRESS;

	// Now load our page table
	// Note: g_pml4 is not in the HHDM region, so we cannot use VMM_hhdm_virtualToPhysical
	// It is in the kernel data section, so we use the kernel code offset
	paddr_t pml4_phys = kphys + ((uint64_t)m_pml4 - kvirt);
	bool res = setPML4(pml4_phys);
	if (!res){
		log(PANIC, MODULE, "Could not set page table !!");
		panic();
	}

	m_enabled = true;

	log(SUCCESS, MODULE, "Kernel page table set successfully ! Kernel starts at %#lx", kvirt);
}
