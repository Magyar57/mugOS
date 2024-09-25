#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stdint.h>

// Returns the value of the IP register
uint32_t __attribute__((cdecl)) x86_get_ip();

#endif
