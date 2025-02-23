#ifndef __ISR_H__
#define __ISR_H__

#include <stdint.h>

struct ISR_Params {
	uint64_t ds;
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
	uint64_t vector, err;
	uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

typedef void (*ISR)(struct ISR_Params*);

// Initialize the ISR (Interrupt Service Routines) in the IDT (important: call IDT_initialize beforehand !)
void ISR_initialize();

// Register a handler for a given interrupt vector
void ISR_registerHandler(uint8_t vector, ISR handler);

// Deregister a handler for a given interrupt vector
void ISR_deregisterHandler(uint8_t vector);

#endif
