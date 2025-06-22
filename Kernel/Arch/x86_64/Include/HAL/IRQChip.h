#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>
#include "HAL/CPU.h"
#include "IRQ.h"

// x86_64 IRQ Chip driver
// Can be either the PIC or the APIC (not implemented yet)

// Initialize the plateform's IRQs
void IRQChip_init();

void IRQChip_registerHandler(int irq, IRQHandler handler);
void IRQChip_deregisterHandler(int irq);

void IRQChip_enableSpecific(int irq);
void IRQChip_disableSpecific(int irq);

#endif
