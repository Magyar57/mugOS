#include <stddef.h>
#include "stdio.h"
#include "Panic.h"
#include "ISR.h"
#include "PIC.h"

#include "IRQ.h"

// IRQ 0-7 are mapped to interrupts 32 to 39 ; IRQ 8-15 to interrupts 40 to 47
#define IRQ_MASTER_PIC_REMAP_OFFSET 0x20

// Global array of [un]registered IRQ handlers
IRQHandler g_IRQHandlers[16];

static const char* const g_IRQTypes[] = {
    "Programmable Interrupt Timer Interrupt",
    "Keyboard Interrupt",
    "Cascade",											// Used internally by the two PICs, never raised
    "COM2",												// If enabled
    "COM1",												// If enabled
    "LPT2",												// If enabled
    "Floppy Disk",
    "LPT1 / Unreliable 'spurious' interrupt",			// Usually (?)
    "CMOS real-time clock",								// If enabled
    "Free for peripherals/legacy SCSI/NIC",
    "Free for peripherals/legacy SCSI/NIC",
    "Free for peripherals/legacy SCSI/NIC",
    "PS2 Mouse",
    "FPU/Coprocessor/Inter-processor",
    "Primary ATA Hard Disk",
    "Secondary ATA Hard Disk ",
};

void x86_IRQ_RegisterIRQHandler(uint8_t irq, IRQHandler handler){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = handler;
}

void x86_IRQ_DeregisterIRQHandler(uint8_t irq){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = NULL;
}

// All IRQ will lead to this common x86_IRQ_Prehandler
void x86_IRQ_Prehandler(ISR_Params* params){
	// If we have a handler to call, we call it, and 'alles gut'
	
	uint8_t irq = (uint8_t) params->vector;
	irq = irq - IRQ_MASTER_PIC_REMAP_OFFSET; // clamp irq to 0-15

	if (g_IRQHandlers[irq] != NULL){
		g_IRQHandlers[irq](params);
		x86_PIC_SendEIO(irq);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = (params->vector < 32) ? g_IRQTypes[params->vector] : "Interrupt";
	printf("Unhandled IRQ n°%d %s !\n", irq, interrupt_type);
	printf("\tvector=%p eflags=%p err=%p\n", params->vector, params->eflags, params->err);
	printf("\teax=%p ebx=%p ecx=%p edx=%p esi=%p edi=%p\n",
		params->eax, params->ebx, params->ecx, params->edx, params->esi, params->edi
	);
	printf("\teip=%p esp=%p ebp=%p\n", params->eip, params->esp, params->ebp);
	printf("\tcs=%p ds=%p ss=%p\n", params->cs, params->ds, params->ss);
	PANIC();
}

void x86_IRQ_Initialize(){
	x86_PIC_Remap(IRQ_MASTER_PIC_REMAP_OFFSET, IRQ_MASTER_PIC_REMAP_OFFSET+8);
	for(int i=0 ; i<16 ; i++){
		x86_ISR_RegisterHandler(IRQ_MASTER_PIC_REMAP_OFFSET+i, x86_IRQ_Prehandler);
		x86_PIC_EnableIRQ(i);
	}
}
