#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include "ISR.h"

typedef ISR IRQHandler;

// Initialise the IRQ handlers in the ISR table (important: call ISR_initialize beforehand !)
void IRQ_initialize();

// Register a new IRQ handler (if set, replace the current handler)
void IRQ_registerHandler(uint8_t irq, IRQHandler handler);

// Remove a set IRQ handler
void IRQ_deregisterHandler(uint8_t irq);

#endif
