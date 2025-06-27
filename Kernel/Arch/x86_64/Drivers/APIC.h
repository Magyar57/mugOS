#ifndef __APIC_H__
#define __APIC_H__

// APIC.h: Advanced Programmable Interrupt Controller driver
// Includes CPU-local APIC as well as global(s) I/O APIC

void APIC_init();

// Send EOI (end of interrupt) to the local APIC
void APIC_sendEIO(int irq);

#endif
