#ifndef __APIC_H__
#define __APIC_H__

#include "Memory/Memory.h"

// APIC.h: Advanced Programmable Interrupt Controller driver
// Includes CPU-local APIC as well as global(s) I/O APIC(s)

/// @brief Initialize the APIC subsystem: BSP's local APIC, and the global I/O APICs
void APIC_init();

/// @brief Initialize the current CPU's local APIC
/// @note It is NOT necessary to call it for the BSP's, this is done by `APIC_init`
void APIC_initLAPIC();

/// @brief Initialize all CPU's APIC timers
/// @note SMP must have been initialized beforehand
void APIC_initTimers();

/// @brief Send EOI (end of interrupt) to the local APIC
void APIC_sendEIO(int irq);

/// @brief Wake (starts) a local CPU, by sending an INIT IPI interrupt
/// @param lapicID The local APIC ID of the CPU to start
/// @param entry The entry point for the awoken CPU, as a (page-aligned) physical address
void APIC_wakeCPU(int lapicID, paddr_t entry);

#endif
