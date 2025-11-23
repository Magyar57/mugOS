#ifndef __TSC_H__
#define __TSC_H__

#include <stdint.h>

// TSC.h: Time Stamp Counter driver

void TSC_init();

/// @brief Read the TSC current counter
/// @return The value of the TSC counter (on the current CPU)
uint64_t TSC_read();

/// @brief Return the calibrated frequency of the TSC
uint64_t TSC_getFrequency();

#endif
