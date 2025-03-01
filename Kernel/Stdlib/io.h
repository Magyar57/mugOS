#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>
#include <stdbool.h>

// Note: the functions are meant to be implemented by the specific architecture

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void io_wait();

#endif
