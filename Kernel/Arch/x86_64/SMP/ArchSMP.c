#include "string.h"
#include "stdlib.h"
#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "Registers.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "Drivers/ACPI/ACPI.h"
#include "Drivers/APIC.h"

#include "HAL/SMP/ArchSMP.h"
#define MODULE "Arch SMP"

int g_nCPUs;
struct CPUInfo* g_CPUInfos;

// EntryAP.asm
extern void entryAP();
extern void endEntryAP; // label in EntryAP.asm

static inline void setCPUInfo(struct CPUInfo* info){
	Registers_writeMSR(MSR_ADDR_IA32_GS_BASE, (uintptr_t) info);
}

static int parseNumberOfValidCPUs(){
	int n_cpus = 0;

	// Count number of valid (enabled or online-capable) CPUs
	for (int i=0 ; i<g_MADT.nLAPIC ; i++){
		if (!g_MADT.LAPICs[i].flags.bits.onlineCapable && !g_MADT.LAPICs[i].flags.bits.enabled)
			continue;

		n_cpus++;
	}

	return n_cpus;
}

void ArchSMP_init(){
	g_nCPUs = parseNumberOfValidCPUs();

	g_CPUInfos = kmalloc(g_nCPUs * sizeof(struct CPUInfo));
	if (g_CPUInfos == NULL){
		log(PANIC, MODULE, "Couldn't allocate enough memory for per-CPU informations structures !!");
		panic();
	}

	// Initialize each CPU's ID and local APIC ID
	int id = 0;
	for (int i=0 ; i<g_nCPUs ; i++){
		g_CPUInfos[id].ID = id;
		g_CPUInfos[id].apicID = g_MADT.LAPICs[i].lapicID;
		id++;
	}

	// Set the BootStrap Processor (BSP)'s per-CPU informations
	setCPUInfo(g_CPUInfos); // first of the array
}

void ArchSMP_startCPUs(){
	// One CPU system, no init needed (and might be a PIC system !)
	if (g_nCPUs == 1)
		return;

	// Prepare CPU's startup code
	int size = (void*) &endEntryAP - (void*) entryAP;
	int n_pages = roundToPage(size);
	physical_address_t ap_entry_phys = PMM_allocatePages(n_pages);
	if ((ap_entry_phys & ~0xff000) != 0){
		log(ERROR, MODULE, "Could not allocate low memory needed for starting CPUs. SMP disabled");
		return;
	}

	// Copy the entryAP code to low memory. This CPU is in long mode, so we need to map the region
	// We use a temporary userspace page: it is guaranteed to be free (userspace isn't up yet)
	virtual_address_t ap_entry_virt = 0x1000;
	VMM_map(ap_entry_phys, ap_entry_virt, n_pages, PAGE_KERNEL|PAGE_READ|PAGE_WRITE);
	memcpy((void*) ap_entry_virt, entryAP, size);

	for (int i=0 ; i<g_nCPUs ; i++){
		if (g_CPUInfos[i].ID == ArchSMP_getCpuId())
			continue;

		// Wake each CPU, which will start executing the entryAP
		APIC_wakeCPU(g_CPUInfos[i].apicID, ap_entry_phys);
	}

	VMM_unmap(ap_entry_virt, n_pages);
	PMM_freePages(ap_entry_phys, n_pages);

	log(SUCCESS, MODULE, "Successfully started %d threads", g_nCPUs);
}
