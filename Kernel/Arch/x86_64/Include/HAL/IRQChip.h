#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>
#include "Preprocessor.h"
#include "HAL/CPU.h"
#include "IRQ.h"

// x86_64 IRQ Chip driver
// Can be either the PIC or the APIC (not implemented yet)

// Initialize the plateform's IRQs ; important: call ISR_initialize beforehand !
void IRQChip_initialize();

void IRQChip_registerHandler(uint8_t irq, IRQHandler handler);

void IRQChip_deregisterHandler(uint8_t irq);

#endif
