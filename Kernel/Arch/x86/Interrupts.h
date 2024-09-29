#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

void __attribute__((cdecl)) x86_disable_interrupts();
void __attribute__((cdecl)) x86_enable_interrupts();

#endif
