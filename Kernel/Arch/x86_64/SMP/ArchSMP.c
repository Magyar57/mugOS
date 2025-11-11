#include "string.h"
#include "stdlib.h"
#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "Drivers/ACPI/ACPI.h"
#include "HAL/SMP/PerCPU.h"
#include "Drivers/IrqChip/APIC.h"

#include "HAL/SMP/ArchSMP.h"
#define MODULE "Arch SMP"

int g_nCPUs;

// EntryAP.asm
extern void entryAP();
extern uint8_t endEntryAP; // label in EntryAP.asm

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
	PerCPU_init(g_nCPUs);
}

void ArchSMP_startCPUs(){
	// One CPU system, no init needed (and might be a PIC system !)
	if (g_nCPUs == 1)
		return;

	// Prepare CPU's startup code
	int size = (void*) &endEntryAP - (void*) entryAP;
	int n_pages = roundToPage(size);
	paddr_t ap_entry_phys = PMM_allocatePages(n_pages);
	if ((ap_entry_phys & ~0xff000) != 0){
		log(ERROR, MODULE, "Could not allocate low memory needed for starting CPUs. SMP disabled");
		return;
	}

	// Copy the entryAP code to low memory. This CPU is in long mode, so we need to map the region
	// We use a temporary userspace page: it is guaranteed to be free (userspace isn't up yet)
	vaddr_t ap_entry_virt = 0x1000;
	VMM_map(ap_entry_phys, ap_entry_virt, n_pages, PAGE_KERNEL|PAGE_READ|PAGE_WRITE);
	memcpy((void*) ap_entry_virt, entryAP, size);

	uint32_t this_lapic = PerCPU_getCPUInfoMember(apicID);
	struct MADTEntry_LAPIC* cur;
	for (int i=0 ; i<g_MADT.nLAPIC ; i++){
		cur = g_MADT.LAPICs + i;
		if (!cur->flags.bits.onlineCapable && !cur->flags.bits.enabled)
			continue;
		if (cur->lapicID == this_lapic)
			continue;

		// Wake each CPU, which will start executing the entryAP
		APIC_wakeCPU(cur->lapicID, ap_entry_phys);
	}

	VMM_unmap(ap_entry_virt, n_pages);
	PMM_freePages(ap_entry_phys, n_pages);

	log(SUCCESS, MODULE, "Successfully started %d threads", g_nCPUs);
}
