#include <stddef.h>
#include "io.h"
#include "Logging.h"
#include "Panic.h"
#include "ISR.h"
#include "Drivers/i8259.h"
#include "Drivers/PS2.h"

#include "IRQ.h"

#define MODULE "IRQ"

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

void IRQ_prehandler(struct ISR_Params* params){
	// If we have a handler to call, we call it, and 'alles gut'

	uint8_t irq = (uint8_t) params->vector;
	irq = irq - IRQ_MASTER_PIC_REMAP_OFFSET; // clamp irq to 0-15

	if (g_IRQHandlers[irq] != NULL){
		g_IRQHandlers[irq](params);
		i8259_sendEIO(irq);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = g_IRQTypes[irq];
	log(PANIC, MODULE, "Unhandled IRQ number %d - %s", irq, interrupt_type);
	panic();
}

void IRQ_timer(struct ISR_Params* params){
	// Nothing to do yet
}

void IRQ_keyboard(struct ISR_Params* params){
	// Tell the PS/2 driver to update
	PS2_notifyKeyboard();
}

void IRQ_mouse(struct ISR_Params* params){
	// Tell the PS/2 driver to update
	PS2_notifyMouse();
}

void IRQ_initialize(){
	// Remap the PIC
	i8259_remap(IRQ_MASTER_PIC_REMAP_OFFSET, IRQ_MASTER_PIC_REMAP_OFFSET+8);
	i8259_disableAllIRQ();

	// Register our IRQ Pre-handler
	for(int i=0 ; i<16 ; i++){
		ISR_registerHandler(IRQ_MASTER_PIC_REMAP_OFFSET+i, IRQ_prehandler);
	}

	// Register our IRQ handlers
	IRQ_registerHandler(IRQ_TIMER, IRQ_timer);
	IRQ_registerHandler(IRQ_KEYBOARD, IRQ_keyboard);
	IRQ_registerHandler(IRQ_PS2_MOUSE, IRQ_mouse);

	i8259_enableAllIRQ();
}
