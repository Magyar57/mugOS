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
	lnode_t node;
	// Read the clock's counter
	uint64_t (*read)();
	// Mask for the value returned by the clock
	uint64_t mask;
	// The frequency at which the ticks increments, in Hz
	uint64_t frequency;
	// The mult and shift conversion operators, so that `(steadyTimer.read() * mult) >> shift`
	// yields a time interval in nanoseconds. They must be set by the timer's driver,
	// eventually computed with `Time_computeConversion`.
	uint32_t mult, shift;
};

// A timer that is capable of generating events (IRQs).
// Used for sleep functions & scheduling
struct EventTimer {
	const char* name;
	int score;
	lnode_t node;
	// Sleep family of functions, relaxes CPU, IRQ unsafe
	// Note that this interface will have to change when we want to support multiple
	// sleep at a time, as well as scheduling
	void (*sleep)(unsigned long sec);
	void (*msleep)(unsigned long ms);
	void (*usleep)(unsigned long us);
	void (*nsleep)(unsigned long ns);
};

#endif
