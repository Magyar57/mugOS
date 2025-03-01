#include <stdint.h>
#include "Preprocessor.h"

#include "IDT.h"

// ================ IDT Entries ================

// An IDT_Entry is a Gate, aka ISR Interrupt Service Routine, aka Interrupt handler
// Each Gate is either an Interrupt, a Task or a Trap Gate descriptor
// Offset: address of the entry point of the ISR
// Segment selector: a selector with multiple fields (must point to a valid code segment in the GDT)
struct IDTEntry {
	uint16_t offset_0to15;				// Offset (bit 0-15)
	uint16_t segment_0to15;				// Segment selector (bit 0-15)
	uint8_t reserved0;					// Unused, set to 0
	uint8_t attributes;					// Gate type, interrupt privilege level, present bit
	uint16_t offset_16to31;				// Offset (bit 15-31)
	uint32_t offset_32to63;				// Offset (bit 32-63)
	uint32_t reserved1;					// Unused, set to 0
} packed;

// ================ Descriptor ================

// IDT Descriptor (64 bits)
struct IDTLocationDescriptor {
	uint16_t size;		// IDT size -1
	uint64_t offset;	// IDT linear address in memory (paging applies)
} packed;

// ================ Declare IDT ================

// Global IDT variable, in (kernel) memory
struct IDTEntry g_IDT[256];

// Global IDT location descriptor, in (kernel) memory
struct IDTLocationDescriptor g_IDTLocationDescriptor = { sizeof(g_IDT)-1, (uint64_t) g_IDT };

// ================ IDT and ISR manipulations ================

// setIDT (IDT.asm) - Sets the IDT located at 'descriptor'
void setIDT(struct IDTLocationDescriptor* descriptor);

void IDT_initialize(){
	setIDT(&g_IDTLocationDescriptor);
}

bool IDT_isInterruptHandlerEnabled(uint8_t interrupt){
	return (g_IDT[interrupt].attributes & IDT_ATTR_PRESENT);
}

void IDT_enableInterruptHandler(uint8_t interrupt){
	g_IDT[interrupt].attributes |= IDT_ATTR_PRESENT;
}

void IDT_disableInterruptHandler(uint8_t interrupt){
	// attributes = attributes & 0b01111111
	// 0b01111111 is 'not IDT_ATTR_PRESENT'
	g_IDT[interrupt].attributes &= (~IDT_ATTR_PRESENT);
}

void IDT_setInterruptHandler(uint8_t interrupt, void* base, uint16_t segmentDescriptor, uint8_t attributes){
	uint64_t offset = (uint64_t) base;
	g_IDT[interrupt].offset_0to15  = (offset & 0x000000000000ffff);
	g_IDT[interrupt].segment_0to15 = segmentDescriptor;
	g_IDT[interrupt].reserved0     = 0;
	g_IDT[interrupt].attributes    = attributes;
	g_IDT[interrupt].offset_16to31 = ((offset & 0x00000000ffff0000) >> 16);
	g_IDT[interrupt].offset_32to63 = ((offset & 0xffffffff00000000) >> 32);
	g_IDT[interrupt].reserved1     = 0;
}
