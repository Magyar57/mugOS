#include <stddef.h>
#include "stdio.h"

#include "Arch.h"

// ================ x86 ================
#ifdef ARCH_x86

#include "Arch/x86/IO.h"
#include "Arch/x86/Interrupts.h"
#include "Arch/x86/GDT.h"
#include "Arch/x86/IDT.h"
#include "Arch/x86/ISR.h"

void HAL_Initialize(){
	x86_DisableInterrupts();

	x86_GDT_Initialize();
	x86_IDT_Initialize();
	x86_ISR_Initialize();

	// x86_DoStuff();
	// x86_EnableInterrupts();

	puts("[ OK ] x86 HAL initalized");
}

void outb(uint16_t port, uint8_t value){
	x86_outb(port, value);
}

uint8_t inb(uint16_t port){
	return x86_inb(port);
}

#endif // ARCH_x86

// ================ arm64 ================
#ifdef ARCH_arm64

// Example, not yet implemented
// void HAL_Initialize(){}

#endif // ARCH_arm64
