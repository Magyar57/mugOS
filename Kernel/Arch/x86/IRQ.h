#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include "ISR.h"

typedef ISR IRQHandler;

void x86_IRQ_Initialize();
void x86_IRQ_RegisterIRQHandler(uint8_t irq, IRQHandler handler);
void x86_IRQ_DeregisterIRQHandler(uint8_t irq);

#endif
