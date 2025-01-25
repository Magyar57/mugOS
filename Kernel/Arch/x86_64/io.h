#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>
#include <stdbool.h>

void __attribute__((cdecl)) outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) inb(uint16_t port);

void __attribute__((cdecl)) io_wait();

#endif
