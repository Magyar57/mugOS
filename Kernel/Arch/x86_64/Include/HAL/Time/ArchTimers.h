#ifndef __ARCH_TIMERS__
#define __ARCH_TIMERS__

#include "Time/Timers.h"

// ArchTimers.h: Initialize EventTimer and SteadyTimer for the Time subsystem
// with the best (available and supported) architecture-specific timers

/// @brief Initialize the system's EventTimer
/// @param timer EventTimer to initialize
void ArchTimers_initEventTimer(struct EventTimer* timer);

/// @brief Initialize the system's SteadyTimer
/// @param timer SteadyTimer to initialize
void ArchTimers_initSteadyTimer(struct SteadyTimer* timer);

#endif
