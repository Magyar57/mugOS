#include "stdlib.h"
#include "assert.h"
#include "Panic.h"
#include "Logging.h"
#include "Registers.h"

#include "HAL/SMP/PerCPU.h"
#define MODULE "Per-CPU data"

struct CPUInfo g_bspInfo = {
	.ID = 2,
	.apicID = -1
};

struct CPUInfo* g_CPUInfos;

void PerCPU_wake(){
	PerCPU_setInfo(&g_bspInfo);
}

void PerCPU_setInfo(struct CPUInfo* info){
	Registers_writeMSR(MSR_ADDR_IA32_GS_BASE, (uintptr_t) info);
}
