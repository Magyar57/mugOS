#ifndef __ISR_H__
#define __ISR_H__

#include <stdint.h>
#include "Preprocessor.h"

struct ISR_Params {
	uint64_t ds;
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
	uint64_t vector, err;
	uint64_t rip, cs, rflags, rsp, ss;
} packed;

typedef void (*isr_t)(struct ISR_Params*);

/// @brief Initialize the ISR (Interrupt Service Routines) in the IDT
/// @note Call IDT_init beforehand !
void ISR_init();

/// @brief Install a handler for a given interrupt vector
void ISR_installHandler(uint8_t vector, isr_t handler);

/// @brief Remove (clear) a handler for a given interrupt vector
void ISR_removeHandler(uint8_t vector);

/// @returns Whether an handler is already installed for the given `vector` ISR
bool ISR_isHandlerPresent(uint8_t vector);

#endif
