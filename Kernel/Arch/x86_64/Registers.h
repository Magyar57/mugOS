#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stdint.h>
#include "Memory/Memory.h"

// Registers.h: CPU registers bitfields and flags (includes MSR)
// Note: assembly functions are regrouped in CPU.asm

// EFLAGS/RFLAGS register bits
// S: status flag, C: control flag, X: system flag
#define X86_64_RFLAGS_CF	(1<<0)			// (S) Carry flag
#define X86_64_RFLAGS_PF	(1<<2)			// (S) Parity flag
#define X86_64_RFLAGS_AF	(1<<4)			// (S) Auxiliary carry flag
#define X86_64_RFLAGS_ZF	(1<<6)			// (S) Zero flag
#define X86_64_RFLAGS_SF	(1<<7)			// (S) Sign flag
#define X86_64_RFLAGS_TF	(1<<8)			// (X) Trap flag
#define X86_64_RFLAGS_IF	(1<<9)			// (X) Interrupt enable flag
#define X86_64_RFLAGS_DF	(1<<10)			// (C) Direction flag
#define X86_64_RFLAGS_OF	(1<<11)			// (S) Overflow flag
#define X86_64_RFLAGS_IOPL	(1<<12|1<<13)	// (X) I/O privilege level
#define X86_64_RFLAGS_NT	(1<<14)			// (X) Nested task
#define X86_64_RFLAGS_RF	(1<<16)			// (X) Resume flag
#define X86_64_RFLAGS_VM	(1<<17)			// (X) Virtual-8086 mode
#define X86_64_RFLAGS_AC	(1<<18)			// (X) Alignment check / access control
#define X86_64_RFLAGS_VIF	(1<<19)			// (X) Virtual interrupt flag
#define X86_64_RFLAGS_VIP	(1<<20)			// (X) Virtual interrupt pending
#define X86_64_RFLAGS_ID	(1<<21)			// (X) ID flag

// MSR addresses
#define MSR_ADDR_IA32_MISC_ENABLE	0x000001a0
#define MSR_ADDR_IA32_APIC_BASE		0x0000001b
#define MSR_ADDR_IA32_EFER			0xc0000080

/// @brief Reads and returns the value of the `msr` Model-Specific Register
/// @warning Does NOT check whether the MSR and the `rdmsr` instruction are supported
uint64_t Registers_readMSR(int msr);

/// @brief Writes the value `value` into the `msr` Model-Specific Register
/// @warning Does NOT check whether the MSR and the `wrmsr` instruction are supported
void Registers_writeMSR(int msr, uint64_t value);

uint64_t Registers_readCR0();
uint64_t Registers_readCR4();
void Registers_writeCR0(uint64_t val);
void Registers_writeCR4(uint64_t val);

// ================ Registers bitfields ================

union CR0 {
	uint64_t value;
	struct {
		uint64_t PE : 1; // Protection Enable (protected mode)
		uint64_t MP : 1; // Monitor Coprocessor
		uint64_t EM : 1; // Emulation (if clear, x87 FPU is present)
		uint64_t ST : 1; // Task Switched
		uint64_t ET : 1; // Extension Type
		uint64_t NE : 1; // Numeric Error
		uint64_t reserved_0 : 10;
		uint64_t WP : 1; // Write Protect
		uint64_t reserved_1 : 1;
		uint64_t AM : 1; // Alignement Mask
		uint64_t reserved_2 : 10;
		uint64_t NW : 1; // Not Write-through
		uint64_t CD : 1; // Cache Disable
		uint64_t PG : 1; // Paging
	} bits;
};

union CR4 {
	uint64_t value;
	struct {
		uint64_t VME : 1;
		uint64_t PVI : 1;
		uint64_t TSD : 1;
		uint64_t DE : 1;
		uint64_t PSE : 1;
		uint64_t PAE : 1;
		uint64_t MCE : 1;
		uint64_t PGE : 1;
		uint64_t PCE : 1;
		uint64_t OSFXSR : 1;
		uint64_t OSXMMEXCPT : 1;
		uint64_t UMIP : 1;
		uint64_t LA57 : 1;
		uint64_t VMXE : 1;
		uint64_t SMXE : 1;
		uint64_t reserved_0 : 1;
		uint64_t FSGSBASE : 1;
		uint64_t PCIDE : 1;
		uint64_t OSXSAVE : 1;
		uint64_t KL : 1;
		uint64_t SMEP : 1;
		uint64_t SMAP : 1;
		uint64_t PKE : 1;
		uint64_t CET : 1;
		uint64_t PKS : 1;
		uint64_t UINTR : 1;
		// rest is reserved
	} bits;
};

union MSR_IA32_MISC_ENABLE {
	uint64_t value;
	struct {
		uint64_t FSE : 1;				// Fast-Strings Enable (R/W?)
		uint64_t reserved_0 : 2;
		uint64_t ATCCE : 1;				// Automatic Thermal Control Circuit Enable (R/W)
		uint64_t reserved_1 : 3;
		uint64_t PMA : 1;				// Performance Monitoring Available (R)
		uint64_t reserved_2 : 3;
		// uint64_t FERRME : 1;			// FERR# Multiplexing Enable (R/W) => only for specific CPUs
		uint64_t BTSU : 1;				// Branch Trace Storage Unavailable (R/O)
		uint64_t PEBSU : 1;				// Processor Event Based Sampling Unavailable (R/O)
		uint64_t reserved_3 : 3;
		uint64_t EISTE : 1;				// Enhanced Intel SpeedStep Technology Enabled (R/W)
		uint64_t reserved_4 : 1;
		uint64_t MFSME : 1;				// Monitor FSM Enabled (R/W)
		uint64_t reserved_5 : 3;
		uint64_t LimitCPUIDMaxval : 1;	// (R/W)
		uint64_t xPTRMD : 1;			// xTPR Message Disable (R/W)
		uint64_t reserved_6 : 40;
	} bits;
};

union MSR_IA32_APIC_BASE {
	uint64_t value;
	struct {
		uint64_t reserved_0 : 8;
		uint64_t BSP : 1; // BootStrap Processor flag (R/W)
		uint64_t reserved_1 : 1;
		uint64_t x2APIC : 1; // Enable x2APIC mode (R/W)
		uint64_t GlobalEnable : 1; // APIC Global Enable (R/W)
		uint64_t base : ADDRESS_SIZE - 12; // APIC Base (R/W)
		uint64_t reserved_2 : 64 - ADDRESS_SIZE;
	} bits;
};

union MSR_IA32_EFER {
	uint64_t value;
	struct {
		uint64_t SCE : 1;			// SYSCALL Enable (R/W)
		uint64_t reserved_0 : 7;
		uint64_t LME : 1;			// IA-32e Mode Enable (R/W)
		uint64_t reserved_1 : 1;
		uint64_t LMA : 1;			// IA-32e Mode Active (R)
		uint64_t NXE : 1;			// Execute Disable bit Enable (R/W)
		uint64_t reserved_2 : 52;
	} bits;
};

#endif
