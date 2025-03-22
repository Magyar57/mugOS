#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// i8259.h: Intel 8259 PIC Driver

// Initialize/configure (remap) the PIC. Offsets must be divisable by 8 !!
void i8259_remap(uint8_t offsetMasterPIC, uint8_t offsetSlavePIC);

// Disables (masks) a given IRQ
void i8259_disableIRQ(uint8_t irq);
// Enables (unmasks) a given IRQ
void i8259_enableIRQ(uint8_t irq);

// Disables (masks) all IRQ
void i8259_disableAllIRQ();
// Enables (unmasks) all IRQ
void i8259_enableAllIRQ();

// Disable the PIC (by masking every single interrupt)
void i8259_disable();

// Send EOI (end of interrupt) to the PIC
void i8259_sendEIO(int irq);

// Returns the combined IRR (Interrupt Request Register) values:
// [ bit 15-8 Slave's IRR - bit 8-0 Master's IRR ]
uint16_t i8259_getCombinedIRR();

// Returns the combined ISR (In-Service Register) values:
// [ bit 15-8 Slave's ISR - bit 8-0 Master's ISR ]
uint16_t i8259_getCombinedISR();

#endif
