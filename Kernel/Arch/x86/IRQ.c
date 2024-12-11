#include <stddef.h>
#include "stdio.h"
#include "Panic.h"
#include "io.h"
#include "ISR.h"
#include "PIC.h"
#include "Drivers/PS2.h"

#include "IRQ.h"

// IRQ 0-7 are mapped to interrupts 32 to 39 ; IRQ 8-15 to interrupts 40 to 47
#define IRQ_MASTER_PIC_REMAP_OFFSET 0x20

// Global array of [un]registered IRQ handlers
IRQHandler g_IRQHandlers[16];

#define IRQ_TIMER		0x00
#define IRQ_KEYBOARD	0x01
#define IRQ_PS2_MOUSE	0x0c

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

void IRQ_registerHandler(uint8_t irq, IRQHandler handler){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = handler;
}

void IRQ_deregisterHandler(uint8_t irq){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = NULL;
}

// ================ IRQ Handlers ================

void IRQ_prehandler(ISR_Params* params){
	// If we have a handler to call, we call it, and 'alles gut'

	uint8_t irq = (uint8_t) params->vector;
	irq = irq - IRQ_MASTER_PIC_REMAP_OFFSET; // clamp irq to 0-15

	if (g_IRQHandlers[irq] != NULL){
		g_IRQHandlers[irq](params);
		PIC_sendEIO(irq);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = g_IRQTypes[irq];
	printf("Unhandled IRQ number %d - %s\n", irq, interrupt_type);
	printf("\tvector=%p eflags=%p err=%p\n", params->vector, params->eflags, params->err);
	printf("\teax=%p ebx=%p ecx=%p edx=%p esi=%p edi=%p\n",
		params->eax, params->ebx, params->ecx, params->edx, params->esi, params->edi
	);
	printf("\teip=%p esp=%p ebp=%p\n", params->eip, params->esp, params->ebp);
	printf("\tcs=%p ds=%p ss=%p\n", params->cs, params->ds, params->ss);
	PANIC();
}

void IRQ_timer(ISR_Params* params){
	// Nothing to do yet
}

void IRQ_keyboard(ISR_Params* params){
	// Tell the PS/2 driver to update
	PS2_notifyKeyboard();
}

void IRQ_mouse(ISR_Params* params){
	// Tell the PS/2 driver to update
	PS2_notifyMouse();
}

void IRQ_initialize(){
	// Remap the PIC
	PIC_remap(IRQ_MASTER_PIC_REMAP_OFFSET, IRQ_MASTER_PIC_REMAP_OFFSET+8);

	// Register our IRQ Pre-handler
	for(int i=0 ; i<16 ; i++){
		ISR_registerHandler(IRQ_MASTER_PIC_REMAP_OFFSET+i, IRQ_prehandler);
	}

	// Register our IRQ handlers
	IRQ_registerHandler(IRQ_TIMER, IRQ_timer);
	IRQ_registerHandler(IRQ_KEYBOARD, IRQ_keyboard);
	IRQ_registerHandler(IRQ_PS2_MOUSE, IRQ_mouse);

	PIC_enableAllIRQ();
}
