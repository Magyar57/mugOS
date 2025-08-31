#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// i8259.h: Intel 8259 PIC Driver

/// @brief Initialize/configure (remap) the PIC
void i8259_init();

/// @brief Enables (unmasks) a given IRQ
void i8259_enableSpecific(int irq);

/// @brief Disables (masks) a given IRQ
void i8259_disableSpecific(int irq);

/// @brief Enables (unmasks) all IRQ. This disables the PIC
void i8259_enableAllIRQ();

/// @brief Disables (masks) all IRQ
void i8259_disableAllIRQ();

/// @brief Send EOI (end of interrupt) to the PIC
void i8259_sendEIO(int irq);

#endif
