#include <stdint.h>
#include "Logging.h"
#include "Memory/VMM.h"
#include "IRQ.h"
#include "Registers.h"
#include "Drivers/i8259.h"

#include "APIC.h"
#define MODULE_APIC "APIC"
#define MODULE_IOAPIC "I/O APIC"

// ================ APIC ================

// Memory-mapped APIC registers page address
#define APIC_REGISTERS 0x00000000fee00000
static const uint8_t* m_apicRegs = (uint8_t*) APIC_REGISTERS;

// Memory-mapped APIC registers offsets in the page
#define APIC_REG_ID						0x020
#define APIC_REG_VERSION				0x030
#define APIC_REG_TPR					0x080 // Task Priority Register
#define APIC_REG_EOI					0x0b0 // End Of Interrupt register
#define APIC_REG_DFR					0x0e0 // Destination Format Register
#define APIC_REG_SPURIOUS_INTERRUPT		0x0f0
#define APIC_REG_ISR					0x100 // In-Service Register (256 bits)
#define APIC_REG_TMR					0x180 // Trigger Mode Register (256 bits)
#define APIC_REG_IRR					0x200 // Interrupt Request Register (256 bits)
#define APIC_REG_TIMER					0x320
#define APIC_REG_LINT0					0x350
#define APIC_REG_LINT1					0x360
#define APIC_REG_ERROR					0x370
#define APIC_REG_TIMER_INITIAL_COUNT	0x380
#define APIC_REG_TIMER_COUNT			0x390
#define APIC_REG_TIMER_DIVIDE			0x3e0

// APIC Timer available modes
#define APIC_TIMER_MODE_ONESHOT			0b00
#define APIC_TIMER_MODE_PERIODIC		0b01
#define APIC_TIMER_MODE_TSCDEADLINE		0b10

// APIC Timer divisors values (note: bit 3 is reserved)
#define APIC_TIMER_DIVISOR_1			0b1011
#define APIC_TIMER_DIVISOR_2			0b0000
#define APIC_TIMER_DIVISOR_4			0b0001
#define APIC_TIMER_DIVISOR_8			0b0010
#define APIC_TIMER_DIVISOR_16			0b0011
#define APIC_TIMER_DIVISOR_32			0b1000
#define APIC_TIMER_DIVISOR_64			0b1001
#define APIC_TIMER_DIVISOR_128			0b1010

// Interrupt delivery mode
#define APIC_DELIVERY_FIXED				0b000
#define APIC_DELIVERY_SMI				0b010
#define APIC_DELIVERY_NMI				0b100
#define APIC_DELIVERY_EXTINT			0b111
#define APIC_DELIVERY_INIT				0b101

#define APIC_DFR_MODEL_FLAT				0b1111
#define APIC_DFR_MODEL_CLUSTER			0b0000

union VersionRegister {
	uint32_t value;
	struct {
		uint32_t version : 8;
		uint32_t reserved_0 : 8;
		uint32_t maxLVTEntry : 8;
		uint32_t EIOBroadcastSupressionSupport : 1;
		uint32_t reserved_1 : 7;
	} bits;
};

union LINTRegister {
	uint32_t value;
	struct {
		uint32_t vector : 8;
		uint32_t deliveryMode : 3;
		uint32_t reserved_0 : 1;
		uint32_t pending : 1;
		uint32_t pinPolarity : 1;
		uint32_t remoteIRR : 1;
		uint32_t triggerMode : 1;
		uint32_t masked : 1;
	} bits;
};

union TimerRegister {
	uint32_t value;
	struct {
		uint32_t vector : 8;
		uint32_t reserved_0 : 4;
		uint32_t pending : 1;
		uint32_t reserved_1 : 3;
		uint32_t masked : 1;
		uint32_t timerMode : 2;
		uint32_t reserved_2 : 13;
	} bits;
};

union DestinationFormatRegister {
	uint32_t value;
	struct {
		uint32_t reserved_all_ones : 28;
		uint32_t model : 4;
	} bits;
};

union TaskPriorityRegister {
	uint32_t value;
	struct {
		uint32_t subclass : 4;
		uint32_t class : 4;
	} bits;
};

union SpuriousInterruptRegister {
	uint32_t value;
	struct {
		uint32_t vector : 8;
		uint32_t APICEnabled : 1;
		uint32_t focusProcessorChecking : 1;
		uint32_t reserved_0 : 2;
		uint32_t EIOBroadcastSupression : 1;
	} bits;
};

struct Register256 {
	uint32_t value0;
	uint32_t value1;
	uint32_t value2;
	uint32_t value3;
	uint32_t value4;
	uint32_t value5;
	uint32_t value6;
	uint32_t value7;
};

// We cache some registers
static union TimerRegister m_timerReg;

static inline uint32_t readRegister32(int offset){
	return *((uint32_t*)(m_apicRegs + offset));
}

static inline void writeRegister32(int offset, uint32_t val){
	*((uint32_t*)(m_apicRegs + offset)) = val;
}

static unused inline void readRegister256(const int offset, struct Register256* reg_out){
	reg_out->value0 = readRegister32(offset);
	reg_out->value1 = readRegister32(offset + 0x10);
	reg_out->value2 = readRegister32(offset + 0x20);
	reg_out->value3 = readRegister32(offset + 0x30);
	reg_out->value4 = readRegister32(offset + 0x40);
	reg_out->value5 = readRegister32(offset + 0x50);
	reg_out->value6 = readRegister32(offset + 0x60);
	reg_out->value7 = readRegister32(offset + 0x70);
}

// Temporary: blinking rectangle on the bottom right of the screen
#include "Drivers/Graphics/Framebuffer.h"
static void timerIRQ(void*){
	extern Framebuffer m_framebuffer;
	static bool clock = false;
	const int rect_size = 4;
	color_t color = (clock) ? COLOR_32BPP(0,128,0) : LIGHT_GREY;
	Framebuffer_fillRectangle(&m_framebuffer, m_framebuffer.width-2*rect_size-2, m_framebuffer.height-rect_size-1, rect_size, rect_size, color);
	clock = !clock;
}

static void handleSpuriousIRQ(void*){
	static int n_spurious_irqs = 0;
	n_spurious_irqs++;
	log(WARNING, MODULE_APIC, "Got spurious IRQ (count is now %d)", n_spurious_irqs);
}

void APIC_init(){
	// Remap the PIC. We need to do it even if we don't use it, as it
	// can still fire spurious IRQs
	i8259_init();
	i8259_disableAllIRQ();

	// Setup the APIC in its MSR
	union MSR_IA32_APIC_BASE apic_base;
	apic_base.value = Registers_readMSR(MSR_ADDR_IA32_APIC_BASE);
	apic_base.bits.GlobalEnable = true;
	apic_base.bits.x2APIC = false; // we operate in xAPIC mode
	Registers_writeMSR(MSR_ADDR_IA32_APIC_BASE, apic_base.value);

	// Memory-map the APIC registers -> TODO use kernel memory instead !
	VMM_map(APIC_REGISTERS, APIC_REGISTERS, 1, PAGE_READ|PAGE_WRITE|PAGE_CACHE_DISABLED|PAGE_KERNEL);

	// DEBUG INFO
	union VersionRegister version;
	version.value = readRegister32(APIC_REG_VERSION);
	debug("APIC id=%p version=%#hhx maxLVTEntry=%d EIOBroadcastSupressionSupport=%d",
		readRegister32(APIC_REG_ID), version.bits.version, version.bits.maxLVTEntry,
		version.bits.EIOBroadcastSupressionSupport);

	// Setup LINT0 and LINT1
	union LINTRegister lint0, lint1;
	lint0.value = 0;
	lint0.bits.vector = 32 + 1;
	lint0.bits.deliveryMode = APIC_DELIVERY_EXTINT;
	lint0.bits.masked = false;
	lint1.value = 0;
	lint1.bits.vector = 32 + 2;
	lint1.bits.deliveryMode = APIC_DELIVERY_SMI;
	lint1.bits.masked = false;
	writeRegister32(APIC_REG_LINT0, lint0.value);
	writeRegister32(APIC_REG_LINT1, lint1.value);

	// Setup the TPR (Task Priority Register)
	union TaskPriorityRegister tpr;
	tpr.value = 0;
	tpr.bits.class = 0;
	tpr.bits.subclass = 0;
	writeRegister32(APIC_REG_TPR, tpr.value);

	// Setup the DFR (Destination Format Register)
	union DestinationFormatRegister dfr;
	dfr.bits.model = APIC_DFR_MODEL_FLAT;
	dfr.bits.reserved_all_ones = 0xffffff;
	writeRegister32(APIC_REG_DFR, dfr.value);

	// Install a spurious interrupt handler
	IRQ_registerHandler(IRQ_APIC_SPURIOUS, handleSpuriousIRQ); // TODO FIX this is wrong since generic IRQ handling sends EOI

	// Finally, set the 'enable' bit in the Spurious Interrupt Register
	union SpuriousInterruptRegister spur;
	spur.value = 0;
	spur.bits.vector = IRQ_APIC_SPURIOUS;
	spur.bits.APICEnabled = true;
	writeRegister32(APIC_REG_SPURIOUS_INTERRUPT, spur.value);

	// Setup the timer
	m_timerReg.bits.vector = IRQ_APIC_TIMER;
	m_timerReg.bits.timerMode = APIC_TIMER_MODE_PERIODIC;
	m_timerReg.bits.masked = false;
	writeRegister32(APIC_REG_TIMER, m_timerReg.value);

	IRQ_registerHandler(IRQ_APIC_TIMER, timerIRQ);
	writeRegister32(APIC_REG_TIMER_DIVIDE, APIC_TIMER_DIVISOR_1);
	writeRegister32(APIC_REG_TIMER_INITIAL_COUNT, 1000000000); // 1GHz bus speed => 1s
}

void APIC_sendEIO(int){
	writeRegister32(APIC_REG_EOI, 0);
}

// ================ IO APIC ================

// Unimplemented yet
