#ifndef __X86_IO_H__
#define __X86_IO_H__

#include <stdint.h>
#include <stdbool.h>

void __attribute__((cdecl)) x86_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) x86_inb(uint16_t port);

void __attribute__((cdecl)) x86_io_wait();

#endif
