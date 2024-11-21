#ifndef __ISR_H__
#define __ISR_H__

#include <stdint.h>

typedef struct {
	uint32_t ds;
	uint32_t edi, esi, ebp, ignored, ebx, edx, ecx, eax;
	uint32_t vector, err;
	uint32_t eip, cs, eflags, esp, ss;
} __attribute__((packed)) ISR_Params;

typedef void (*ISR)(ISR_Params*);

// Initialize the ISR (Interrupt Service Routines) in the IDT (important: call IDT_initialize beforehand !)
void ISR_initialize();

// Register a handler for a given interrupt vector
void ISR_registerHandler(uint8_t vector, ISR handler);

// Deregister a handler for a given interrupt vector
void ISR_deregisterHandler(uint8_t vector);

#endif
