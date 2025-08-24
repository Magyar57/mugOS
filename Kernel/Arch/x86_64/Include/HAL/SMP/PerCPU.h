#ifndef __PER_CPU_H__
#define __PER_CPU_H__

#include <stdint.h>

// SMP-related, architecture-agnostic CPU informations
struct CPUInfo {
	uint32_t ID; // the actual ID we use, from 0 to #CPUS-1
	uint32_t apicID;
};

// Initial BSP info, used during early boot
extern struct CPUInfo g_bspInfo;

// Final CPU infos, final malloc-ed array
extern struct CPUInfo* g_CPUInfos;

/// @brief Get the value of the `member` (of type `type`) from the per-CPU struct CPUInfo instance
#define PerCPU_getCPUInfoMember(member) ({ \
	__typeof__(((struct CPUInfo *)0)->member) __val; \
	/* '%c1' ensures the offset is treated as a compile-time constant */ \
	__asm__ volatile ("mov %%gs:%c1, %0" \
		: "=r" (__val) \
		: "i" (offsetof(struct CPUInfo, member)) \
		: "memory"); /* Clobber memory to prevent reordering */ \
	__val; \
})

#define PerCPU_getCpuId() PerCPU_getCPUInfoMember(ID)

/// @brief Sets up the BSP's per-CPU info for early boot
void PerCPU_wake();

void PerCPU_setInfo(struct CPUInfo* info);

#endif
