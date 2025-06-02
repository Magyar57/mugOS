#include "string.h"
#include "assert.h"
#include "Preprocessor.h"
#include "Panic.h"
#include "Logging.h"
#include "Memory/MemoryMap.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "GDT.h"
#include "HAL/CPU.h"
#include "Registers.h"

#include "HAL/Paging.h"
#define MODULE "Paging"

#define TABLE_SIZE		512	// Number of entries in the tables
#define ADDRESS_SIZE	48	// In bits ; aka MAXPHYADDR aka M

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

#define getIndexPML4(addr) 						( ( (virtual_address_t )addr & 0x0000ff8000000000) >> 39 )
#define getIndexPageDirectoryPointerTable(addr)	( ( (virtual_address_t )addr & 0x0000007fc0000000) >> 30 )
#define getIndexPageDirectory(addr)				( ( (virtual_address_t )addr & 0x000000003fe00000) >> 21 )
#define getIndexPageTable(addr)					( ( (virtual_address_t )addr & 0x00000000001ff000) >> 12 )

#define get4KBEntryAddress(addr)				( ( (physical_address_t)addr & 0x0000fffffffff000) >> 12 )
#define get2MBEntryAddress(addr)				( ( (physical_address_t)addr & 0x0000ffffffe00000) >> 21 )
#define get1GBEntryAddress(addr)				( ( (physical_address_t)addr & 0x0000ffffc0000000) >> 30 )
#define getTableEntryAddress(addr) 				get4KBEntryAddress(addr)

#define parseEntryAddress(entry_addr)			(void*) VMM_toPaging(entry_addr << 12)

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
	physical_address_t address : ADDRESS_SIZE-12;
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
	physical_address_t address : ADDRESS_SIZE-12;
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
	physical_address_t address : ADDRESS_SIZE-30; // @phys of the 1GB page (ADDRESS_SIZE-30 bits)
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
	physical_address_t address : ADDRESS_SIZE-12;
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
	physical_address_t address : ADDRESS_SIZE-21; // @phys of the 2MB page (ADDRESS_SIZE-21 bits)
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
	physical_address_t address : ADDRESS_SIZE-12; // @phys of the 4KB page (ADDRESS_SIZE-12 bits)
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

static bool m_has1GBPages = false;

aligned(PAGE_SIZE) struct PDTPDescriptor g_pml4[TABLE_SIZE];
compile_assert(sizeof(g_pml4) == PAGE_SIZE);

// Note:
// When the most restrictive bit applies, we set the most permissive
// rights in the tables, and the correct rights in the pages descriptors.
// -> Concerns bits R/W "writable", U/S "privilege", XD "executeDisabled"
// When the last level met applies, we do the opposite (set caching on)
// -> Concerns btis PWT "writeThrough", PCD "cacheDisabled"

// Paging.asm
bool setPML4(physical_address_t pml4);
void flushTLB(void* addr);

// ================ Paging_map ================

static void setPDPT(struct PDTPDescriptor* entry, physical_address_t address){
	assert(!entry->present);

	entry->present = true;
	entry->writable = true;
	entry->privilege = PRIVILEGE_USER;
	entry->writeThrough = false;
	entry->cacheDisabled = false;
	entry->accessed = false;
	entry->reserved0 = 0b0;
	entry->restart = false;
	entry->address = getTableEntryAddress(address);
	entry->reserved1 = 0b0000;
	entry->executeDisabled = false;
}

static void setPageDirectory(struct PageDirectoryDescriptor* entry, physical_address_t addr){
	assert(!entry->present);

	entry->present = true;
	entry->writable = true;
	entry->privilege = PRIVILEGE_USER;
	entry->writeThrough = false;
	entry->cacheDisabled = false;
	entry->accessed = false;
	entry->pageSize = 0;
	entry->restart = false;
	entry->address = getTableEntryAddress(addr);
	entry->reserved = 0b0000;
	entry->executeDisabled = false;
}

static void setPageTable(struct PageTableDescriptor* entry, physical_address_t addr){
	assert(!entry->present);

	entry->present = true;
	entry->writable = true;
	entry->privilege = PRIVILEGE_USER;
	entry->writeThrough = false;
	entry->cacheDisabled = false;
	entry->accessed = false;
	entry->pageSize = 0;
	entry->restart = false;
	entry->address = getTableEntryAddress(addr);
	entry->reserved = 0b0000;
	entry->executeDisabled = false;
}

static void set4KBPage(struct PageDescriptor4KB* entry, physical_address_t addr, int flags){
	assert(!entry->present); // prevent overrides (temp)

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

static void set2MBPage(struct PageDescriptor2MB* entry, physical_address_t addr, int flags){
	assert(!entry->present); // prevent overrides (temp)

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

static void set1GBPage(struct PageDescriptor1GB* entry, physical_address_t addr, int flags){
	assert(!entry->present); // prevent overrides (temp)
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

static physical_address_t allocatePageOrPanic(){
	physical_address_t res = PMM_allocatePages(1);
	if (!res){
		log(PANIC, MODULE, "Could not allocate necessary paging structure !");
		panic();
	}

	return res;
}

void Paging_map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags){
	physical_address_t page_phys, page_virt;
	physical_address_t phys_cur = phys;
	virtual_address_t virt_cur = virt;

	union PageDirectoryPointerTableEntry* cur_pdp;
	union PageDirectoryEntry* cur_pd;
	struct PageDescriptor4KB* cur_pt;

	uint64_t pages_remaining = n_pages; // in 4KB pages
	uint64_t mappable;

	while(pages_remaining > 0){
		uint64_t pml4_index = getIndexPML4(virt_cur);
		uint64_t pdp_index = getIndexPageDirectoryPointerTable(virt_cur);
		uint64_t pd_index = getIndexPageDirectory(virt_cur);
		uint64_t pt_index = getIndexPageTable(virt_cur);

		// PML4
		if (g_pml4[pml4_index].present == 0){
			page_phys = allocatePageOrPanic();
			page_virt = VMM_toPaging(page_phys);
			memset((void*) page_virt, 0, PAGE_SIZE);
			setPDPT(g_pml4+pml4_index, page_phys);
		}
		cur_pdp = parseEntryAddress(g_pml4[pml4_index].address);

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

		// Page directory pointer
		if (cur_pdp[pdp_index].pageDirectory.present == 0){
			page_phys = allocatePageOrPanic();
			page_virt = VMM_toPaging(page_phys);
			memset((void*) page_virt, 0, PAGE_SIZE);
			setPageDirectory(&cur_pdp[pdp_index].pageDirectory, page_phys);
		}
		cur_pd = parseEntryAddress(cur_pdp[pdp_index].pageDirectory.address);

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

		// Page directory
		if (cur_pd[pd_index].pageTable.present == 0){
			page_phys = allocatePageOrPanic();
			page_virt = VMM_toPaging(page_phys);
			memset((void*) page_virt, 0, PAGE_SIZE);
			setPageTable(&cur_pd[pd_index].pageTable, page_phys);
		}
		cur_pt = parseEntryAddress(cur_pd[pd_index].pageTable.address);

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

// ================ Paging initialization ================

static inline void mapKernel(){
	extern uint8_t LOAD_ADDRESS;
	extern uint8_t __text_start, __rodata_start, __data_start;
	physical_address_t kernel_phys = g_memoryMap.kernelAddress;
	virtual_address_t kernel_virt = (virtual_address_t) &LOAD_ADDRESS;

	// Compute size (in #pages) of kernel regions to map
	// Note: since the __end symbol from the linker map points before the actual end of the bss,
	// we use the size that the bootloader gave us instead to know the bss' size
	uint64_t text_size = (&__rodata_start - &__text_start) / PAGE_SIZE;
	uint64_t rodata_size = (&__data_start - &__rodata_start) / PAGE_SIZE;

	uint64_t data_size = (g_memoryMap.kernelSize/PAGE_SIZE - (rodata_size+text_size));
	physical_address_t ktext_phys = (physical_address_t)&__text_start - kernel_virt + kernel_phys;
	physical_address_t rodata_phys = (physical_address_t)&__rodata_start - kernel_virt + kernel_phys;
	physical_address_t data_phys = (physical_address_t)&__data_start - kernel_virt + kernel_phys;

	// .text section: r-x
	Paging_map(ktext_phys, (virtual_address_t) &__text_start, text_size, PAGE_READ|PAGE_EXEC|PAGE_KERNEL);
	// .rodata section: r--
	Paging_map(rodata_phys, (virtual_address_t) &__rodata_start, rodata_size, PAGE_READ|PAGE_KERNEL);
	// .data and .bss sections: rw-
	Paging_map(data_phys, (virtual_address_t) &__data_start, data_size, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
}

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

void Paging_initializeTables(){
	// Clear the PML4 (sets all entries to invalid)
	memset(g_pml4, 0, PAGE_SIZE);

	// Assert that we have the features we need, and enable them
	initializeFeatures();

	// Map the kernel
	mapKernel();

	// Map the HHDM & framebuffer
	for(int i=0 ; i<g_memoryMap.size ; i++){
		struct MemoryMapEntry* cur = g_memoryMap.entries + i;
		uint64_t n_pages = (cur->length + PAGE_SIZE-1) / PAGE_SIZE; // round up
		switch (cur->type){
		case MEMORY_USABLE:
			// Paging_map(cur->address, VMM_HHDM_physToVirt(cur->address), n_pages, // TODO I WANT TO GET RID OF THIS
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
	physical_address_t kernel_phys = g_memoryMap.kernelAddress;
	virtual_address_t kernel_virt = (virtual_address_t) &LOAD_ADDRESS;

	// Now load our page table
	// Note: g_pml4 is not in the HHDM region, so we cannot use VMM_hhdm_virtualToPhysical
	// It is in the kernel data section, so we use the kernel code offset
	physical_address_t pml4_phys = kernel_phys + ((uint64_t)g_pml4 - kernel_virt);
	bool res = setPML4(pml4_phys);
	if (!res){
		log(PANIC, MODULE, "Could not set page table !!");
		panic();
	}

	log(SUCCESS, MODULE,
		"Kernel page table set successfully ! Higher Half Direct Map starts at %p, kernel at %p",
		VMM_toHHDM(0), kernel_virt);
}
