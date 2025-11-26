#include <stdint.h>
#include "assert.h"
#include "Panic.h"
#include "Logging.h"
#include "IRQ/IRQ.h"
#include "Time/Time.h"
#include "Memory/VMM.h"
#include "Drivers/ACPI/ACPI.h"
#include "HAL/SMP/PerCPU.h"
#include "IO.h"
#include "CPU.h"
#include "ISR.h"
#include "Drivers/IrqChip/i8259.h"
#include "Drivers/IrqChip/IOAPIC.h"
#include "Drivers/Timers/TSC.h"

#include "APIC.h"
#define MODULE "APIC"

// ================ APIC ================

// Memory-mapped APIC registers page address
#define APIC_REGISTERS_ADDR_DEFAULT 0x00000000fee00000
static volatile void* m_apicRegs;

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
#define APIC_REG_ICR					0x300 // Interrupt Command Register (64 bits)
#define APIC_REG_TIMER					0x320 // LVT: Local APIC Timer register
#define APIC_REG_LINT0					0x350
#define APIC_REG_LINT1					0x360
#define APIC_REG_ERROR					0x370
#define APIC_REG_TIMER_INITIAL_COUNT	0x380
#define APIC_REG_TIMER_COUNT			0x390
#define APIC_REG_TIMER_DIVIDE			0x3e0 // Possible register values: APIC_TIMER_DIVISOR_*

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
// Note: some are invalid, depending on the register used
#define APIC_DELIVERY_FIXED				0b000
#define APIC_DELIVERY_LOWEST_PRIORITY	0b001
#define APIC_DELIVERY_SMI				0b010
#define APIC_DELIVERY_NMI				0b100
#define APIC_DELIVERY_INIT				0b101
#define APIC_DELIVERY_STARTUP			0b110
#define APIC_DELIVERY_EXTINT			0b111

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

union TaskPriorityRegister {
	uint32_t value;
	struct {
		uint32_t subclass : 4;
		uint32_t class : 4;
	} bits;
};

union DestinationFormatRegister {
	uint32_t value;
	struct {
		uint32_t reserved_all_ones : 28;
		uint32_t model : 4;
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

union InterruptCommandRegister {
	uint64_t value;
	struct {
		uint64_t vector : 8;
		uint64_t deliveryMode : 3; // see APIC_DELIVERY_*
		uint64_t destinationMode : 1;
		uint64_t pending : 1;
		uint64_t reserved_0 : 1;
		uint64_t level : 1; // 0=de-assert 1=assert
		uint64_t triggerMode : 1; // 0=edge 1=level
		uint64_t reserved_1 : 2;
		uint64_t destinationShorthand : 2; // 0b00=no shorthand 0b01=self 0b10=all 0b11 all but self
		uint64_t reserved_2 : 12 + 24;
		uint64_t destination : 8;
	} bits;
};

union TimerRegister {
	uint32_t value;
	struct {
		// Programmable IRQ delivery vector
		uint32_t vector : 8;
		uint32_t reserved_0 : 4;
		// Whether an IRQ is pending
		uint32_t pending : 1;
		uint32_t reserved_1 : 3;
		// Whether the IRQ is masked
		uint32_t masked : 1;
		// `APIC_TIMER_MODE_ONESHOT = 0b00`: one-shot mode
		// `APIC_TIMER_MODE_PERIODIC = 0b01`: periodic IRQ mode
		// `APIC_TIMER_MODE_TSCDEADLINE = 0b10`: one-shot, TSC value mode
		uint32_t timerMode : 2;
		uint32_t reserved_2 : 13;
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

static inline uint32_t readRegister32(int offset){
	return read32(m_apicRegs+offset);
}

static inline void writeRegister32(int offset, uint32_t val){
	write32(m_apicRegs+offset, val);
}

static unused inline uint64_t readRegister64(int offset){
	uint64_t res;
	res = (uint64_t) read32(m_apicRegs+offset + 0x10) << 32;
	res = (uint64_t) read32(m_apicRegs+offset);
	return res;
}

static inline void writeRegister64(int offset, uint64_t val){
	write32(m_apicRegs+offset+0x10,	val >> 32);
	write32(m_apicRegs+offset, val);
}

static void handleSpuriousIRQ(struct ISR_Params* params){
	uint64_t vector = params->vector;

	static int n_spurious_irqs = 0;
	n_spurious_irqs++;

	if (vector == IRQ_APIC_SPURIOUS){
		log(ERROR, MODULE, "Spurious APIC interrupt, should not happen. Wtf ?");
		return;
	}

	// Read only the part which contains the 'vector-th' bit
	uint32_t isr = readRegister32(APIC_REG_ISR + ((vector & ~0x1f) >> 1));
	bool bit_present = (isr & (1 << (vector & 0x1f)));

	// Ack the interrupt only if it's actually pending
	if (bit_present){
		log(WARNING, MODULE, "Got spurious interrupt %d, acked (count is now %d)",
			vector, n_spurious_irqs);
		APIC_sendEIO(0);
	}
	else {
		log(WARNING, MODULE, "Got spurious interrupt %d, not pending (count is now %d)",
			vector, n_spurious_irqs);
	}
}

static paddr_t getAPICAddress(){
	if (!g_MADTPresent)
		return APIC_REGISTERS_ADDR_DEFAULT;

	if (g_MADT.nLAPIC_ADDR_OVERRIDE == 0)
		return (paddr_t) g_MADT.localApicAddress;

	if (g_MADT.nLAPIC_ADDR_OVERRIDE > 1)
		log(WARNING, MODULE, "Found several APIC address overrides, should not happen !!");

	log(INFO, MODULE, "Found APIC address override, using its address");
	return (paddr_t) g_MADT.LAPIC_ADDR_OVERRIDEs[0].address;
}

static uint32_t getAcpiProcessorId(uint32_t lapicId){
	for (int i=0 ; i<g_MADT.nLAPIC ; i++){
		if (g_MADT.LAPICs[i].lapicID == lapicId)
			return g_MADT.LAPICs[i].processorID;
	}

	// Shouldn't happen, but better safe than sorry
	log(PANIC, MODULE, "No ACPI processor ID corresponding to LAPIC %d !!",
		lapicId);
	panic();
}

/// @brief Apply the NMI configurations (given by ACPI) to the LINT pins
static void configurePins(int acpiProcessorId){
	struct MADTEntry_LAPIC_NMI* entry;
	union LINTRegister lint;
	lint.value = 0;

	for (int i=0 ; i<g_MADT.nLAPIC_NMI ; i++){
		const int cur_id = g_MADT.LAPIC_NMIs[i].ACPIProcessorID;
		if (cur_id != acpiProcessorId && cur_id != 0xff)
			continue;

		entry = g_MADT.LAPIC_NMIs + i;
		assert((entry->LINTi & ~1) == 0); // LINTi should only be 0 or 1

		// Apply the NMI configuration specified by the entry
		lint.bits.vector = 0; // does not matter, ignored by hardware
		lint.bits.deliveryMode = APIC_DELIVERY_NMI;
		lint.bits.pinPolarity = entry->flags.bits.pinPolarity;
		lint.bits.triggerMode = 0; // edge-triggered, Intel SDM says we must ignore the ACPI value
		lint.bits.masked = false;
		writeRegister32((entry->LINTi == 0) ? APIC_REG_LINT0 : APIC_REG_LINT1, lint.value);
	}
}

// ================ APIC Timer ================

static struct EventTimer m_apicTimer = {
	.name = "APIC Timer",
	.score = 100,
	.frequency = 0,
	.scheduleEvent = NULL
};

static bool m_tscDeadlineMode;

static void timerIrq(void*){
	m_apicTimer.eventHandler();
}

static uint64_t measureFrequency(){
	// The calibration is two-fold: we measure using both the system's Event timer (mdelay: freq1)
	// and Steady timer (Time_get: freq2, t0, tf, dt)
	uint64_t freq_temp1, freq_temp2, freq1 = UINT64_MAX, freq2 = UINT64_MAX;
	uint64_t ticks_final, ticks_delta;
	const uint64_t ticks_intial = 0xffffffff;
	ktime_t t0, tf, dt;

	// Calibration setup: IRQ masked (already done), one-shot mode
	writeRegister32(APIC_REG_TIMER_DIVIDE, APIC_TIMER_DIVISOR_1);

	for (int i=0 ; i<5 ; i++){
		t0 = Time_get();
		writeRegister32(APIC_REG_TIMER_INITIAL_COUNT, ticks_intial);
		mdelay(50);
		ticks_final = readRegister32(APIC_REG_TIMER_COUNT);
		tf = Time_get();

		ticks_delta = ticks_intial - ticks_final;
		dt = tf - t0;

		freq_temp1 = ticks_delta * (1000 / 50);
		freq_temp2 = ticks_delta * 1000000000 + dt-1 / dt;

		freq1 = min(freq1, freq_temp1);
		freq2 = min(freq2, freq_temp2);
	}

	// Calibration is done, cancel the pending event since it (very probably) didn't fire
	writeRegister32(APIC_REG_TIMER_INITIAL_COUNT, 0);

	uint64_t freq = min(freq1, freq2);
	return (freq == UINT64_MAX) ? 0 : freq;
}

static uint64_t findFrequency(){
	uint64_t freq;

	// According to the Intel SDM:
	// The APIC timer frequency will be the processor’s bus clock or core crystal clock frequency
	// (when TSC/core crystal clock ratio is enumerated in CPUID leaf 0x15) divided by the value
	// specified in the divide configuration register.
	if (g_CPU.features.bits.BusFrequency != 0){
		freq = g_CPU.features.bits.BusFrequency;
		log(INFO, MODULE, "APIC frequency based on bus frequency: %lu.%03lu MHz",
			freq / 1000000, freq % 1000000 / 1000);
		return g_CPU.features.bits.BusFrequency;
	}

	freq = measureFrequency();
	log(INFO, MODULE, "Fell back to manual APIC frequency calibration: measured %lu.%03lu MHz",
		freq / 1000000, freq % 1000000 / 1000);
	return freq;
}

static void scheduleEvent(unsigned long ticks){
	writeRegister32(APIC_REG_TIMER_INITIAL_COUNT, ticks);
}

static void scheduleEventTsc(unsigned long ticks){
	Registers_writeMSR(MSR_ADDR_IA32_TSC_DEADLINE, TSC_read() + ticks);
}

static void initTimerNoTsc(){
	// Setup the timer
	union TimerRegister timerReg = { 0 };
	timerReg.bits.vector = IRQ_APIC_TIMER;
	timerReg.bits.timerMode = APIC_TIMER_MODE_ONESHOT;
	timerReg.bits.masked = true;
	writeRegister32(APIC_REG_TIMER, timerReg.value);

	uint64_t freq = findFrequency();

	// After frequency calibration, we can enable IRQs
	timerReg.bits.masked = false;
	writeRegister32(APIC_REG_TIMER, timerReg.value);

	// As the initial count register is 32 bits, we divide the frequency a little
	// to allow for longer periodic intervals
	writeRegister32(APIC_REG_TIMER_DIVIDE, APIC_TIMER_DIVISOR_16);
	m_apicTimer.frequency = freq / 16;
	m_apicTimer.minTick = 1;
	m_apicTimer.maxTick = UINT32_MAX;
	m_apicTimer.scheduleEvent = scheduleEvent;
}

static void initTimerTsc(){
	// Setup the timer
	union TimerRegister timerReg = { 0 };
	timerReg.bits.vector = IRQ_APIC_TIMER;
	timerReg.bits.timerMode = APIC_TIMER_MODE_TSCDEADLINE;
	timerReg.bits.masked = false;
	writeRegister32(APIC_REG_TIMER, timerReg.value);

	m_apicTimer.frequency = TSC_getFrequency();
	m_apicTimer.minTick = 1;
	m_apicTimer.maxTick = UINT64_MAX;
	m_apicTimer.scheduleEvent = scheduleEventTsc;
}

// ================ Public API ================

void APIC_init(){
	// Remap the PIC. We need to do it even if we don't use it, as it
	// can still fire spurious IRQs
	i8259_init();
	i8259_disableAllIRQ();

	// Install the APIC spurious IRQ handler (directly as an ISR)
	ISR_installHandler(IRQ_APIC_SPURIOUS, handleSpuriousIRQ);

	// Setup the APIC in its MSR
	union MSR_IA32_APIC_BASE apic_base;
	apic_base.value = Registers_readMSR(MSR_ADDR_IA32_APIC_BASE);
	apic_base.bits.GlobalEnable = true;
	apic_base.bits.x2APIC = false; // we operate in xAPIC mode
	Registers_writeMSR(MSR_ADDR_IA32_APIC_BASE, apic_base.value);

	// If ACPI MADT is present, use the address it provides
	paddr_t apic_regs_phys = getAPICAddress();
	vaddr_t apics_regs_virt = apic_regs_phys | VMM_KERNEL_MEMORY;
	m_apicRegs = (uint8_t*) apics_regs_virt;

	// Memory-map the APIC registers
	VMM_map(apic_regs_phys, apics_regs_virt, 1, PAGE_READ|PAGE_WRITE|PAGE_CACHE_DISABLED|PAGE_KERNEL);

	// Initialize the BSP's LAPIC
	// Other LAPICs will be initialized by the SMP system
	APIC_initLAPIC();

	// Finally, initialize the I/O APIC(s)
	IOAPIC_init();
}

void APIC_initLAPIC(){
	uint32_t lapicId = readRegister32(APIC_REG_ID);
	uint32_t acpi_processor_id = getAcpiProcessorId(lapicId);

	PerCPU_setCPUInfoMember(apicID, lapicId);

	configurePins(acpi_processor_id);

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

	// Finally, set the 'enable' bit in the Spurious Interrupt Register
	// Note: the IRQ handlers are installed already in the module's init code
	union SpuriousInterruptRegister spur;
	spur.value = 0;
	spur.bits.vector = IRQ_APIC_SPURIOUS;
	spur.bits.APICEnabled = true;
	writeRegister32(APIC_REG_SPURIOUS_INTERRUPT, spur.value);

	// Mask the APIC timer IRQ (until we initialize the timer)
	union TimerRegister timerReg = { 0 };
	timerReg.bits.vector = IRQ_APIC_TIMER;
	timerReg.bits.masked = true;
	writeRegister32(APIC_REG_TIMER, timerReg.value);

	log(SUCCESS, MODULE, "Initalized local APIC (ID=%d)", lapicId);
}

void APIC_initTimers(){
	if (!g_CPU.features.bits.APIC){
		log(INFO, MODULE, "APIC not present, aborting timer initialization");
		return;
	}

	// If the CPU supports it, we use TSC deadline mode
	m_tscDeadlineMode = g_CPU.features.bits.TSC_Deadline;

	// Call the appropriate initialization function
	// Note: for now, multiple CPU is NOT supported. We only initialize the BSP
	m_tscDeadlineMode ? initTimerTsc() : initTimerNoTsc();

	IRQ_installHandler(IRQ_APIC_TIMER, timerIrq);
	// Note: no need to call IRQ_enableSpecific, this IRQ does not go through the I/O APIC

	Time_registerEventTimer(&m_apicTimer);

	log(SUCCESS, MODULE, "Initialized APIC Timer%s with a frequency of %lu.%03lu MHz",
		m_tscDeadlineMode ? " in TSC deadline mode," : "",
		m_apicTimer.frequency / 1000000, m_apicTimer.frequency % 1000000 / 1000);
}

void APIC_sendEIO(int){
	writeRegister32(APIC_REG_EOI, 0);
}

void APIC_wakeCPU(int lapicID, paddr_t entry){
	// Send an INIT IPI
	union InterruptCommandRegister icr;
	icr.value = 0;
	icr.bits.vector = 0; // ignored
	icr.bits.deliveryMode = APIC_DELIVERY_INIT;
	icr.bits.destinationMode = 0; // physical
	icr.bits.level = 0; // must be 0 for INIT, 1 otherwise
	icr.bits.triggerMode = 0; // edge
	// No shorthand: send the IPI to the CPU specified in the destination field
	icr.bits.destinationShorthand = 0b00;
	icr.bits.destination = lapicID; // INIT CPU#cpu
	writeRegister64(APIC_REG_ICR, icr.value);

	msleep(10);

	// Then, send SIPI
	// Destination and triggerMode don't differ from previous call
	icr.bits.vector = (uintptr_t) entry >> PAGE_SHIFT;
	icr.bits.deliveryMode = APIC_DELIVERY_STARTUP;
	icr.bits.level = 1;
	writeRegister64(APIC_REG_ICR, icr.value);

	msleep(10);

	// Second SIPI, same parameters
	writeRegister64(APIC_REG_ICR, icr.value);
}
