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

static struct SteadyTimer* m_pmTimer;
static uint16_t m_port;

static uint64_t readCounterIoPort(){
	return inl(m_port);
}

static uint64_t readCounterMMIO(){
	log(PANIC, MODULE, "MMIO is currently unsupported for PM Timer !");
	panic();
}

static void initIoPort(uint16_t port){
	m_port = port;
	m_pmTimer->read = readCounterIoPort;
	log(SUCCESS, MODULE, "Initialized PM Timer on port %#hx, with mask %#x", m_port, m_pmTimer->mask);
}

static void initMMIO(paddr_t){
	m_pmTimer->read = readCounterMMIO;
	log(PANIC, MODULE, "MMIO is currently unsupported for PM Timer !");
	panic();
}

// ================ Public API ================

void PMTimer_init(struct SteadyTimer* pmtimer){
	assert(PMTimer_isPresent());

	m_pmTimer = pmtimer;
	m_pmTimer->frequency = BASE_FREQUENCY;
	m_pmTimer->name = "PM Timer";

	// The internal counter is 24 bits by default, and may be extended to 32 bits
	m_pmTimer->mask = (g_FADT.fixedFeatureFlags.TMR_VAL_EXT == 0) ? 0xffffff : 0xffffffff;

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
