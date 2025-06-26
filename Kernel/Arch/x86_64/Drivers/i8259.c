#include "io.h"
#include "Preprocessor.h"
#include "Logging.h"
#include "Panic.h"

#include "i8259.h"
#define MODULE "i8259 PIC"

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
#define PIC_ICW4_SFNM		0x10	// Special fully nested (not)
// PIC Commands
#define PIC_CMD_EOI			0x20	// EOI End Of Interrupt
#define PIC_CMD_READ_IRR	0x0a	// Read IRR Interrupt Request Register
#define PIC_CMD_READ_ISR	0x0b	// Read ISR In-Service Register

#define isValidIRQ(irq)	(irq >= 0 && irq < 16)

static inline bool isDivisibleBy8(uint8_t num){
	return ((num & 7) == 0);
}

static inline uint16_t getCombinedRegister(int ocw3){
	// OCW3 to PIC CMD to get the register values. PIC_SLAVE is chained, and
	// represents IRQs 8-15. PIC_MASTER is IRQs 0-7, with 2 being the chain
	outb(PIC_MASTER_CMD, ocw3);
	outb(PIC_SLAVE_CMD, ocw3);
	return (inb(PIC_SLAVE_CMD) << 8) | inb(PIC_MASTER_CMD);
}

// Returns the combined IRR (Interrupt Request Register) values:
// [ bit 15-8 Slave's IRR - bit 8-0 Master's IRR ]
static unused uint16_t i8259_getCombinedIRR(){
	return getCombinedRegister(PIC_CMD_READ_IRR);
}

// Returns the combined ISR (In-Service Register) values:
// [ bit 15-8 Slave's ISR - bit 8-0 Master's ISR ]
static uint16_t i8259_getCombinedISR(){
	return getCombinedRegister(PIC_CMD_READ_ISR);
}

void i8259_remap(uint8_t offsetMasterPIC, uint8_t offsetSlavePIC){
	if ( !isDivisibleBy8(offsetMasterPIC) || !isDivisibleBy8(offsetSlavePIC) ){
		log(PANIC, MODULE, "PIC Configuration error: tried to remap with an incorrect offset");
		log(PANIC, MODULE, "Both %p and %p must be divisible by 8", offsetMasterPIC, offsetSlavePIC);
		panic();
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

	i8259_enableAllIRQ();
}

void i8259_disableIRQ(int irq){
	if (isValidIRQ(irq)) return;

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
	mask |= (1 << irq); // Add the corresponding bit to the mask
	outb(port, mask);
}

void i8259_enableIRQ(int irq){
	if (isValidIRQ(irq)) return;
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

void i8259_enableAllIRQ(){
	// Unmask all interrupts
	outb(PIC_MASTER_DATA, 0x00);
	outb(PIC_SLAVE_DATA, 0x00);
}

void i8259_disableAllIRQ(){
	// Mask all interrupts
	outb(PIC_MASTER_DATA, 0xff);
	outb(PIC_SLAVE_DATA, 0xff);
}

void i8259_sendEIO(int irq){
	if (isValidIRQ(irq)) return;

	if (irq >= 8)
		outb(PIC_SLAVE_CMD, PIC_CMD_EOI);
	outb(PIC_MASTER_CMD, PIC_CMD_EOI);
}

bool i8259_handleSpuriousIRQ(int irq){
	uint16_t isr = i8259_getCombinedISR();

	// Spurious IRQ 7
	if ((isr & (1<<7)) == 0){
		log(WARNING, MODULE, "Got spurious IRQ %d, ignored (no EOI sent)", irq);
		return true;
	}

	// Spurious IRQ 15
	// We don't still need to ACK the interrupt to the slave PIC,
	//  but we do for the master PIC though
	if ((isr & (1<<15)) == 0){
		log(WARNING, MODULE, "Got spurious IRQ %d, ignored (no EOI sent)", irq);
		i8259_sendEIO(7); // Master PIC ACK
		return true;
	}

	return false;
}
