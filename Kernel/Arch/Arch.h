#ifndef __ARCH_H__
#define __ARCH_H__

// Arch.h:
// Defines the Hardware Abstraction Layer (HAL),
// and implements it depending on the requested architecture ARCH_*

#if !defined(ARCH_x86) && !defined(ARCH_arm64)
#error "Invalid architecture provided ! Please define a valid architecture maccro from the supported list."
#endif

#include <stdint.h>

void HAL_Initialize();

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

// Halt indefinitely the CPU
void halt();

#endif
