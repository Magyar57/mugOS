#ifndef __IOAPIC_H__
#define __IOAPIC_H__

#include <stdint.h>

/// @brief Initialize all the I/O APICs on the system
void IOAPIC_init();

/// @brief Enable (unmask) a specific IRQ in the I/O APICs
void IOAPIC_enableSpecific(int irq);

/// @brief Disable (mask) a specific IRQ in the I/O APICs
void IOAPIC_disableSpecific(int irq);

/// @brief Enable all IRQs in all I/O APICs
void IOAPIC_enableAllIRQ();

/// @brief Disables all IRQs in all I/O APICs
void IOAPIC_disableAllIRQ();

#endif
