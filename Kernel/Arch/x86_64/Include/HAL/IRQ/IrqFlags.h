#ifndef __IRQ_FLAGS_H__
#define __IRQ_FLAGS_H__

#include "mugOS/Preprocessor.h"
#include "Arch/x86_64/CPU/Registers.h"

#define ISA_IRQ_OFFSET		0x20

// Standard ISA IRQs
#define IRQ_PIT				0x00 + ISA_IRQ_OFFSET // Programmable Interval Timer
#define IRQ_PS2_KEYBOARD	0x01 + ISA_IRQ_OFFSET
#define IRQ_COM2			0x03 + ISA_IRQ_OFFSET // Serial
#define IRQ_COM1			0x04 + ISA_IRQ_OFFSET // Serial
#define IRQ_LPT1			0x07 + ISA_IRQ_OFFSET // LPT1 or PIC spurious IRQ
#define IRQ_PS2_MOUSE		0x0c + ISA_IRQ_OFFSET
#define IRQ_ATA2			0x0f + ISA_IRQ_OFFSET // Secondary ATA hard disk or spurious IRQ

// Programmed IRQs (we can choose those)
#define IRQ_APIC_TIMER		0x30
#define IRQ_APIC_SPURIOUS	0xff

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
