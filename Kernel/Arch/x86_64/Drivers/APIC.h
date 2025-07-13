#ifndef __APIC_H__
#define __APIC_H__

#include "Memory/Memory.h"

// APIC.h: Advanced Programmable Interrupt Controller driver
// Includes CPU-local APIC as well as global(s) I/O APIC

/// @brief Initialize the CPU's local APIC
void APIC_init();

/// @brief Send EOI (end of interrupt) to the local APIC
void APIC_sendEIO(int irq);

void APIC_wakeCPU(int lapicID, physical_address_t entry);

#endif
