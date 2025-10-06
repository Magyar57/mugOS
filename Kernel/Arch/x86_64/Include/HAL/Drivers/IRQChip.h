#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>
#include "IRQ.h"

// x86_64 IRQ Chip driver
// Can be either the PIC or the APIC

struct IRQChip {
	void (*init)();
	void (*sendEOI)(int irq);
	void (*enableSpecific)(int irq);
	void (*disableSpecific)(int irq);
	void (*enableAll)();
	void (*disableAll)();
	void (*installPrehandler)(irqhandler_t prehandler);
};

// Get the plateform's (best) IRQ chip
struct IRQChip* IRQChip_get();

int IRQChip_getIRQ(void* params);

#endif
