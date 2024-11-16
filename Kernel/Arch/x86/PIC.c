#include <stdbool.h>
#include "stdio.h"
#include "Arch/io.h"
#include "Panic.h"

#include "PIC.h"

// 8259 is the intel PIC
// https://wiki.osdev.org/8259_PIC

#define PIC_MASTER_CMD		0x0020	// Master's PIC Command port
#define PIC_MASTER_DATA		0x0021	// Master's PIC Data port
#define PIC_SLAVE_CMD		0x00a0	// Slave's PIC Command port
#define PIC_SLAVE_DATA		0x00a1	// Slave's PIC Data port
// PIC ICW1 (Initialization Control Word 1) Commands
#define PIC_ICW1_ICW4		0x01	// Indicates that ICW4 will be present
#define PIC_ICW1_SINGLE		0x02	// Single (cascade) mode
#define PIC_ICW1_INTERVAL4	0x04	// Call address interval 4 (8)
#define PIC_ICW1_LEVEL		0x08	// Level triggered (edge) mode
#define PIC_ICW1_INIT		0x10	// Initialization - required!
// PIC ICW4 (Initialization Control Word 4) Commands
#define PIC_ICW4_8086		0x01	// 8086/88 (MCS-80/85) mode
#define PIC_ICW4_AUTO_EOI	0x02	// Auto (normal) EOI
#define PIC_ICW4_BUF_MASTER	0x04	//
// #define PIC_ICW4_BUF_SLAVE	0x08	// Buffered mode/slave
// #define PIC_ICW4_BUF_MASTER	0x0c	// Buffered mode/master
#define PIC_ICW4_SFNM		0x10	// Special fully nested (not)
// PIC Commands
#define PIC_CMD_EOI			0x20	// EOI End Of Interrupt
#define PIC_CMD_READ_IRR	0x0a	// Read IRR In-Service Register
#define PIC_CMD_READ_ISR	0x0b	// Read ISR Interrupt Request Register

static inline bool isDivisibleBy8(uint8_t num){
	return ((num & 7) == 0);
}

void x86_PIC_Remap(uint8_t offsetMasterPIC, uint8_t offsetSlavePIC){
	if ( !isDivisibleBy8(offsetMasterPIC) || !isDivisibleBy8(offsetSlavePIC) ){
		printf("PIC Configuration error: tried to remap with an incorrect offset\n");
		printf("Both %p and %p must be divisible by 8\n", offsetMasterPIC, offsetSlavePIC);
		PANIC();
	}

	// According to this forum: https://forum.osdev.org/viewtopic.php?t=30111
	// We don't need to io_wait in between PIC coms if we interleave them

	// ICW1
	outb(PIC_MASTER_CMD, PIC_ICW1_ICW4|PIC_ICW1_INIT);
	outb(PIC_SLAVE_CMD, PIC_ICW1_ICW4|PIC_ICW1_INIT);
	// ICW2 - Offsets
	outb(PIC_MASTER_DATA, offsetMasterPIC);
	outb(PIC_SLAVE_DATA, offsetSlavePIC);
	// ICW3
	outb(PIC_MASTER_DATA, 4); // Tell master for the slave at IRQ2 (0000 0100)
	outb(PIC_SLAVE_DATA, 2); // Tell slave for its cascade identity (0000 0010)
	// ICW4
	outb(PIC_MASTER_DATA, PIC_ICW4_8086); // Have the PICs use 8086 mode (and not 8080 mode)
	outb(PIC_SLAVE_DATA, PIC_ICW4_8086);

	x86_PIC_EnableAllIRQ();
}

void x86_PIC_DisableIRQ(uint8_t irq){
	if (irq>=16) return; // Ignore invalid IRQ number

	uint8_t port;

	// Master PIC
	if (irq < 8){
		port = PIC_MASTER_DATA;
	}
	// Slave PIC
	else {
		port = PIC_SLAVE_CMD;
		irq -= 8;
	}

	uint8_t mask = inb(port);
	mask |= (1 << irq); // Add the corresponding bit to the mask
	outb(port, mask);
}

void x86_PIC_EnableIRQ(uint8_t irq){
	if (irq>=16) return; // Ignore invalid IRQ number
	uint8_t port;

	// Master PIC
	if (irq < 8){
		port = PIC_MASTER_DATA;
	}
	// Slave PIC
	else {
		port = PIC_SLAVE_DATA;
		irq -= 8;
	}
	uint8_t mask = inb(port);
	mask &= ~(1 << irq); // Remove the corresponding bit to the mask
	outb(port, mask);
}

void x86_PIC_EnableAllIRQ(){
	// Unmask all interrupts
	outb(PIC_MASTER_DATA, 0x00);
	outb(PIC_SLAVE_DATA, 0x00);
}

void x86_PIC_DisableAllIRQ(){
	// Unmask all interrupts
	outb(PIC_MASTER_DATA, 0x00);
	outb(PIC_SLAVE_DATA, 0x00);
}

void x86_PIC_Disable(){
	x86_PIC_DisableAllIRQ();
}

void x86_PIC_SendEIO(int irq){
	if (irq >= 8) outb(PIC_SLAVE_CMD, PIC_CMD_EOI);
	outb(PIC_MASTER_CMD, PIC_CMD_EOI);
}

static inline uint16_t x86_PIC_GetCombinedRegister(int ocw3){
	// OCW3 to PIC CMD to get the register values. PIC_SLAVE is chained, and
	// represents IRQs 8-15. PIC_MASTER is IRQs 0-7, with 2 being the chain
	outb(PIC_MASTER_CMD, ocw3);
	outb(PIC_SLAVE_CMD, ocw3);
	return (inb(PIC_SLAVE_CMD) << 8) | inb(PIC_MASTER_CMD);
}

uint16_t x86_PIC_GetCombinedIRR(){
	return x86_PIC_GetCombinedRegister(PIC_CMD_READ_IRR);
}

uint16_t x86_PIC_GetCombinedISR(void){
	return x86_PIC_GetCombinedRegister(PIC_CMD_READ_ISR);
}
