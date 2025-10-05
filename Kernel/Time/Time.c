#include <stddef.h>
#include "Logging.h"
#include "Panic.h"
#include "Time/Timer.h"
#include "HAL/Time/ArchTimer.h"

#include "Time.h"
#define MODULE "Time"

static struct Timer m_timer;

void Time_init(){
	ArchTimer_init(&m_timer);
}

void sleep(unsigned long sec){
	m_timer.sleep(sec);
}

void msleep(unsigned long ms){
	m_timer.msleep(ms);
}

void usleep(unsigned long us){
	m_timer.usleep(us);
}

void nsleep(unsigned long ns){
	m_timer.nsleep(ns);
}

void mdelay(unsigned long ms){
	m_timer.mdelay(ms);
}

void udelay(unsigned long us){
	m_timer.udelay(us);
}

void ndelay(unsigned long ns){
	m_timer.ndelay(ns);
}
