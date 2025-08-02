#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// i8259.h: Intel 8259 PIC Driver

// Initialize/configure (remap) the PIC
void i8259_init();

// Enables (unmasks) a given IRQ
void i8259_enableSpecific(int irq);

// Disables (masks) a given IRQ
void i8259_disableSpecific(int irq);

// Disables (masks) all IRQ
void i8259_disableAllIRQ();

// Enables (unmasks) all IRQ. This disables the PIC
void i8259_enableAllIRQ();

// Send EOI (end of interrupt) to the PIC
void i8259_sendEIO(int irq);

#endif
