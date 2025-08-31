#include <stddef.h>
#include "Logging.h"
#include "Panic.h"
#include "Time/Timer.h"
#include "HAL/Time/ArchTimer.h"

#include "Time.h"
#define MODULE "Time"

struct Timer* m_timer;

void Time_init(){
	m_timer = ArchTimer_get();
	if (m_timer == NULL){
		log(PANIC, MODULE, "Architecture didn't provide any timer !!");
		panic();
	}

	m_timer->init();
}

void sleep(unsigned long sec){
	m_timer->sleep(sec);
}

void msleep(unsigned long ms){
	m_timer->msleep(ms);
}

void usleep(unsigned long us){
	m_timer->usleep(us);
}

void nsleep(unsigned long ns){
	m_timer->nsleep(ns);
}
