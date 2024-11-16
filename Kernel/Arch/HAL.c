#include <stddef.h>
#include "stdio.h"

#include "HAL.h"

// ================ x86 ================
#ifdef ARCH_x86

#include "Arch/x86/Interrupts.h"
#include "Arch/x86/GDT.h"
#include "Arch/x86/IDT.h"
#include "Arch/x86/ISR.h"
#include "Arch/x86/IRQ.h"

void HAL_initialize(){
	x86_DisableInterrupts();

	x86_GDT_Initialize();
	x86_IDT_Initialize();
	x86_ISR_Initialize();
	x86_IRQ_Initialize();

	x86_EnableInterrupts();
	puts("[  OK  ] x86 HAL initalized");
}

#endif // ARCH_x86

// ================ arm64 ================
#ifdef ARCH_arm64

// Example, not yet implemented
// void HAL_Initialize(){}

#endif // ARCH_arm64
