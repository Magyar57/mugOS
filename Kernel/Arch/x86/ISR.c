#include <stdint.h>
#include <stddef.h>
#include "Logging.h"
#include "CPU.h"
#include "GDT.h"
#include "IDT.h"
#include "Panic.h"

#include "ISR.h"

#define MODULE "ISR"

#define ISR_DIVIDE_BY_ZERO_ERROR	0x00
#define ISR_DOUBLE_FAULT			0x08

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

// ================ ISR Handlers ================

// All ISR lead to a common ISR_Asm_Prehandler function, which calls this function
void __attribute__((cdecl)) ISR_C_prehandler(ISR_Params* params){

	// If we have a handler to call, we call it, and 'alles gut'
	if (g_ISR[params->vector] != NULL){
		g_ISR[params->vector](params);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = (params->vector < 32) ? g_ExceptionTypes[params->vector] : "Interrupt";
	log(PANIC, MODULE, "Unhandled %s !", interrupt_type);
	log(PANIC, NULL, "-> vector=%p eflags=%p err=%p", params->vector, params->eflags, params->err);
	log(PANIC, NULL, "-> eax=%p ebx=%p ecx=%p edx=%p", params->eax, params->ebx, params->ecx, params->edx);
	log(PANIC, NULL, "-> esi=%p edi=%p", params->esi, params->edi);
	log(PANIC, NULL, "-> eip=%p esp=%p ebp=%p", params->eip, params->esp, params->ebp);
	log(PANIC, NULL, "-> cs=%p ds=%p ss=%p", params->cs, params->ds, params->ss);
	panic();
}

void ISR_divisionByZeroError(ISR_Params* params){
	log(PANIC, NULL, "Division by zero error !!");
	panic();
}

void ISR_doubleFault(ISR_Params* params){
	log(PANIC, NULL, "Double fault !!");
	log(PANIC, NULL, "-> vector=%p eflags=%p err=%p", params->vector, params->eflags, params->err);
	log(PANIC, NULL, "-> eax=%p ebx=%p ecx=%p edx=%p", params->eax, params->ebx, params->ecx, params->edx);
	log(PANIC, NULL, "-> esi=%p edi=%p", params->esi, params->edi);
	log(PANIC, NULL, "-> eip=%p esp=%p ebp=%p", params->eip, params->esp, params->ebp);
	log(PANIC, NULL, "-> cs=%p ds=%p ss=%p", params->cs, params->ds, params->ss);
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

	// Register our ISR handlers
	ISR_registerHandler(ISR_DIVIDE_BY_ZERO_ERROR, ISR_divisionByZeroError);
	ISR_registerHandler(ISR_DOUBLE_FAULT, ISR_doubleFault);
}
