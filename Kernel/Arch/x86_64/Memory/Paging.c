#include "string.h"
#include "Preprocessor.h"
#include "Panic.h"
#include "Logging.h"
#include "Memory/PMM.h"
#include "GDT.h"

#include "HAL/Paging.h"
#define MODULE "Paging"

// Number of entries in the tables
#define TABLE_SIZE 512

#define ADDRESS_SIZE 48 // aka MAXPHYADDR aka M

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

#define getEntryAddress(addr)					( ( (physical_address_t)addr & 0x0000fffffffff000) >> 12 )
#define getIndexPML4(addr) 						( ( (physical_address_t)addr & 0x0000ffc000000000) >> 39 )
#define getIndexPageDirectoryPointerTable(addr)	( ( (physical_address_t)addr & 0x0000007fc0000000) >> 30 )
#define getIndexPageDirectory(addr)				( ( (physical_address_t)addr & 0x000000003fe00000) >> 21 )
#define getIndexPageTable(addr)					( ( (physical_address_t)addr & 0x00000000001ff000) >> 12 )
#define get4KBPageAddress(addr)					( ( (physical_address_t)addr & 0x0000fffffffff000) >> 12 )
#define get2MBPageAddress(addr)					( ( (physical_address_t)addr & 0x0000ffffffe00000) >> 21 )
#define get1GBPageAddress(addr)					( ( (physical_address_t)addr & 0x0000ffffc0000000) >> 30 )

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
	physical_address_t address : ADDRESS_SIZE-30; // @phys of the 2MB page (ADDRESS_SIZE-30 bits)
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

#pragma endregion

aligned(PAGE_SIZE) struct PageMapLevel4Entry g_pml4[TABLE_SIZE];
aligned(PAGE_SIZE) union PageDirectoryPointerTableEntry g_pageDirectoryPointer[TABLE_SIZE];
aligned(PAGE_SIZE) union PageDirectoryEntry g_pageDirectory[TABLE_SIZE];
aligned(PAGE_SIZE) struct PageTableEntry g_pageTable[TABLE_SIZE];

compile_assert(sizeof(struct PageMapLevel4Entry) == 8);
compile_assert(sizeof(union PageDirectoryPointerTableEntry) == 8);
compile_assert(sizeof(union PageDirectoryEntry) == 8);
compile_assert(sizeof(struct PageTableEntry) == 8);
compile_assert(sizeof(g_pml4) == 4096);
compile_assert(sizeof(g_pageDirectoryPointer) == 4096);
compile_assert(sizeof(g_pageDirectory) == 4096);
compile_assert(sizeof(g_pageTable) == 4096);

// Paging.asm
bool setPML4(physical_address_t pml4);

static void setPML4Entry(struct PageMapLevel4Entry* entry, physical_address_t address){
	entry->present = true;
	entry->readWrite = true;
	entry->privilege = PRIVILEGE_KERNEL;
	entry->writeThrough = 0;
	entry->cacheDisabled = false;
	entry->accessed = 0;
	entry->restart = 0;
	entry->address = getEntryAddress(address);
	entry->executeDisabled = 0;
}

static void setPageDirectoryPointerTableEntry(union PageDirectoryPointerTableEntry* entry, physical_address_t address){
	entry->pageDirectory.present = true;
	entry->pageDirectory.readWrite = true;
	entry->pageDirectory.privilege = PRIVILEGE_KERNEL;
	entry->pageDirectory.writeThrough = false;
	entry->pageDirectory.cacheDisabled = true;
	entry->pageDirectory.accessed = false;
	entry->pageDirectory.pageSize = 0; // points to a page directory
	entry->pageDirectory.restart = false;
	entry->pageDirectory.address = getEntryAddress(address);
	entry->pageDirectory.executeDisabled = false;
}

static void setPageDirectoryEntry(union PageDirectoryEntry* entry, physical_address_t address){
	entry->pageTable.present = true;
	entry->pageTable.readWrite = true;
	entry->pageTable.privilege = PRIVILEGE_KERNEL;
	entry->pageTable.writeThrough = false;
	entry->pageTable.cacheDisabled = false;
	entry->pageTable.accessed = false;
	entry->pageTable.pageSize = 0;
	entry->pageTable.restart = false;
	entry->pageTable.address = getEntryAddress(address);
	entry->pageTable.executeDisabled = false;
}

static void setPageTableEntry(struct PageTableEntry* entry, physical_address_t address){
	entry->present = true;
	entry->readWrite = true;
	entry->privilege = PRIVILEGE_KERNEL;
	entry->writeThrough = false;
	entry->cacheDisabled = false;
	entry->accessed = 0;
	entry->dirty = 0;
	entry->pat = 0; // TODO check what PAT is ???
	entry->global = false;
	entry->restart = false;
	entry->address = getEntryAddress(address);
	entry->protectionKey = 0b0000;
	entry->executeDisabled = false;
}

void Paging_initialize(physical_address_t kernelPhys, virtual_address_t kernelVirt, size_t kSize, size_t virtualOffset){
	// Clear the tables (that sets all entries to invalid)
	memset(g_pml4, 0, sizeof(g_pml4));
	memset(g_pageDirectoryPointer, 0, sizeof(g_pageDirectoryPointer));
	memset(g_pageDirectory, 0, sizeof(g_pageDirectory));
	memset(g_pageTable, 0, sizeof(g_pageTable));

	// Map the kernel code. These are the STARTING indexes, we'll have to fill several entries
	int kernel_pml4_index = getIndexPML4(kernelVirt);
	int kernel_pdp_index = getIndexPageDirectoryPointerTable(kernelVirt);
	int kernel_pd_index = getIndexPageDirectory(kernelVirt);
	int kernel_pt_index = getIndexPageTable(kernelVirt);

	physical_address_t pml4_addr = (physical_address_t) &g_pml4 - virtualOffset;
	physical_address_t pdp_addr = (physical_address_t) &g_pageDirectoryPointer - virtualOffset;
	physical_address_t pd_addr = (physical_address_t) &g_pageDirectory - virtualOffset;
	physical_address_t pt_addr = (physical_address_t) &g_pageTable - virtualOffset;

	setPML4Entry(g_pml4 + kernel_pml4_index, pdp_addr);
	setPageDirectoryPointerTableEntry(g_pageDirectoryPointer + kernel_pdp_index, pd_addr);
	setPageDirectoryEntry(g_pageDirectory + kernel_pd_index, pt_addr);

	// Page table
	size_t mapped = 0; // bytes mapped
	size_t offset = 0; // offset from kernel
	for(int i=kernel_pt_index ; i<TABLE_SIZE ; i++){
		setPageTableEntry(g_pageTable + kernel_pt_index, kernelPhys+offset);
		mapped += PAGE_SIZE;
		offset += PAGE_SIZE;
	}

	// We mapped the kernel (text)
	debug("kernel @phys=%p @virt=%p", kernelPhys, kernelVirt);
	// But now we need to map the rest of the data (stacks, limine stuff, framebuffer...)

	if (mapped < kSize){
		debug("Couldn't map the whole kernel, need another page directory");
		panic();
	}

	bool res = setPML4(pml4_addr);
	if (!res){
		log(PANIC, MODULE, "Could not set page table !!");
		panic();
	}

	debug("Page table set successfully");
}
