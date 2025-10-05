#include "assert.h"
#include "IO.h"
#include "Panic.h"
#include "Logging.h"
#include "Drivers/ACPI/ACPI.h"
#include "Memory/VMM.h"
#include "HAL/Halt.h"

#include "PMTimer.h"
#define MODULE "PM Timer"

#define BASE_FREQUENCY 3579545 // 3.579545 MHz clock

static uint32_t m_mask;
static uint16_t m_port;

static uint32_t (*readCounter)(void);

static uint32_t readCounterIoPort(){
	return inl(m_port);
}

static uint32_t readCounterMMIO(){
	log(PANIC, MODULE, "MMIO is currently unsupported for PM Timer !");
	panic();
}

static void initIoPort(uint16_t port){
	m_port = port;
	readCounter = readCounterIoPort;
	log(SUCCESS, MODULE, "Initialized PM Timer on port %#hx, with mask %#x", m_port, m_mask);
}

static void initMMIO(paddr_t){
	readCounter = readCounterMMIO;
	log(PANIC, MODULE, "MMIO is currently unsupported for PM Timer !");
	panic();
}

// Temporary stub. We obviously won't support second-scale busy-waiting...
// Note that it doesn't take loop-back into account, too
static unused void delay(unsigned long seconds){
	uint64_t n_ticks = BASE_FREQUENCY*seconds;

	uint32_t t0 = readCounter();
	while ((readCounter() - t0) < n_ticks){
		pause();
	}
}

// ================ Public API ================

void PMTimer_init(){
	assert(PMTimer_isPresent());

	// The internal counter is 24 bit by default, and may be extended to 32 bits
	m_mask = (g_FADT.fixedFeatureFlags.TMR_VAL_EXT == 0) ? 0xffffff : 0xffffffff;

	paddr_t mmio = g_FADT.X_PMTimerBlock.address[1];
	mmio = (mmio << 32) | g_FADT.X_PMTimerBlock.address[0];

	// According to the ACPI spec, if X_PM_TMR_BLK is present (mmio != 0),
	// we shall use it over PM_TMR_BLK ; else we use PM_TMR_BLK

	if (mmio != 0){
		switch (g_FADT.X_PMTimerBlock.addressSpace){
		case 0:
			initMMIO(mmio);
			break;
		case 1:
			initIoPort((uint16_t) mmio);
			break;
		default:
			// Should not happen
			log(PANIC, MODULE, "Got unexpected enum addressSpace value %d",
				g_FADT.X_PMTimerBlock.addressSpace);
			panic();
		}
	}
	else if (g_FADT.PMTimerBlock != 0) {
		initIoPort(g_FADT.PMTimerBlock);
	}
	else {
		// No field are available. This should not happen, but since both are optional,
		// we still check for this case
		log(PANIC, MODULE, "PM Timer is present but the firmware provided no way of accessing it");
		panic();
	}
}

bool PMTimer_isPresent(){
	// PMTimerLength is 4 if PM Timer is present, else 0
	return (g_FADTPresent && g_FADT.PMTimerLength == 4);
}
