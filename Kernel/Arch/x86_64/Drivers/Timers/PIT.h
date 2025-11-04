#ifndef __PIT_H__
#define __PIT_H__

/// @brief Initialize the PIT
void PIT_init();

/// @brief Sleep `sec` seconds (IRQ unsafe)
void PIT_sleep(unsigned long sec);

/// @brief Sleep `ms` milliseconds (IRQ unsafe)
void PIT_msleep(unsigned long ms);

/// @brief Sleep `ms` milliseconds (IRQ unsafe)
void PIT_usleep(unsigned long us);

/// @brief Sleep `ms` milliseconds (IRQ unsafe)
void PIT_nsleep(unsigned long ns);

#endif
