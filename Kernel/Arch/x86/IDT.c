#include <stdint.h>

#include "IDT.h"

// =========== IDT Entries ===========

// An IDT_Entry is a Gate, aka ISR Interrupt Service Routine, aka Interrupt handler
// Each Gate is either an Interrupt, a Task or a Trap Gate descriptor
// Offset: address of the entry point of the ISR
// Segment selector: a selector with multiple fields (must point to a valid code segment in the GDT)
typedef struct {
	uint16_t offset_0to15;				// Offset (bit 0-15)
	uint16_t segment_0to15;				// Segment selector (bit 0-15)
	uint8_t reserved;					// Unused, set to 0
	uint8_t attributes;					// Gate type, interrupt privilege level, present bit
	uint16_t offset_16to31;				// Offset (bit 15-31)
} __attribute__((packed)) IDT_Entry_32;

// =========== Descriptor ===========

// IDT Descriptor (32 bits)
typedef struct {
	uint16_t size;		// IDT size -1
	uint32_t offset;	// IDT linear address in memory (paging applies)
} __attribute__((packed)) IDT_LocationDescriptor_32;

// =========== Declare IDT ===========

// Global IDT variable, in (kernel) memory
IDT_Entry_32 g_IDT[256];

// Global IDT location descriptor, in (kernel) memory
IDT_LocationDescriptor_32 g_IDTLocationDescriptor32 = { sizeof(g_IDT)-1, (uint32_t) g_IDT };

// =========== IDT and ISR manipulations ===========

// x86_setIDT (IDT.asm) - Sets the IDT located at 'descriptor'
void __attribute__((cdecl)) x86_setIDT(IDT_LocationDescriptor_32* descriptor);

void x86_IDT_Initialize(){
	x86_setIDT(&g_IDTLocationDescriptor32);
}

void x86_IDT_EnableInterrupt(uint8_t interrupt){
	g_IDT[interrupt].attributes |= IDT_ATTR_PRESENT;
}

void x86_IDT_DisableInterrupt(uint8_t interrupt){
	// attributes = attributes & 0b01111111
	// 0b01111111 is 'not IDT_ATTR_PRESENT'
	g_IDT[interrupt].attributes &= (~IDT_ATTR_PRESENT);
}

void x86_IDT_SetInterruptHandler(uint8_t interrupt, void* base, uint16_t segmentDescriptor, uint8_t attributes){
	uint32_t offset = (uint32_t) base;
	g_IDT[interrupt].offset_0to15 = (offset & 0xffff);
	g_IDT[interrupt].segment_0to15 = segmentDescriptor;
	g_IDT[interrupt].reserved = 0;
	g_IDT[interrupt].attributes = attributes;
	g_IDT[interrupt].offset_16to31 = ((offset & 0xffff0000) >> 16);
}