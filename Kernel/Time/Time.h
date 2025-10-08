#ifndef __TIME_H__
#define __TIME_H__

/// @brief Initialize the Time subsystem
void Time_init();

/// @brief Compute mult/shift operators for converting frequencies.
/// These operators are optimizations to convert timer cycles/ticks to time intervals:
/// `ns = (cycles * mult) >> shift`
/// @param mult Output for the computed `mult` factor
/// @param shift Output for the computed `shift` power
/// @param from Frequency (Hz) to convert from.
//         For a SteadyTimer, it is its ticking frequency in Hz.
/// @param to Frequency (Hz) to convert to.
///        For a SteadyTimer, it is `1000000000` (1GHz, 1 billion nanoseconds))
/// @param maxSec Minimum range to be supported by the conversion (without overflowing).
///        A greater value means greater conversion range, but lower conversion accuracy
void Time_computeConversion(uint32_t* mult, uint32_t* shift, uint32_t from, uint32_t to, uint32_t maxSec);

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
