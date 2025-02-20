#ifndef __IRQ_H__
#define __IRQ_H__

#include <stdint.h>
#include "Preprocessor.h"
#include "CPU.h"
#include "ISR.h"

typedef ISR IRQHandler;

// Initialise the IRQ handlers in the ISR table (important: call ISR_initialize beforehand !)
void IRQ_initialize();

// Register a new IRQ handler (if set, replace the current handler)
void IRQ_registerHandler(uint8_t irq, IRQHandler handler);

// Remove a set IRQ handler
void IRQ_deregisterHandler(uint8_t irq);

static always_inline bool areIRQSet(unsigned long flags){
	return (flags & X86_64_RFLAGS_IF);
}

#define IRQ_disable() CPU_IRQDisable()

#define IRQ_enable() CPU_IRQEnable()

// Saves and disables the IRQ (for the current CPU). Call IRQ_restore to undo
#define IRQ_disableSave(flags) \
	do { \
		typecheck(unsigned long, flags); \
		flags = getFlags(); \
		CPU_IRQDisable(); \
	} while (0)

#define IRQ_restore(flags) \
	do { \
		typecheck(unsigned long, flags); \
		if (areIRQSet(flags)) \
			CPU_IRQEnable(); \
	} while (0)

#endif
