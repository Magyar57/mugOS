#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>

// x86_64 IRQ Chip driver
// Can be either the PIC or the APIC

struct IRQChip {
	void (*init)(void);
	void (*sendEOI)(int irq);
};

// Get the plateform's (best) IRQ chip
struct IRQChip* IRQChip_get();

#endif
