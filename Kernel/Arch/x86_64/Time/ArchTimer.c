#include "Logging.h"
#include "Drivers/Timers/PIT.h"
#include "Time/Timer.h"

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

static void unimplemented(unsigned long){
	debug("Unimplemented !!");
}

void ArchTimer_init(struct Timer* timer){
	PIT_init();

	timer->sleep = PIT_sleep;
	timer->msleep = PIT_msleep;
	timer->usleep = PIT_usleep;
	timer->nsleep = PIT_nsleep;

	timer->mdelay = unimplemented;
	timer->udelay = unimplemented;
	timer->ndelay = unimplemented;

	log(SUCCESS, MODULE, "Initialized with: PIT sleep, unimplemented delay");
}
