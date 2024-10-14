#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// Configure (remap) the PIC. Offsets must be divisable by 8 !!
void x86_PIC_Remap(uint8_t offsetMasterPIC, uint8_t offsetSlavePIC);

// Disables (masks) a given IRQ
void x86_PIC_DisableIRQ(uint8_t irq);
// Enables (unmasks) a given IRQ
void x86_PIC_EnableIRQ(uint8_t irq);

// Disables (masks) all IRQ
void x86_PIC_DisableAllIRQ();
// Enables (unmasks) all IRQ
void x86_PIC_EnableAllIRQ();

// Disable the PIC (by masking every single interrupt)
void x86_PIC_Disable();

// Send EOI (end of interrupt) to the PIC
void x86_PIC_SendEIO(int irq);

// Returns the combined IRR registers values:
// [ bit 15-8 Slave's IRR - bit 8-0 Master's IRR ]
uint16_t x86_PIC_GetCombinedIRR();
// Returns the combined ISR registers values: 
// [ bit 15-8 Slave's ISR - bit 8-0 Master's ISR ]
uint16_t x86_PIC_GetCombinedISR(void);

#endif
