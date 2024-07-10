#ifndef __ARCH_H__
#define __ARCH_H__

// Arch.h: header file that includes the right architecture header files

// x86 (32 bits)
#ifdef ARCH_x86
#include "Arch/x86/IO.h"
#endif

#endif
