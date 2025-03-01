#ifndef __IRQ_FLAGS_H__
#define __IRQ_FLAGS_H__

#include "Preprocessor.h"
#include "HAL/CPU.h"

// Standard ISA IRQ numbers
#define IRQ_TIMER			0x00
#define IRQ_PS2_KEYBOARD	0x01
#define IRQ_COM2			0x03
#define IRQ_COM1			0x04
#define IRQ_PS2_MOUSE		0x0c

// Flags manipulations

/// @brief Disable interrupts (cli)
#define IRQ_disable() __asm__ volatile("cli")

/// @brief Enable interrupts (sti)
#define IRQ_enable() __asm__ volatile("sti")

/// @brief Returns whether interrupts are set (enabled) on the local CPU
always_inline bool IRQ_areIRQSet(unsigned long flags){
	return (flags & X86_64_RFLAGS_IF);
}

always_inline unsigned long IRQ_getFlags(){
	unsigned long flags;

	// "=rm": compiler can place flags_var in registers or memory
	__asm__ volatile("pushf ; pop %[flags_var]"
		: [flags_var] "=rm" (flags)
		:
		: "memory"
	);

	return flags;
}

// Saves and disables the IRQ (for the current CPU). Call IRQ_restore to undo
#define IRQ_disableSave(flags) \
	do { \
		typecheck(unsigned long, flags); \
		flags = IRQ_getFlags(); \
		IRQ_disable(); \
	} while (0)

#define IRQ_restore(flags) \
	do { \
		typecheck(unsigned long, flags); \
		if (IRQ_areIRQSet(flags)) \
			IRQ_enable(); \
	} while (0)

#endif
