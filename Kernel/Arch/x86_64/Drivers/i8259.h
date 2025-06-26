#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// i8259.h: Intel 8259 PIC Driver

// Initialize/configure (remap) the PIC. Offsets must be divisable by 8 !!
void i8259_remap(uint8_t offsetMasterPIC, uint8_t offsetSlavePIC);

// Disables (masks) a given IRQ
void i8259_disableIRQ(int irq);

// Enables (unmasks) a given IRQ
void i8259_enableIRQ(int irq);

// Disables (masks) all IRQ
void i8259_disableAllIRQ();

// Enables (unmasks) all IRQ. This disables the PIC
void i8259_enableAllIRQ();

// Send EOI (end of interrupt) to the PIC
void i8259_sendEIO(int irq);

// Handle a spurious IRQ. Returns whether the IRQ was spurious or not
bool i8259_handleSpuriousIRQ(int irq);

#endif
