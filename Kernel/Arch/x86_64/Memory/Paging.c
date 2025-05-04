#include "string.h"
#include "assert.h"
#include "Preprocessor.h"
#include "Panic.h"
#include "Logging.h"
#include "Memory/MemoryMap.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "GDT.h"

#include "HAL/Paging.h"
#define MODULE "Paging"

#define TABLE_SIZE		512	// Number of entries in the tables
#define ADDRESS_SIZE	48	// In bits ; aka MAXPHYADDR aka M

#pragma region "Paging structures"

// 64-bit paging structures (each table has 512 entries):
// -> PageMapLevel5 (PML5) [optional]
//    -> PML4
//       -> PageDirectoryPointerTable
//          -> PageDirectory or 1GB page
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

#define PRIVILEGE_KERNEL 0
#define PRIVILEGE_USER 1

struct PageMapLevel5Entry {
	uint64_t unimplemented;
} packed;

struct PageMapLevel4Entry {
	uint64_t present : 1;
	uint64_t readWrite : 1;
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

// Page directory pointer table entry, that references a page directory
struct PageDirectoryPointerTableEntry_PageDirectory {
	uint64_t present : 1;
	uint64_t readWrite : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t ignored0 : 1;
	uint64_t pageSize : 1; // Must be zero (otherwise, this would be a PageDirectoryPointerTableEntry_Page1GB)
	uint64_t ignored1 : 3;
	uint64_t restart : 1; // Used if using HLAT paging
	physical_address_t address : ADDRESS_SIZE-12;
	uint64_t reserved : 52-ADDRESS_SIZE;
	uint64_t ignored2 : 11;
	uint64_t executeDisabled : 1;
} packed;

struct PageDirectoryPointerTableEntry_Page1GB {
	uint64_t present : 1;
	uint64_t readWrite : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pageSize : 1; // Must be one (otherwise, this would be a PageDirectoryPointerTableEntry_PageDirectory)
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
	struct PageDirectoryPointerTableEntry_PageDirectory pageDirectory;
	struct PageDirectoryPointerTableEntry_Page1GB page1GB;
} packed;

// Page directory entry, that references a Page Table
struct PageDirectoryEntry_PageTable {
	uint64_t present : 1;
	uint64_t readWrite : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t ignored0 : 1;
	uint64_t pageSize : 1; // Must be zero (otherwise, this would be a PageDirectoryEntry_Page2MB)
	uint64_t ignored1 : 3;
	uint64_t restart : 1; // Used if using HLAT paging
	physical_address_t address : ADDRESS_SIZE-12;
	uint64_t reserved : 52-ADDRESS_SIZE;
	uint64_t ignored2 : 11;
	uint64_t executeDisabled : 1;
} packed;

// Page directory entry, that references a 2MB page
struct PageDirectoryEntry_Page2MB {
	uint64_t present : 1;
	uint64_t readWrite : 1;
	uint64_t privilege : 1;
	uint64_t writeThrough : 1;
	uint64_t cacheDisabled : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pageSize : 1; // Must be one (otherwise, this would be a PageDirectoryEntry_PageTable)
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
	struct PageDirectoryEntry_PageTable pageTable;
	struct PageDirectoryEntry_Page2MB page2MB;
} packed;

// Page table entry, references a 4KB Page
struct PageTableEntry {
	uint64_t present : 1;
	uint64_t readWrite : 1;
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

compile_assert(sizeof(struct PageMapLevel5Entry) == 8);
compile_assert(sizeof(struct PageMapLevel4Entry) == 8);
compile_assert(sizeof(union PageDirectoryPointerTableEntry) == 8);
compile_assert(sizeof(union PageDirectoryEntry) == 8);
compile_assert(sizeof(struct PageTableEntry) == 8);

#pragma endregion

aligned(PAGE_SIZE) struct PageMapLevel4Entry g_pml4[TABLE_SIZE];
compile_assert(sizeof(g_pml4) == PAGE_SIZE);

void setPML4Entry(struct PageMapLevel4Entry* entry, physical_address_t address){
	entry->present = true;
	entry->readWrite = true;
	entry->privilege = PRIVILEGE_KERNEL;
	entry->writeThrough = 0;
	entry->cacheDisabled = false;
	entry->accessed = 0;
	entry->reserved0 = 0b0;
	entry->restart = 0;
	entry->address = getTableEntryAddress(address);
	entry->reserved1 = 0b0000;
	entry->executeDisabled = 0;
}

void setPageDirectoryPointerTableEntry(union PageDirectoryPointerTableEntry* entry, physical_address_t address){
	entry->pageDirectory.present = true;
	entry->pageDirectory.readWrite = true;
	entry->pageDirectory.privilege = PRIVILEGE_KERNEL;
	entry->pageDirectory.writeThrough = false;
	entry->pageDirectory.cacheDisabled = true;
	entry->pageDirectory.accessed = false;
	entry->pageDirectory.pageSize = 0; // points to a page directory
	entry->pageDirectory.restart = false;
	entry->pageDirectory.address = getTableEntryAddress(address);
	entry->pageDirectory.reserved = 0b0000;
	entry->pageDirectory.executeDisabled = false;
}

void setPageDirectoryEntry(union PageDirectoryEntry* entry, physical_address_t address){
	entry->pageTable.present = true;
	entry->pageTable.readWrite = true;
	entry->pageTable.privilege = PRIVILEGE_KERNEL;
	entry->pageTable.writeThrough = false;
	entry->pageTable.cacheDisabled = false;
	entry->pageTable.accessed = false;
	entry->pageTable.pageSize = 0;
	entry->pageTable.restart = false;
	entry->pageTable.address = getTableEntryAddress(address);
	entry->pageTable.reserved = 0b0000;
	entry->pageTable.executeDisabled = false;
}

void setPageTableEntry(struct PageTableEntry* entry, physical_address_t address, int flags){
	entry->present = true;
	entry->readWrite = ((flags & PAGE_WRITE) != 0);
	entry->privilege = ((flags & PAGE_USER) != 0);
	entry->writeThrough = ((flags & PAGE_CACHE_WRITETHROUGH) != 0);
	entry->cacheDisabled = ((flags & PAGE_CACHE_DISABLED) != 0);
	entry->accessed = 0;
	entry->dirty = 0;
	entry->pat = 0; // TODO add Page Attribute Table check & support
	entry->global = false;
	entry->restart = false;
	entry->address = get4KBEntryAddress(address);
	entry->reserved = 0b0000;
	entry->protectionKey = 0b0000;
	entry->executeDisabled = !(flags & PAGE_EXEC);
}

static physical_address_t allocatePageOrPanic(){
	void* tmp = PMM_allocate(1);
	if (!tmp){
		log(PANIC, MODULE, "Could not allocate necessary paging structure !");
		panic();
	}

	memset(tmp, 0, PAGE_SIZE);
	return VMM_virtualToPhysical((virtual_address_t)tmp);
}

static void map(physical_address_t phys, virtual_address_t virt, uint64_t n_pages, int flags){
	physical_address_t tmp;
	physical_address_t phys_cur = phys;
	virtual_address_t virt_cur = virt;

	// TODO replace with another algorithm:
	// - Map with 4KB pages until we get to an address multiple of 2MB `while (cur % SIZE_4KB)`
	// - Map with 2MB pages until we get to an address multiple of 1GB
	// - Map with 1GB pages until we can't
	// - Fill remaining with 2MB pages until we can't
	// - Fill remaning with 2KB pages
	// At each step, we of course need to check whether we finished paging what was asked

	union PageDirectoryPointerTableEntry* cur_directoryPointer;
	union PageDirectoryEntry* cur_directory;
	struct PageTableEntry* cur_pageTable;

	for (uint64_t i=0 ; i<n_pages ; i++){
		int pml4_index = getIndexPML4(virt_cur);
		int pdp_index = getIndexPageDirectoryPointerTable(virt_cur);
		int pd_index = getIndexPageDirectory(virt_cur);
		int pt_index = getIndexPageTable(virt_cur);

		// PML4
		if (g_pml4[pml4_index].present == 0){
			tmp = allocatePageOrPanic();
			setPML4Entry(g_pml4+pml4_index, tmp);
		}
		cur_directoryPointer = (void*) VMM_physicalToVirtual(g_pml4[pml4_index].address * PAGE_SIZE);

		// Page directory pointer
		if (cur_directoryPointer[pdp_index].pageDirectory.present == 0){
			tmp = allocatePageOrPanic();
			setPageDirectoryPointerTableEntry(cur_directoryPointer+pdp_index, tmp);
		}
		tmp = cur_directoryPointer[pdp_index].pageDirectory.address * PAGE_SIZE;
		cur_directory = (void*) VMM_physicalToVirtual(tmp);

		// Page directory
		if (cur_directory[pd_index].pageTable.present == 0){
			tmp = allocatePageOrPanic();
			setPageDirectoryEntry(cur_directory+pd_index, tmp);
		}
		tmp = cur_directory[pd_index].pageTable.address * PAGE_SIZE;
		cur_pageTable = (void*) VMM_physicalToVirtual(tmp);

		// Page table
		setPageTableEntry(cur_pageTable+pt_index, phys_cur, flags);

		phys_cur += PAGE_SIZE;
		virt_cur += PAGE_SIZE;
	}
}

// Paging.asm
bool setPML4(physical_address_t pml4);

void Paging_initialize(){
	// Clear the PML4 (sets all entries to invalid)
	memset(g_pml4, 0, PAGE_SIZE);

	// Kernel is loaded at its specific address
	physical_address_t kernel_phys = g_memoryMap.kernelAddress;
	extern uint8_t LOAD_ADDRESS, __text_start, __rodata_start, __data_start;
	virtual_address_t kernel_virt = (virtual_address_t) &LOAD_ADDRESS;

	for(int i=0 ; i<g_memoryMap.size ; i++){
		struct MemoryMapEntry* cur = g_memoryMap.entries + i;
		switch (cur->type){
		case MEMORY_USABLE:
		case MEMORY_ACPI_NVS:
		case MEMORY_ACPI_RECLAIMABLE:
		case MEMORY_BOOTLOADER_RECLAIMABLE:
			map(cur->address, VMM_physicalToVirtual(cur->address), cur->length/PAGE_SIZE,
				PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
			break;
		case MEMORY_RESERVED:
			break;
		case MEMORY_KERNEL:
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
			map(ktext_phys, (virtual_address_t) &__text_start, text_size, PAGE_READ|PAGE_EXEC|PAGE_KERNEL);
			// .rodata section: r--
			map(rodata_phys, (virtual_address_t) &__rodata_start, rodata_size, PAGE_READ|PAGE_KERNEL);
			// .data and .bss sections: rw-
			map(data_phys, (virtual_address_t) &__data_start, data_size, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
			break;
		case MEMORY_FRAMEBUFFER:
			map(cur->address, VMM_physicalToVirtual(cur->address), cur->length/PAGE_SIZE,
				PAGE_READ|PAGE_WRITE|PAGE_KERNEL|PAGE_CACHE_DISABLED);
			break;
		default:
			break;
		}
	}

	// Now load our page table
	// Note: g_pml4 is not in the HHDM region, so we cannot use VMM_virtualToPhysical
	// It is in the kernel data section, so we use the kernel code offset
	physical_address_t pml4_phys = kernel_phys + ((uint64_t)g_pml4 - kernel_virt);
	bool res = setPML4(pml4_phys);
	if (!res){
		log(PANIC, MODULE, "Could not set page table !!");
		panic();
	}

	log(SUCCESS, MODULE,
		"Kernel page table set successfully ! Higher Half Direct Map starts at %p, kernel at %p",
		VMM_getKernelMemoryOffset(), kernel_virt);
}
