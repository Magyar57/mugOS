#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct z_CPU {
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

// Halt (stops until next interrupt) the processor
void __attribute__((cdecl)) halt();

// Stops DEFINITELY the processor (interrupts are masked)
void __attribute__((cdecl)) terminate();

// Disable interrupts (cli)
void __attribute__((cdecl)) disableInterrupts();

// Enable interrupts (sti)
void __attribute__((cdecl)) enableInterrupts();

// Call a given interrupt (`int` instruction)
void __attribute__((cdecl)) callInterrupt(uint8_t vector);

/// @returns Whether the cpu supports the cpuid instruction
extern bool CPU_supportsCpuid();

/// @brief Fill the CPU struct with CPU identification informations
/// @returns true on success, false on error (cpuid instruction not supported)
bool CPU_identify(CPU* cpu);

#endif
