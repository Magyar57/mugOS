#include <stddef.h>
#include "stdio.h"

#include "HAL.h"

// ================ x86 ================
#ifdef ARCH_x86

#include "Arch/x86/CPU.h"
#include "Arch/x86/GDT.h"
#include "Arch/x86/IDT.h"
#include "Arch/x86/ISR.h"
#include "Arch/x86/IRQ.h"

void HAL_initialize(){
	disableInterrupts();

	GDT_initialize();
	IDT_initialize();
	ISR_initialize();
	IRQ_initialize();

	enableInterrupts();
	puts("[  OK  ] x86 HAL initalized");
}

#endif // ARCH_x86

// ================ arm64 ================
#ifdef ARCH_arm64

// Example, not yet implemented
// void HAL_Initialize(){}

#endif // ARCH_arm64
