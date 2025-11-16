#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <stdint.h>
#include "mugOS/List.h"

// Timers.h: defines the timers abstractions for architectural timers

// A timer that is capable of keeping track of time (has a readable, monotonic,
// ever-increasing counter). Used for various timekeeping & busy-wait delays
struct SteadyTimer {
	const char* name;
	int score;
	// Read the clock's counter
	uint64_t (*read)();
	// Mask for the value returned by the clock
	uint64_t mask;
	// The frequency at which the timer's ticks increments, in Hz
	uint64_t frequency;

	lnode_t node;
	// The mult and shift conversion operators, so that `(steadyTimer.read() * mult) >> shift`
	// yields a time interval in nanoseconds. Note that there are handy wrapper functions
	// to be used for conversions instead of doing it manually
	uint32_t mult, shift;
};

// A timer that is capable of generating events (IRQs).
// Used for sleep functions & scheduling
struct EventTimer {
	const char* name;
	int score;
	// The frequency at which the timer's ticks increments, in Hz
	uint64_t frequency;
	// Schedule an IRQ to fire in `tick` timer ticks
	void (*scheduleEvent)(unsigned long tick);
	// The min/max input that the scheduleEvent function supports
	unsigned long minTick, maxTick;

	// Handler that the timer MUST call when the event (aka IRQ) fires
	void (*eventHandler)();

	lnode_t node;
	// The mult and shift conversion operators, so that `(time_in_nanoseconds * mult) >> shift`
	// yields a number of ticks of the clock
	uint32_t mult, shift;
};

#endif
