#include <stdint.h>
#include "assert.h"
#include "IO.h"
#include "Logging.h"
#include "Memory/VMM.h"
#include "SMP/SMP.h"
#include "IRQ.h"
#include "Registers.h"
#include "ISR.h"
#include "Drivers/i8259.h"
#include "Drivers/ACPI/ACPI.h"
#include "Drivers/IOAPIC.h"

#include "APIC.h"
#define MODULE_APIC "APIC"

// ================ APIC ================

// Memory-mapped APIC registers page address
#define APIC_REGISTERS_ADDR_DEFAULT 0x00000000fee00000
volatile static void* m_apicRegs;

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
	return read32(m_apicRegs+offset);
}

static inline void writeRegister32(int offset, uint32_t val){
	write32(m_apicRegs+offset, val);
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

static void handleSpuriousIRQ(struct ISR_Params* params){
	uint64_t vector = params->vector;

	static int n_spurious_irqs = 0;
	n_spurious_irqs++;

	if (vector == IRQ_APIC_SPURIOUS){
		log(ERROR, MODULE_APIC, "Spurious APIC interrupt, should not happen. Wtf ?");
		return;
	}

	// Read only the part which contains the 'vector-th' bit
	uint32_t isr = readRegister32(APIC_REG_ISR + ((vector & ~0x1f) >> 1));
	bool bit_present = (isr & (1 << (vector & 0x1f)));

	// Ack the interrupt only if it's actually pending
	if (bit_present){
		log(WARNING, MODULE_APIC, "Got spurious interrupt %d, acked (count is now %d)",
			vector, n_spurious_irqs);
		APIC_sendEIO(0);
	}
	else {
		log(WARNING, MODULE_APIC, "Got spurious interrupt %d, not pending (count is now %d)",
			vector, n_spurious_irqs);
	}
}

static physical_address_t getAPICAddress(){
	if (!g_MADTPresent)
		return APIC_REGISTERS_ADDR_DEFAULT;

	physical_address_t addr = (physical_address_t) g_MADT.localApicAddress;

	if (g_MADT.nLAPIC_ADDR_OVERRIDE == 0)
		return addr;

	if (g_MADT.nLAPIC_ADDR_OVERRIDE > 1)
		log(WARNING, MODULE_APIC, "Found several APIC address overrides, should not happen !!");

	log(INFO, MODULE_APIC, "Found APIC address override, using its address");
	return (physical_address_t) g_MADT.LAPIC_ADDR_OVERRIDEs[0].address;
}

/// @brief Apply the NMI configurations (given by ACPI) to the LINT pins
static void configurePins(int cpu_id){
	struct MADTEntry_LAPIC_NMI* entry;
	union LINTRegister lint;
	lint.value = 0;

	for (int i=0 ; i<g_MADT.nLAPIC_NMI ; i++){
		const int cur_id = g_MADT.LAPIC_NMIs[i].ACPIProcessorID;
		if (cur_id != cpu_id && cur_id != 0xff)
			continue;

		assert((entry->LINTi & ~1) == 0); // LINTi should only be 0 or 1

		// Apply the NMI configuration specified by the entry
		entry = g_MADT.LAPIC_NMIs + i;
		lint.bits.vector = 0; // does not matter, ignored by hardware
		lint.bits.deliveryMode = APIC_DELIVERY_NMI;
		lint.bits.pinPolarity = entry->flags.bits.pinPolarity;
		lint.bits.triggerMode = 0; // edge-triggered, Intel SDM states that we shall ignore the ACPI value
		lint.bits.masked = false;
		writeRegister32((entry->LINTi == 0) ? APIC_REG_LINT0 : APIC_REG_LINT1, lint.value);
	}
}

// ================ Public interface ================

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

	// If ACPI MADT is present, use the address it provides
	physical_address_t apic_regs_phys = getAPICAddress();
	virtual_address_t apics_regs_virt = apic_regs_phys | VMM_KERNEL_MEMORY;
	m_apicRegs = (uint8_t*) apics_regs_virt;

	// Memory-map the APIC registers
	VMM_map(apic_regs_phys, apics_regs_virt, 1, PAGE_READ|PAGE_WRITE|PAGE_CACHE_DISABLED|PAGE_KERNEL);

	configurePins(SMP_getCpuId());

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
	ISR_installHandler(IRQ_APIC_SPURIOUS, handleSpuriousIRQ);

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

	IRQ_installHandler(IRQ_APIC_TIMER, timerIRQ);
	writeRegister32(APIC_REG_TIMER_DIVIDE, APIC_TIMER_DIVISOR_1);
	writeRegister32(APIC_REG_TIMER_INITIAL_COUNT, 1000000000); // 1GHz bus speed => 1s

	log(SUCCESS, MODULE_APIC, "Initalized local APIC (ID=%d)", readRegister32(APIC_REG_ID));

	IOAPIC_init();
}

void APIC_sendEIO(int){
	writeRegister32(APIC_REG_EOI, 0);
}
