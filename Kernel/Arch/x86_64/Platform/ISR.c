#include <stdint.h>
#include <stddef.h>
#include "assert.h"
#include "Panic.h"
#include "Logging.h"
#include "IDT.h"

#include "Platform/ISR.h"
#define MODULE "ISR"

#define ISR_DIVIDE_BY_ZERO_ERROR	0x00
#define ISR_DOUBLE_FAULT			0x08
#define ISR_SEGMENT_NOT_PRESENT		0x0b
#define ISR_PAGE_FAULT				0x0e

// Global array of [un]registered exception/trap handlers
// It is handled (edited) in ISR.c, and handlers are called in ISR.asm
isr_t m_handlers[256];

static const char* const EXCEPTION_TYPES[] = {
    "Exception (Fault) - Divide by zero error",
    "Exception (Fault/Trap) - Debug exception",
    "Non-Maskable Interrupt (NMI)",
    "Exception (Trap) - Breakpoint",
    "Exception (Trap) - Overflow",
    "Exception (Fault) - Bound range exceeded",
    "Exception (Fault) - Invalid Opcode",
    "Exception (Fault) - Device not available (no math coprocessor)",
    "Exception (Abort) - Double fault",
    "Exception (Fault) - Coprocessor segment overrun",
    "Exception (Fault) - Invalid TSS",
    "Exception (Fault) - Segment not present",
    "Exception (Fault) - Stack-segment fault",
    "Exception (Fault) - General protection fault",
    "Exception (Fault) - Page fault",
    "RESERVED DO NOT USE",
    "Exception (Fault) - x87 floating-point exception",
    "Exception (Fault) - Alignment check",
    "Exception (Fault) - Machine check",
    "Exception (Fault) - SIMD floating-point exception",
    "Exception (Fault) - Virtualization exception",
    "Exception (Fault) - Control protection exception ",
    "RESERVED DO NOT USE",
    "RESERVED DO NOT USE",
    "RESERVED DO NOT USE",
    "RESERVED DO NOT USE",
    "RESERVED DO NOT USE",
    "RESERVED DO NOT USE",
    "RESERVED DO NOT USE", // "Hypervisor injection exception" ?
    "RESERVED DO NOT USE", // "VMM communication exception" ?
    "RESERVED DO NOT USE", // "Security exception" ?
	"RESERVED DO NOT USE"
	// After: only interrupts
};

static inline void dumpRegisters(int logLevel, struct ISR_Params* params){
	log(logLevel, NULL, "vector=%#.2lx rflags=%#.16lx err=%#lx",
		params->vector, params->rflags, params->err);
	log(logLevel, NULL, "\trip=%#.16lx rsp=%#.16lx rbp=%#.16lx",
		params->rip, params->rsp, params->rbp);
	log(logLevel, NULL, "\trax=%#.16lx rbx=%#.16lx rcx=%#.16lx rdx=%#.16lx",
		params->rax, params->rbx, params->rcx, params->rdx);
	log(logLevel, NULL, "\trsi=%#.16lx rdi=%#.16lx", params->rsi, params->rdi);
	log(logLevel, NULL, "\t r8=%#.16lx  r9=%#.16lx r10=%#.16lx r11=%#.16lx",
		params->r8, params->r9, params->r10, params->r11);
	log(logLevel, NULL, "\tr12=%#.16lx r13=%#.16lx r14=%#.16lx r15=%#.16lx",
		params->r12, params->r13, params->r14, params->r15);
	log(logLevel, NULL, "\tcs=%#.2lx ds=%#.2lx ss=%#.2lx",
		params->cs, params->ds, params->ss);
}

// ================ ISR Handlers ================

// This function is ran by the assembly stub when there is no handler to call
void ISR_noHandler(struct ISR_Params* params){
	if (params->vector >= 32){
		log(WARNING, MODULE, "Unhandled IRQ %lu !", params->vector);
		return;
	}

	log(PANIC, MODULE, "Unhandled %s !", EXCEPTION_TYPES[params->vector]);
	dumpRegisters(PANIC, params);
	panic();
}

void ISR_divisionByZeroError(struct ISR_Params*){
	log(PANIC, NULL, "Division by zero error !!");
	panic();
}

void ISR_doubleFault(struct ISR_Params* params){
	log(PANIC, NULL, "Double fault !!");
	dumpRegisters(PANIC, params);
	panic();
}

void ISR_segmentNotPresent(struct ISR_Params* params){
	// https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-220.html

	const char* origin = (params->err & 0x0001) ? "External" : "Internal";
	uint8_t table = (params->err & 0x0006) >> 1;
	uint16_t descriptor = (params->err & (0xfffc)) >> 3 ;

	const char* tableName;
	switch (table){
	case 0:
		tableName = "GDT"; break;
	case 1:
		tableName = "IDT"; break;
	case 2:
		tableName = "LDT"; break;
	case 3:
		tableName = "IDT"; break;
	default:
		tableName = "None"; // unreachable
	}

	log(PANIC, MODULE, "%s", EXCEPTION_TYPES[params->vector]);
	log(PANIC, MODULE, "%s Exception in %s at descriptor=%#hx", origin, tableName, descriptor);
	if (!IDT_isInterruptHandlerEnabled(descriptor))
		log(PANIC, MODULE, "This was triggered because the IDT entry is disabled");
	panic();
}

void ISR_pageFault(struct ISR_Params* params){
	uint64_t cr2;
	__asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

	const char* cause = params->err & 0b00000001 ? "page-protection violation" : "non-present page";
	const char* type = params->err & 0b00000010 ? "write access" : "read access";
	const char* cpu_mode = params->err & 0b00000100 ? "user" : "kernel";
	bool reserved = (params->err & 0b00001000);
	bool instruction_fetch = (params->err & 0b00010000);
	bool protection_key_violation = (params->err & 0b00100000);
	bool shadow_stack = (params->err & 0b01000000);
	bool sgx_violation = (params->err & 0b10000000);

	log(PANIC, MODULE, "Page fault (error=%#lx) ! Access to %#lx from code at %#lx",
		params->err, cr2, params->rip);
	log(PANIC, MODULE, "Cause: %s, from a %s, with CPU in %s mode",
		cause, type, cpu_mode);

	if (reserved) log(PANIC, MODULE, "Reserved bits are set in some entries that were accessed !");
	if (instruction_fetch) log(PANIC, MODULE, "Source: instruction fetch");
	if (protection_key_violation) log(PANIC, MODULE, "Source: protection key violation");
	if (shadow_stack) log(PANIC, MODULE, "Source: shadow stack access");
	if (sgx_violation) log(PANIC, MODULE, "Source: SGX violation");
	panic();
}

// ================ Initialize ================

// (ISR_defs.c) Sets all assembly entry points in the IDT
void ISR_initInterruptHandlers();

void ISR_init(){
	ISR_initInterruptHandlers();

	// Initialize handlers
	for (int i=0 ; i<256 ; i++){
		m_handlers[i] = NULL;
		IDT_enableInterruptHandler(i);
	}

	// Register our exception handlers
	ISR_installHandler(ISR_DIVIDE_BY_ZERO_ERROR, ISR_divisionByZeroError);
	ISR_installHandler(ISR_DOUBLE_FAULT, ISR_doubleFault);
	ISR_installHandler(ISR_SEGMENT_NOT_PRESENT, ISR_segmentNotPresent);
	ISR_installHandler(ISR_PAGE_FAULT, ISR_pageFault);
}

void ISR_installHandler(uint8_t vector, isr_t handler){
	m_handlers[vector] = handler;
}

void ISR_removeHandler(uint8_t vector){
	m_handlers[vector] = NULL;
}

bool ISR_isHandlerPresent(uint8_t vector){
	return (m_handlers[vector] != NULL);
}
