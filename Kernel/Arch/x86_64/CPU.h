#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <stdbool.h>
#include "Preprocessor.h"

// CPU.h:
// - Architecture related definitions
// - CPU structure

#define X86_64_RFLAGS_IF 1<<9

typedef struct s_CPU {
	char vendor[13];
	uint8_t model;
	uint16_t family;
	uint8_t type;
	uint8_t stepping;
	uint8_t brandIndex;
	uint8_t cflushLineSize;
	uint8_t maxAddressableCpuIds;
	uint32_t features0, features1;
} CPU;

/// @brief Halt (stops until next interrupt) the processor
#define halt() __asm__ volatile("hlt")

/// @brief Stops DEFINITELY the processor (interrupts are masked)
#define haltAndCatchFire() __asm__ volatile("cli; 1: hlt; jmp 1b")

/// @brief Disable interrupts (cli)
#define CPU_IRQDisable() __asm__ volatile("cli")

/// @brief Enable interrupts (sti)
#define CPU_IRQEnable() __asm__ volatile("sti; nop")

static always_inline unsigned long getFlags(){
	unsigned long flags;

	// "=rm": compiler can place flags_var in registers or memory
	__asm__ volatile("pushf ; pop %[flags_var]"
		: [flags_var] "=rm" (flags)
		:
		: "memory"
	);

	return flags;
}

/// @returns Whether the cpu supports the cpuid instruction
extern bool CPU_supportsCpuid();

/// @brief Fill the CPU struct with CPU identification informations
/// @returns true on success, false on error (cpuid instruction not supported)
bool CPU_identify(CPU* cpu);

#endif
