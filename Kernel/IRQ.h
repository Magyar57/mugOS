#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include "HAL/IRQFlags.h"

// irqhandler_t function type
typedef void (*irqhandler_t)(void* registers);

/// @brief Initialise the IRQs & the IRQ chip
void IRQ_init();

/// @brief Enable a specific IRQ
void IRQ_enableSpecific(int irq);

/// @brief Disable a specific IRQ
void IRQ_disableSpecific(int irq);

/// @brief Register a new IRQ handler (if set, replace the current handler)
void IRQ_installHandler(int irq, irqhandler_t handler);

/// @brief Remove a set IRQ handler
void IRQ_removeHandler(int irq);

/// @brief Common IRQ prehandler
/// It can be run manually by IRQ chips when handling spurious interrupts
void IRQ_prehandler(void* params);

#endif
