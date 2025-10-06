#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "Time/Timers.h"
#include "CPU.h"
#include "Drivers/Timers/PIT.h"
#include "Drivers/Timers/PMTimer.h"

#include "HAL/Time/ArchTimers.h"
#define MODULE "ArchTimer"

// Summary of available x86 timers:
//
// Timer name    Access  Count           Width  Counter freq  IRQs  Max IRQ freq  Comment
// ============  ======  ==============  =====  ============  ====  ============  =======
// PIT           IO      Monotonic up    16     1.193 MHz     Yes   < 1.193 MHz   Simple, old AF
// RTC           IO      Monotonic up    24     1 Hz          Yes   8 kHz         Slow AF
// LAPIC timer   MMIO    Monotonic down  32     To measure    Yes   To measure    Fucking awesome
// PM timer      MMIO    Monotonic up    24/32  3.579 MHz     No    N/A           Reliable and simple
// HPET          MMIO    Monotonic up    64     10-100 MHz    Yes   10-100 MHz    To avoid
// TSC           rdtsc   Up              64     CPU clock     No    N/A           Variable frequency, sucks
// InvariantTSC  rdtsc   Monotonic up    64     To measure    No    N/A           Fucking awesome

void ArchTimers_initEventTimer(struct EventTimer* timer){
	assert(timer);

	if (g_CPU.features.bits.APIC && false){
		timer->name = "LAPIC Timer";
		// LAPIC Timer unsupported yet, but planned
	}
	else {
		// PIT is guaranteed to be present/emulated
		PIT_init(timer);
	}
}

void ArchTimers_initSteadyTimer(struct SteadyTimer* timer){
	assert(timer);

	if (g_CPU.features.bits.TSC && g_CPU.extFeatures.bits.InvariantTSC && false){
		// (Invariant) TSC unsupported yet, but planned
	}
	else if (PMTimer_isPresent()) {
		PMTimer_init(timer);
	}
	else {
		log(PANIC, MODULE, "No supported dt clock source found !!");
		panic();
	}
}
