#ifndef __ARCH_FLOW_H__
#define __ARCH_FLOW_H__

/// @brief Halt (stops until next interrupt) the processor
#define halt() __asm__ volatile("hlt")

/// @brief Stops DEFINITELY the processor (interrupts are masked)
#define haltAndCatchFire() __asm__ volatile("cli; 1: hlt; jmp 1b")

#endif
