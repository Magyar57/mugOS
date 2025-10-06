#ifndef __PIT_H__
#define __PIT_H__

#include "Time/Timers.h"

/// @brief Initialize the PIT
void PIT_init(struct EventTimer* timer);

/// @brief Sleep `sec` seconds (IRQ unsafe)
void PIT_sleep(unsigned long sec);

/// @brief Sleep `ms` milliseconds (IRQ unsafe)
void PIT_msleep(unsigned long ms);

/// @brief Sleep `ms` milliseconds (IRQ unsafe)
void PIT_usleep(unsigned long us);

/// @brief Sleep `ms` milliseconds (IRQ unsafe)
void PIT_nsleep(unsigned long ns);

#endif
