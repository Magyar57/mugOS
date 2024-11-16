#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// Configure (remap) the PIC. Offsets must be divisable by 8 !!
void PIC_remap(uint8_t offsetMasterPIC, uint8_t offsetSlavePIC);

// Disables (masks) a given IRQ
void PIC_disableIRQ(uint8_t irq);
// Enables (unmasks) a given IRQ
void PIC_enableIRQ(uint8_t irq);

// Disables (masks) all IRQ
void PIC_disableAllIRQ();
// Enables (unmasks) all IRQ
void PIC_enableAllIRQ();

// Disable the PIC (by masking every single interrupt)
void PIC_disable();

// Send EOI (end of interrupt) to the PIC
void PIC_sendEIO(int irq);

// Returns the combined IRR registers values:
// [ bit 15-8 Slave's IRR - bit 8-0 Master's IRR ]
uint16_t PIC_getCombinedIRR();
// Returns the combined ISR registers values: 
// [ bit 15-8 Slave's ISR - bit 8-0 Master's ISR ]
uint16_t PIC_getCombinedISR(void);

#endif
