#ifndef __ARCH_SMP_H__
#define __ARCH_SMP_H__

#include <stdint.h>

// SMP-related, architecture-agnostic CPU informations
struct CPUInfo {
	uint32_t ID; // the actual ID we use, from 0 to #CPUS-1
	uint32_t apicID;
};

extern int g_nCPUs;
extern struct CPUInfo* g_CPUInfos;

#define ArchSMP_getCpuId() 0

void ArchSMP_init();

#endif
