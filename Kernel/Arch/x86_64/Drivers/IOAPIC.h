#ifndef __IOAPIC_H__
#define __IOAPIC_H__

#include <stdint.h>

void IOAPIC_init();
void IOAPIC_enableSpecific(int irq);
void IOAPIC_disableSpecific(int irq);

#endif
