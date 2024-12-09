#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>

// Halt (stops indefinitely) the processor
void __attribute((cdecl)) halt();

// Disable interrupts (cli)
void __attribute__((cdecl)) disableInterrupts();

// Enable interrupts (sti)
void __attribute__((cdecl)) enableInterrupts();

// Call a given interrupt (`int` instruction)
void __attribute__((cdecl)) callInterrupt(uint8_t vector);

#endif
