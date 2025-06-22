#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

// Flags values for Gates (= ISR = Interrupt handlers)
enum IDT_ATTRIBUTES_64 {
	IDT_ATTR_GATE_INTERRUPT64 =	0b00001110, // Gate is Interrupt Gate (64-bit code)
	IDT_ATTR_GATE_TRAP64 =		0b00001111, // Gate is a Trap Gate (64-bit code)

	IDT_ATTR_RING0 =			0b00000000, // Gate is accessible from ring 0
	IDT_ATTR_RING1 =			0b00100000, // Gate is accessible from ring <= 1
	IDT_ATTR_RING2 =			0b01000000, // Gate is accessible from ring <= 2
	IDT_ATTR_RING3 =			0b01100000, // Gate is accessible from ring <= 3

	IDT_ATTR_PRESENT =			0b10000000 // Gate is enabled (disabled if flag not present)
};

// Initialize the IDT (after this call, all interrupt handlers are invalid!)
void IDT_init();

// Enable the 'interrupt' interrupt handler
void IDT_enableInterruptHandler(uint8_t interrupt);

// Returns wheather an interrupt handler is enabled
bool IDT_isInterruptHandlerEnabled(uint8_t interrupt);

// Disable the 'interrupt' interrupt handler
void IDT_disableInterruptHandler(uint8_t interrupt);

// Set an interrupt handler ; see IDT_ATTRIBUTES enum for the attributes
void IDT_setInterruptHandler(uint8_t interrupt, void* base, uint16_t segmentDescriptor, uint8_t attributes);

#endif
