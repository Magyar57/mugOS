#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include "HAL/IRQFlags.h"

// IRQHandler function type
typedef void (*IRQHandler)(void* registers);

/// @brief Initialise the IRQs & the IRQ chip
void IRQ_initialize();

/// @brief Enable a specific IRQ
void IRQ_enableSpecific(int irq);

/// @brief Disable a specific IRQ
void IRQ_disableSpecific(int irq);

/// @brief Register a new IRQ handler (if set, replace the current handler)
void IRQ_registerHandler(int irq, IRQHandler handler);

/// @brief Remove a set IRQ handler
void IRQ_deregisterHandler(int irq);

#endif
