#ifndef __ARCH_H__
#define __ARCH_H__

// Arch.h:
// Defines the Hardware Abstraction Layer (HAL),
// and implements it depending on the requested architecture ARCH_*

#include <stdint.h>

void HAL_Initialize();

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

#endif
