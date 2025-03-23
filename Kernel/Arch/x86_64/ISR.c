#include <stdint.h>
#include <stddef.h>
#include "Logging.h"
#include "HAL/CPU.h"
#include "GDT.h"
#include "IDT.h"
#include "Panic.h"

#include "ISR.h"

#define MODULE "ISR"

#define ISR_DIVIDE_BY_ZERO_ERROR	0x00
#define ISR_DOUBLE_FAULT			0x08
#define ISR_SEGMENT_NOT_PRESENT		0x0b
#define ISR_PAGE_FAULT				0x0e

// Global array of [un]registered interrupt handlers
ISR g_ISR[256];

static const char* const g_ExceptionTypes[] = {
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

void ISR_registerHandler(uint8_t vector, ISR handler){
	g_ISR[vector] = handler;
}

void ISR_deregisterHandler(uint8_t vector){
	g_ISR[vector] = NULL;
}

static inline void dumpRegisters(int logLevel, struct ISR_Params* params){
	log(logLevel, NULL, "vector=%#.2hhx rflags=%#.16llx err=%p", params->vector, params->rflags, params->err);
	log(logLevel, NULL, "\trip=%#.16llx rsp=%#.16llx rbp=%#.16llx", params->rip, params->rsp, params->rbp);
	log(logLevel, NULL, "\trax=%#.16llx rbx=%#.16llx rcx=%#.16llx rdx=%#.16llx", params->rax, params->rbx, params->rcx, params->rdx);
	log(logLevel, NULL, "\trsi=%#.16llx rdi=%#.16llx", params->rsi, params->rdi);
	log(logLevel, NULL, "\t r8=%#.16llx  r9=%#.16llx r10=%#.16llx r11=%#.16llx", params->r8, params->r9, params->r10, params->r11);
	log(logLevel, NULL, "\tr12=%#.16llx r13=%#.16llx r14=%#.16llx r15=%#.16llx", params->r12, params->r13, params->r14, params->r15);
	log(logLevel, NULL, "\tcs=%#.2hhx ds=%#.2hhx ss=%#.2hhx", params->cs, params->ds, params->ss);
}

// ================ ISR Handlers ================

// All ISR lead to a common ISR_Asm_Prehandler function, which calls this function
void ISR_C_prehandler(struct ISR_Params* params){

	// If we have a handler to call, we call it, and 'alles gut'
	if (g_ISR[params->vector] != NULL){
		g_ISR[params->vector](params);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = (params->vector < 32) ? g_ExceptionTypes[params->vector] : "Interrupt";
	log(PANIC, MODULE, "Unhandled %s !", interrupt_type);
	dumpRegisters(PANIC, params);
	panic();
}

void ISR_divisionByZeroError(struct ISR_Params* params){
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

	log(PANIC, MODULE, "%s", g_ExceptionTypes[params->vector]);
	log(PANIC, MODULE, "%s Exception in %s at descriptor=%p", origin, tableName, descriptor);
	if (!IDT_isInterruptHandlerEnabled(descriptor))
		log(PANIC, MODULE, "This was triggered because the IDT entry is disabled");
	panic();
}

void ISR_pageFault(struct ISR_Params* params){
	log(PANIC, MODULE, "Page fault (error=%#llx) ! From code at address %p", params->err, params->rip);
	panic();
}

// ================ Initialize ================

// In ISR_defs.c
void ISR_initializeInterruptHandlers();

void ISR_initialize(){
	ISR_initializeInterruptHandlers();

	// Enable all ISR assembly methods (in ISR_defs.s)
	// They all call ISR_Common with unified parameters
	for(int i=0 ; i<256 ; i++){
		IDT_enableInterruptHandler(i); // Enable Asm ISR
		g_ISR[i] = NULL; // Initialize C ISR
	}
	IDT_disableInterruptHandler(0x80); // until we implement system calls

	// Register our ISR handlers
	ISR_registerHandler(ISR_DIVIDE_BY_ZERO_ERROR, ISR_divisionByZeroError);
	ISR_registerHandler(ISR_DOUBLE_FAULT, ISR_doubleFault);
	ISR_registerHandler(ISR_SEGMENT_NOT_PRESENT, ISR_segmentNotPresent);
	ISR_registerHandler(ISR_PAGE_FAULT, ISR_pageFault);
}
