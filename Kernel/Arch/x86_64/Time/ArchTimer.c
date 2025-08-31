#include "Drivers/Timers/PIT.h"
#include "Time/Timer.h"

#include "HAL/Time/ArchTimer.h"

static struct Timer m_timer;

struct Timer* ArchTimer_get(){
	// Only PIT is supported for now
	m_timer.init = PIT_init;
	m_timer.sleep = PIT_sleep;
	m_timer.msleep = PIT_msleep;
	m_timer.usleep = PIT_usleep;
	m_timer.nsleep = PIT_nsleep;

	return &m_timer;
}
