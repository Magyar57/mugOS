#include <stddef.h>
#include "Logging.h"
#include "Panic.h"
#include "Time/Timers.h"
#include "HAL/Halt.h"
#include "HAL/Time/ArchTimers.h"

#include "Time.h"
#define MODULE "Time"

static list_t m_steadyTimers = LIST_STATIC_INIT(m_steadyTimers);
static list_t m_eventTimers = LIST_STATIC_INIT(m_eventTimers);

static struct SteadyTimer* m_steadyTimer = NULL;
static struct EventTimer* m_eventTimer = NULL;

static void delayTicks(unsigned long ticks){
	uint64_t t0 = m_steadyTimer->read();

	while (((m_steadyTimer->read() - t0) & m_steadyTimer->mask) < ticks){
		pause();
	}
}

/// @brief Compute mult/shift operators for converting frequencies.
/// These operators are optimizations to convert timer cycles/ticks to time intervals:
/// `ns = (cycles * mult) >> shift`
/// @param mult Output for the computed `mult` factor
/// @param shift Output for the computed `shift` power
/// @param from Frequency (Hz) to convert from.
///        For a SteadyTimer, it is its ticking frequency in Hz.
/// @param to Frequency (Hz) to convert to.
///        For a SteadyTimer, it is `1000000000` (1GHz, 1 billion nanoseconds))
/// @param maxSec Minimum range to be supported by the conversion (without overflowing).
///        A greater value means greater conversion range, but lower conversion accuracy
static void computeConversion(uint32_t* mult, uint32_t* shift, uint32_t from, uint32_t to, uint32_t maxSec){
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

// ================ Public API ================

void Time_init(){
	ArchTimers_init();

	if (List_isEmpty(&m_steadyTimers)){
		log(PANIC, MODULE, "No SteadyTimer registered !!");
		panic();
	}

	if (List_isEmpty(&m_eventTimers)){
		log(PANIC, MODULE, "No EventTimer registered !!");
		panic();
	}

	log(SUCCESS, MODULE, "Initialized with %s steady timer & %s event timer",
		m_steadyTimer->name, m_eventTimer->name);
}

void Time_registerSteadyTimer(struct SteadyTimer* timer){
	List_pushBack(&m_steadyTimers, &timer->node);

	computeConversion(&timer->mult, &timer->shift, timer->frequency, 1000000000, 3600);

	if (m_steadyTimer == NULL || m_steadyTimer->score < timer->score)
		m_steadyTimer = timer;
}

void Time_registerEventTimer(struct EventTimer* timer){
	List_pushBack(&m_eventTimers, &timer->node);

	if (m_eventTimer == NULL || m_eventTimer->score < timer->score)
		m_eventTimer = timer;
}

void sleep(unsigned long sec){
	m_eventTimer->sleep(sec);
}

void msleep(unsigned long ms){
	m_eventTimer->msleep(ms);
}

void usleep(unsigned long us){
	m_eventTimer->usleep(us);
}

void nsleep(unsigned long ns){
	m_eventTimer->nsleep(ns);
}

void mdelay(unsigned long ms){
	uint64_t n_ticks = (m_steadyTimer->frequency * ms + 999) / 1000;
	delayTicks(n_ticks);
}

void udelay(unsigned long us){
	uint64_t n_ticks = (m_steadyTimer->frequency * us + 999999) / 1000000;
	delayTicks(n_ticks);
}

void ndelay(unsigned long ns){
	uint64_t n_ticks = (m_steadyTimer->frequency * ns + 999999999) / 1000000000;
	delayTicks(n_ticks);
}
