#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "Interrupts.h"
#include "GDT.h"
#include "IDT.h"
#include "Panic.h"

#include "ISR.h"

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

// In ISR_defs.c
void x86_ISR_InitializeInterruptHandlers();

void x86_ISR_Initialize(){
	x86_ISR_InitializeInterruptHandlers();

	// Enable all ISR assembly methods (in ISR_defs.s)
	// They all call x86_ISR_Common with unified parameters
	for(int i=0 ; i<256 ; i++){
		x86_IDT_EnableInterruptHandler(i);
	}
	x86_IDT_EnableInterruptHandler(0x80);
}

void x86_ISR_RegisterHandler(uint8_t vector, ISR handler){
	g_ISR[vector] = handler;
}

void x86_ISR_DeregisterHandler(uint8_t vector){
	g_ISR[vector] = NULL;
}

// All ISR lead to a common x86_ISR_Asm_Prehandler function, which calls this function
void __attribute__((cdecl)) x86_ISR_C_Prehandler(ISR_Params* params){

	// If we have a handler to call, we call it, and 'alles gut'
	if (g_ISR[params->vector] != NULL){
		g_ISR[params->vector](params);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = (params->vector < 32) ? g_ExceptionTypes[params->vector] : "Interrupt";
	printf("Unhandled %s !\n", interrupt_type);
	printf("\tvector=%p eflags=%p err=%p\n", params->vector, params->eflags, params->err);
	printf("\teax=%p ebx=%p ecx=%p edx=%p esi=%p edi=%p\n",
		params->eax, params->ebx, params->ecx, params->edx, params->esi, params->edi
	);
	printf("\teip=%p esp=%p ebp=%p\n", params->eip, params->esp, params->ebp);
	printf("\tcs=%p ds=%p ss=%p\n", params->cs, params->ds, params->ss);
	PANIC();
}
