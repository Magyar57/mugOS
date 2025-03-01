#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include <stdbool.h>
#include "HAL/IRQFlags.h"

// IRQHandler function type
typedef void (*IRQHandler)(void* registers);

/// @brief Initialise the IRQs & the IRQ chip
void IRQ_initialize();

/// @brief Register a new IRQ handler (if set, replace the current handler)
void IRQ_registerHandler(uint8_t irq, IRQHandler handler);

/// @brief Remove a set IRQ handler
void IRQ_deregisterHandler(uint8_t irq);

#endif
