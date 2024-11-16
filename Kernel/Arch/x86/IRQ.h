#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include "ISR.h"

typedef ISR IRQHandler;

void IRQ_initialize();
void IRQ_registerHandler(uint8_t irq, IRQHandler handler);
void IRQ_deregisterHandler(uint8_t irq);

#endif
