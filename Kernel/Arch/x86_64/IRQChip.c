#include <stddef.h>
#include "io.h"
#include "Logging.h"
#include "Panic.h"
#include "ISR.h"
#include "Drivers/i8259.h"

#include "HAL/IRQChip.h"

#define MODULE "x86_64 IRQ Chip"

// IRQ 0-7 are mapped to interrupts 32 to 39 ; IRQ 8-15 to interrupts 40 to 47
#define IRQ_MASTER_PIC_REMAP_OFFSET 0x20

// Global array of [un]registered IRQ handlers
IRQHandler g_IRQHandlers[16];

static const char* const g_IRQTypes[] = {
    "Programmable Interrupt Timer (PIT)",
    "PS/2 Keyboard",
    "Cascade",								// Used internally by the two PICs, never raised
    "COM2",									// If enabled
    "COM1",									// If enabled
    "LPT2",									// If enabled
    "Floppy Disk",
    "LPT1",
    "CMOS real-time clock",					// If enabled
    "Free for peripherals/legacy SCSI/NIC",
    "Free for peripherals/legacy SCSI/NIC",
    "Free for peripherals/legacy SCSI/NIC",
    "PS/2 Mouse",
    "FPU/Coprocessor/Inter-processor",
    "Primary ATA Hard Disk",
    "Secondary ATA Hard Disk",
};

void IRQChip_registerHandler(uint8_t irq, IRQHandler handler){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = handler;
}

void IRQChip_deregisterHandler(uint8_t irq){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = NULL;
}

// ================ IRQ Handlers ================

static void prehandler(struct ISR_Params* params){
	uint8_t irq = (uint8_t) params->vector;
	irq = irq - IRQ_MASTER_PIC_REMAP_OFFSET; // clamp irq to 0-15

	// IRQ 7 and 15 may be spurious, in which case we must NOT send an EOI
	if (irq==7 || irq==15){
		uint16_t isr = i8259_getCombinedISR();
		// Spurious IRQ 7
		if ((isr & (1<<7)) == 0){
			log(WARNING, MODULE, "Got spurious IRQ %d, ignored (no EOI sent)", irq);
			return;
		}
		// Spurious IRQ 15
		// We don't still need to ACK the interrupt to the slave PIC,
		//  but we do for the master PIC though
		if ((isr & (1<<15)) == 0){
			log(WARNING, MODULE, "Got spurious IRQ %d, ignored (no EOI sent)", irq);
			i8259_sendEIO(7); // Master PIC ACK
			return;
		}
	}

	// If we have a handler to call, we call it, and 'alles gut'
	if (g_IRQHandlers[irq] != NULL) g_IRQHandlers[irq](params);
	// Otherwise, we send a warning
	else log(WARNING, MODULE, "Unhandled IRQ number %d - %s", irq, g_IRQTypes[irq]);

	// Finally, signal to the PIC that we handled the interrupt
	i8259_sendEIO(irq);
}

// Simple blinking timer on the bottom right of the screen
#include "Drivers/Graphics/Framebuffer.h"
void timer(struct ISR_Params* params){
	extern Framebuffer m_framebuffer;
	static bool clock = false;
	const int rect_size = 4;
	color_t color = (clock) ? GREEN : LIGHT_GREY;
	Framebuffer_drawRectangle(&m_framebuffer, m_framebuffer.width-rect_size-1, m_framebuffer.height-rect_size-1, rect_size, rect_size, color);
	clock = !clock;
}

void IRQChip_initialize(){
	// Remap the PIC
	i8259_remap(IRQ_MASTER_PIC_REMAP_OFFSET, IRQ_MASTER_PIC_REMAP_OFFSET+8);
	i8259_disableAllIRQ();

	// Register our IRQ Pre-handler
	for(int i=0 ; i<16 ; i++){
		ISR_registerHandler(IRQ_MASTER_PIC_REMAP_OFFSET+i, prehandler);
	}

	// Temporary, until we have a Timer subsystem or whatever
	IRQ_registerHandler(IRQ_TIMER, (void (*)(void*)) timer);

	i8259_enableAllIRQ();
}
