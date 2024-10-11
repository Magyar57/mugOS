#include <stdint.h>
#include "Interrupts.h"
#include "GDT.h"
#include "IDT.h"

#include "ISR.h"

// In ISR_defs.c
void x86_ISR_InitializeInterruptHandlers();

void x86_ISR_Initialize(){
	x86_ISR_InitializeInterruptHandlers();
	for(int i=0 ; i<256 ; i++){
		x86_IDT_EnableInterruptHandler(i);
	}
	x86_IDT_DisableInterruptHandler(0x80);
}
