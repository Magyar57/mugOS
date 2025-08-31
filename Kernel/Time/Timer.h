#ifndef __TIMER_H__
#define __TIMER_H__

// Timer.h: defines the `struct Timer` abstraction for plateform timers

struct Timer {
	void (*init)();
	void (*sleep)(unsigned long sec);
	void (*msleep)(unsigned long ms);
	void (*usleep)(unsigned long us);
	void (*nsleep)(unsigned long ns);
};

#endif
