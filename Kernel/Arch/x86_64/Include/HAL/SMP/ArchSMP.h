#ifndef __ARCH_SMP_H__
#define __ARCH_SMP_H__

#include <stddef.h>
#include <stdint.h>

// SMP-related, architecture-agnostic CPU informations
struct CPUInfo {
	uint32_t ID; // the actual ID we use, from 0 to #CPUS-1
	uint32_t apicID;
};

extern int g_nCPUs;
extern struct CPUInfo* g_CPUInfos;

/// @brief Get the value of the `member` (of type `type`) from the per-CPU struct CPUInfo instance
#define ArchSMP_getCPUInfoMember(member) ({ \
	__typeof__(((struct CPUInfo *)0)->member) __val; \
	/* '%c1' ensures the offset is treated as a compile-time constant */ \
	__asm__ volatile ("mov %%gs:%c1, %0" \
		: "=r" (__val) \
		: "i" (offsetof(struct CPUInfo, member)) \
		: "memory"); /* Clobber memory to prevent reordering */ \
	__val; \
})

#define ArchSMP_getCpuId() ArchSMP_getCPUInfoMember(ID)

void ArchSMP_init();

void ArchSMP_startCPUs();

#endif
