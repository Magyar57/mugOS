#ifndef __HALT_H__
#define __HALT_H__

/// @brief No operation instruction
#define nop() __asm__ volatile("nop")

/// @brief Pause instruction. Improves the performance of spin-wait loops
#define pause() __asm__ volatile("pause")

/// @brief Halt (stops until next interrupt) the processor
#define halt() __asm__ volatile("hlt")

/// @brief Stops DEFINITELY the processor (interrupts are masked)
#define haltAndCatchFire() __asm__ volatile("cli; 1: hlt; jmp 1b")

#endif
