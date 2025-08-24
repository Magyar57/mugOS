#ifndef __PER_CPU_H__
#define __PER_CPU_H__

#include <stdint.h>

struct CPUInfo {
	uint32_t ID; // the actual ID we use, from 0 to #CPUS-1
	uint32_t apicID;
};

/// @brief Get the value of the `member` (of type `type`) from the per-CPU struct CPUInfo instance
#define PerCPU_getCPUInfoMember(member) ({ \
	__typeof__(((struct CPUInfo*) 0)->member) __val; \
	/* '%c1' ensures the offset is treated as a compile-time constant */ \
	__asm__ volatile ("mov %%gs:%c1, %0" \
		: "=r" (__val) \
		: "i" (offsetof(struct CPUInfo, member)) \
		: "memory"); /* Clobber memory to prevent reordering */ \
	__val; \
})

/// @brief Set the value of the `member` in the per-CPU struct CPUInfo instance
#define PerCPU_setCPUInfoMember(member, value) \
do { \
	__typeof__(((struct CPUInfo*) 0)->member) __val = (value); \
	__asm__ volatile ("mov %0, %%gs:%c1" \
		: /* No outputs */ \
		: "r" (__val), "i" (offsetof(struct CPUInfo, member)) \
		: "memory"); \
} while (0)

#define PerCPU_getCpuId() PerCPU_getCPUInfoMember(ID)

/// @brief Sets up the BSP's per-CPU info for early boot
void PerCPU_wake();

/// @brief Initialize the per-CPU datas for SMP, replacing the early boot state
/// @param nCpus Number of CPUs on the system
void PerCPU_init(int nCpus);

#endif
