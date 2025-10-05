#include "Logging.h"
#include "Panic.h"
#include "Drivers/Timers/PIT.h"
#include "Drivers/Timers/PMTimer.h"
#include "Time/Timer.h"
#include "CPU.h"

#include "HAL/Time/ArchTimer.h"
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

void ArchTimer_init(struct Timer* timer){
	const char* schedulingClock;
	const char* timekeepingClock;

	// Scheduling clock, for sleep & the scheduler
	if (g_CPU.features.bits.APIC && false){
		schedulingClock = "LAPIC Timer";
		// LAPIC Timer unsupported yet, but planned
	}
	else {
		// PIT is guaranteed to be present/emulated
		PIT_init();
		timer->sleep = PIT_sleep;
		timer->msleep = PIT_msleep;
		timer->usleep = PIT_usleep;
		timer->nsleep = PIT_nsleep;
		schedulingClock = "PIT";
	}

	// Timekeeping clock, for busy-wait delay
	if (g_CPU.features.bits.TSC && g_CPU.extFeatures.bits.InvariantTSC && false){
		timekeepingClock = "TSC";
		// (Invariant) TSC unsupported yet, but planned
	}
	else if (PMTimer_isPresent()) {
		PMTimer_init();
		timekeepingClock = "PM Timer";
	}
	else {
		log(PANIC, MODULE, "No supported dt clock source found !!");
		panic();
	}

	log(SUCCESS, MODULE, "Initialized with %s scheduling clock & %s timekeeping clock",
		schedulingClock, timekeepingClock);
}
