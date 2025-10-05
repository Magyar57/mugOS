#ifndef __PM_TIMER_H__
#define __PM_TIMER_H__

// PMTimer.h: Power Management Timer (ACPI timer) driver

/// @brief Intialize the system's PM Timer
/// @note Chip presence must be checked BEFORE calling this init
void PMTimer_init();

/// @return Whether a Power Management Timer is present on the system
bool PMTimer_isPresent();

#endif
