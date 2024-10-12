#ifndef __ISR_H__
#define __ISR_H__

typedef struct {
	uint32_t ds;
	uint32_t edi, esi, ebp, ignored, ebx, edx, ecx, eax;
	uint32_t vector, err;
	uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed)) ISR_Params;

typedef void (*ISR)(ISR_Params*);

// Initialize the ISR (Interrupt Service Routines) in the IDT
void x86_ISR_Initialize();

// Register a handler for a given interrupt vector
void x86_ISR_RegisterHandler(uint8_t vector, ISR handler);

// Deregister a handler for a given interrupt vector
void x86_ISR_DeregisterHandler(uint8_t vector);

#endif
