#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

// Disable interrupts (cli)
void __attribute__((cdecl)) x86_DisableInterrupts();

// Enable interrupts (sti)
void __attribute__((cdecl)) x86_EnableInterrupts();

#endif
