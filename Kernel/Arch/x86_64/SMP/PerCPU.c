#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "Panic.h"
#include "Logging.h"
#include "Registers.h"

#include "HAL/SMP/PerCPU.h"
#define MODULE "Per-CPU data"

// Initial BSP info, used during early boot
static struct CPUInfo m_bspInfo = {
	.ID = 0,
	.apicID = -1
};

// Final, malloc-ed CPU infos array
static struct CPUInfo* m_CPUInfos;

static void setInfo(struct CPUInfo* info){
	Registers_writeMSR(MSR_ADDR_IA32_GS_BASE, (uintptr_t) info);
}

void PerCPU_wake(){
	setInfo(&m_bspInfo);
}

void PerCPU_init(int nCpus){
	assert(nCpus > 0);

	m_CPUInfos = kmalloc(nCpus * sizeof(struct CPUInfo));
	if (m_CPUInfos == NULL){
		log(PANIC, MODULE, "Couldn't allocate memory for per-CPU informations !");
		panic();
	}

	memcpy(m_CPUInfos, &m_bspInfo, sizeof(struct CPUInfo));
	setInfo(m_CPUInfos);
}
