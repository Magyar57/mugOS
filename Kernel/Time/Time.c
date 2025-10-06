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
