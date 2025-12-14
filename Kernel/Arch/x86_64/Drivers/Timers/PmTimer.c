#include "assert.h"
#include "IO.h"
#include "Panic.h"
#include "Logging.h"
#include "Time/Time.h"
#include "Memory/VMM.h"
#include "Drivers/ACPI/ACPI.h"
#include "HAL/Halt.h"

#include "PmTimer.h"
#define MODULE "PM Timer"

#define BASE_FREQUENCY 3579545 // 3.579545 MHz clock

struct PmTimer {
	struct SteadyTimer steadyTimer;
	uint16_t port;
	uint32_t* counter; // Counter register (memory-mapped)
};

static struct PmTimer m_pmTimer = {
	.steadyTimer = {
		.name = "PM Timer",
		.score = 50,
		.read = NULL,
		.frequency = BASE_FREQUENCY,
	},
	.port = 0,
	.counter = NULL
};

static uint64_t readCounterIoPort(){
	return inl(m_pmTimer.port);
}

static uint64_t readCounterMMIO(){
	// The 24 or 32 bits mask doesn't matter here, as the ACPI specification states that
	// this MMIO register's high 8 bits are always 0 if the register' size is 24 bits
	return *m_pmTimer.counter;
}

static void initIoPort(uint16_t port){
	m_pmTimer.port = port;
	m_pmTimer.steadyTimer.read = readCounterIoPort;

	Time_registerSteadyTimer(&m_pmTimer.steadyTimer);

	log(SUCCESS, MODULE, "Initialized PM Timer on port %#hx, with mask %#lx",
		m_pmTimer.port, m_pmTimer.steadyTimer.mask);
}

static void initMMIO(paddr_t physMmioAddr){
	// Map the register's page in kernel virtual memory
	vaddr_t virtMmioAddr = physMmioAddr | VMM_KERNEL_MEMORY;
	VMM_map(physMmioAddr, virtMmioAddr, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);

	m_pmTimer.counter = (uint32_t*) virtMmioAddr;
	m_pmTimer.steadyTimer.read = readCounterMMIO;

	Time_registerSteadyTimer(&m_pmTimer.steadyTimer);

	log(SUCCESS, MODULE, "Initialized PM Timer with register at address %p, with mask %#lx",
		m_pmTimer.counter, m_pmTimer.steadyTimer.mask);
}

/// @return Whether a Power Management Timer is present on the system
static bool isPmTimerPresent(){
	// PMTimerLength is 4 if PM Timer is present, else 0
	return (g_FADTPresent && g_FADT.PMTimerLength == 4);
}

// ================ Public API ================

void PmTimer_init(){
	if (!isPmTimerPresent()){
		log(INFO, MODULE, "Chip is not present");
		return;
	}

	// The internal counter is 24 bits by default, and may be extended to 32 bits
	m_pmTimer.steadyTimer.mask =
		(g_FADT.fixedFeatureFlags.TMR_VAL_EXT == 0) ? 0xffffff : 0xffffffff;

	// Parse the (ACPI) address. Note that an ACPI address can be a MMIO address, a port, or more
	paddr_t acpiAddress = g_FADT.X_PMTimerBlock.address[1];
	acpiAddress = (acpiAddress << 32) | g_FADT.X_PMTimerBlock.address[0];

	// According to the ACPI spec, if X_PM_TMR_BLK is present (acpiAddress != 0),
	// we shall use it over PM_TMR_BLK ; else we use PM_TMR_BLK

	if (acpiAddress != 0){
		switch (g_FADT.X_PMTimerBlock.addressSpace){
		case 0:
			initMMIO(acpiAddress);
			break;
		case 1:
			initIoPort((uint16_t) acpiAddress);
			break;
		default:
			// Should not happen
			log(ERROR, MODULE, "Got unexpected enum addressSpace value %d",
				g_FADT.X_PMTimerBlock.addressSpace);
			return;
		}
	}
	else if (g_FADT.PMTimerBlock != 0){
		initIoPort((uint16_t) g_FADT.PMTimerBlock);
	}
	else {
		// No field available. This should not happen, but since both are optional,
		// we still check for this case
		log(ERROR, MODULE, "PM Timer is present but the firmware provided no way of accessing it");
		return;
	}
}
