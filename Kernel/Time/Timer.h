#ifndef __TIMER_H__
#define __TIMER_H__

// Timer.h: defines the `struct Timer` abstraction for plateform timers

struct Timer {
	// Sleep family of functions, relaxes CPU, IRQ unsafe
	void (*sleep)(unsigned long sec);
	void (*msleep)(unsigned long ms);
	void (*usleep)(unsigned long us);
	void (*nsleep)(unsigned long ns);

	// Delay family of functions, busy-wait, IRQ safe
	void (*mdelay)(unsigned long ms);
	void (*udelay)(unsigned long us);
	void (*ndelay)(unsigned long ns);
};

#endif
