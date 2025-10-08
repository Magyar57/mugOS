#include <stddef.h>
#include "Logging.h"
#include "Panic.h"
#include "Time/Timers.h"
#include "HAL/Halt.h"
#include "HAL/Time/ArchTimers.h"

#include "Time.h"
#define MODULE "Time"

static struct SteadyTimer m_steadyTimer;
static struct EventTimer m_eventTimer;

void Time_init(){
	ArchTimers_initSteadyTimer(&m_steadyTimer);
	ArchTimers_initEventTimer(&m_eventTimer);

	log(SUCCESS, MODULE, "Initialized with %s as steady timer & %s as event timer",
		m_steadyTimer.name, m_eventTimer.name);
}

void Time_computeConversion(uint32_t* mult, uint32_t* shift, uint32_t from, uint32_t to, uint32_t maxSec){
	// Compute mult and shift so that
	// ns = (cycles*mult) >> shift

	// Compute the shift factor which is limiting the conversion range
	uint64_t temp_mult = ((uint64_t)maxSec * from) >> 32;
	uint32_t shift_accuracy = 32;
	while (temp_mult > 0) {
		temp_mult >>= 1;
		shift_accuracy--;
	}

	// Find the conversion shift/mult pair which has the best accuracy
	// while fitting the maxSec conversion range
	uint32_t temp_shift;
	for (temp_shift=32 ; temp_shift>0; temp_shift--) {
		temp_mult = (uint64_t) to << temp_shift;
		temp_mult += from / 2;
		temp_mult /= from;
		if ((temp_mult >> shift_accuracy) == 0)
			break;
	}

	*mult = temp_mult;
	*shift = temp_shift;
}

void sleep(unsigned long sec){
	m_eventTimer.sleep(sec);
}

void msleep(unsigned long ms){
	m_eventTimer.msleep(ms);
}

void usleep(unsigned long us){
	m_eventTimer.usleep(us);
}

void nsleep(unsigned long ns){
	m_eventTimer.nsleep(ns);
}

void mdelay(unsigned long ms){
	uint64_t n_ticks = (m_steadyTimer.frequency * ms) / 1000;

	uint64_t t0 = m_steadyTimer.read();
	while (((m_steadyTimer.read() - t0) & m_steadyTimer.mask) < n_ticks){
		pause();
	}
}

void udelay(unsigned long us){
	uint64_t n_ticks = (m_steadyTimer.frequency * us + 999999) / 1000000;

	uint64_t t0 = m_steadyTimer.read();
	while (((m_steadyTimer.read() - t0) & m_steadyTimer.mask) < n_ticks){
		pause();
	}
}

void ndelay(unsigned long ns){
	uint64_t n_ticks = (m_steadyTimer.frequency * ns + 999999999) / 1000000000;

	uint64_t t0 = m_steadyTimer.read();
	while (((m_steadyTimer.read() - t0) & m_steadyTimer.mask) < n_ticks){
		pause();
	}
}
