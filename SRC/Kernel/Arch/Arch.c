#include "stdio.h"

#include "Arch.h"

// ================ x86 ================
#ifdef ARCH_x86

#include "Arch/x86/IO.h"
#include "Arch/x86/Interrupts.h"
#include "Arch/x86/GDT.h"

void HAL_Initialize(){
	GDT_Initialize();
	puts("[ OK ] HAL initalized");
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
void HAL_Initialize(){}

#endif // ARCH_arm64
