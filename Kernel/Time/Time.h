#ifndef __TIME_H__
#define __TIME_H__

/// @brief Initialize the Time subsystem
void Time_init();

/// @brief Sleep for `sec` seconds (IRQ unsafe)
void sleep(unsigned long sec);

/// @brief Sleep for `ms` milliseconds (IRQ unsafe)
void msleep(unsigned long ms);

/// @brief Sleep for `us` microseconds (IRQ unsafe)
void usleep(unsigned long us);

/// @brief Sleep for `ns` nanoseconds (IRQ unsafe)
void nsleep(unsigned long ns);

/// @brief Delay execution for `ms` milliseconds (IRQ safe, busy-wait)
void mdelay(unsigned long ms);

/// @brief Delay execution for `us` microseconds (IRQ safe, busy-wait)
void udelay(unsigned long us);

/// @brief Delay execution for `ns` nanoseconds (IRQ safe, busy-wait)
void ndelay(unsigned long ns);

#endif
