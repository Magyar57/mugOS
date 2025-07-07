#include "stdlib.h"
#include "Logging.h"
#include "Drivers/ACPI/ACPI.h"

#include "HAL/SMP/ArchSMP.h"
#define MODULE "Arch SMP"

int g_nCPUs;
struct CPUInfo* g_CPUInfos;

void ArchSMP_init(){
	g_nCPUs = g_MADT.nLAPIC;
	g_CPUInfos = kmalloc(g_nCPUs * sizeof(struct CPUInfo));

	for (int i=0 ; i<g_nCPUs ; i++){
		g_CPUInfos->ID = i;
		g_CPUInfos->apicID = g_MADT.LAPICs[i].lapicID;
	}

	// Now start the CPUs
	// (unimplemented)
	// => send INIT IPI (with APIC) to wake up CPUs
	// => in each CPU's init function, we should set the GS register
	//    to point to its own per-CPU 'CPUInfo' structure
	// => then correct the ArchSMP_getCpuId macro to read said per-CPU structure
}
