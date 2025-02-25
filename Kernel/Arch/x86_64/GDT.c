#include <stdint.h>
#include "stdio.h"
#include "CPU.h"

#include "GDT.h"

// =========== GDT Entries ===========

// A GDTEntry is a segment descriptor
// Base  (32-bit value): linear address where the segment begins
// Limit (20-bit value): tells maximum addressable unit (controlled by the granularity)
struct GDTEntry {
	uint16_t limit_0to15;				// Limit (bit 0-15)
	uint16_t base_0to15;				// Base (bit 0-15)
	uint8_t base_16to23;				// Base (bit 16-23)
	uint8_t accessByte;					// Access byte (see GDTAccess)
	uint8_t limit_16to19_and_flags;		// Limit (bit 16-19) followed by flags (on 4 bits)
	uint8_t base_24to31;				// Base (bit 24-31)
} __attribute__((packed));

// Access bytes, bits:
// 7:   Present
// 6-5: Descriptor privilege level
// 4:   Descriptor type
// 3:   Executable bit
// 2:   Direction/Conforming bit
// 1:   Readable/writable bit
// 0:   Accessed bit
enum GDTAccess {
	GDT_ACCESS_PRESENT =			0b10000000, // Present = 1: valid entry/segment (0=invalid entry)

	GDT_ACCESS_RING0 =				0b00000000, // Ring 0 (kernel)
	GDT_ACCESS_RING1 =				0b00100000, // Ring 1 (driver)
	GDT_ACCESS_RING2 =				0b01000000, // Ring 2 (driver)
	GDT_ACCESS_RING3 =				0b01100000, // Ring 3 (user mode)

	GDT_ACCESS_TYPE_SYSTEM =		0b00000000, // Segment type 0 (system segment)
	GDT_ACCESS_TYPE_CODE_OR_DATA =	0b00010000, // Segment type 1 (code/data segment)

	GDT_ACCESS_EXEC_FORBID =		0b00000000, // Segment not executable (=> data segment)
	GDT_ACCESS_EXEC_ALLOW =			0b00001000, // Segment executable (=> code segment)

	GDT_ACCESS_DC_UP =				0b00000000, // (FOR DATA SEGMENTS) Segment grows up   || (FOR CODE SEGMENTS) Lower privilege execution forbidden
	GDT_ACCESS_DC_DOWN =			0b00000100, // (FOR DATA SEGMENTS) Segment grows down || (FOR CODE SEGMENTS) Lower privilege execution allowed

	GDT_ACCESS_RW_FORBID =			0b00000000, // (FOR CODE SEGMENTS) Read forbidden || (FOR DATA SEGMENTS) Write forbidden
	GDT_ACCESS_RW_ALLOW =			0b00000010, // (FOR CODE SEGMENTS) Read allowed   || (FOR DATA SEGMENTS) Write allowed

	GDT_ACCESS_BIT =				0b00000001  // CPU will set this bit (if cleared) when accessing the segment
};

// Flags, bits:
// 3: Granularity flag
// 2: Size flag
// 1: Long-mode code flag
// 0: Reserved
enum GDTFlags {
	GDT_FLAG_GRANULARITY_1B =	0b0000, // Segment size granularity of 1 Byte
	GDT_FLAG_GRANULARITY_4K =	0b1000, // Segment size granularity of 4 KiB

	GDT_FLAG_SIZE_16_PMODE =	0b0000, // Segment is 16-bit protected mode
	GDT_FLAG_SIZE_32_PMODE =	0b0100, // Segment is 32-bit protected mode

	GDT_FLAG_LONG_MODE_OFF =	0b0000, // Segment is anything but 64-bit code
	GDT_FLAG_LONG_MODE_ON =		0b0010, // Segment is 64-bit code. If set, SIZE should be clear (set to GDT_FLAG_SIZE_16_PMODE)
};

// =========== Descriptors ===========

// GDT Descriptor (long mode)
struct GDTLocationDescriptor {
	uint16_t size;		// GDT size -1
	uint64_t offset;	// GDT address in memory
} __attribute__((packed));

// =========== GDT generation maccros ===========

#define GDT_get_limit_0to15(limit)						(uint16_t) (limit & 0xffff)
#define GDT_get_base_0to15(base)						(uint16_t) (base & 0xffff)
#define GDT_get_base_16to23(base)						(uint8_t)  ((base >> 16) & 0xff)
#define GDT_get_limit_16to19_and_flags(limit, flags)	(uint8_t)  (((limit >> 16) & 0x0f) | (((flags)<<4) & 0xf0))
#define GDT_get_base_24to31(base)						(uint8_t)  ((base >> 24) & 0xff)

#define getGDTEntry(base, limit, access, flags) \
	{ GDT_get_limit_0to15(limit), GDT_get_base_0to15(base), GDT_get_base_16to23(base), access, GDT_get_limit_16to19_and_flags(limit, flags), GDT_get_base_24to31(base) }

// =========== Declare GDT and assign GDT entries ===========

#define GDT_KTEXT_ACCESS GDT_ACCESS_PRESENT|GDT_ACCESS_RING0|GDT_ACCESS_TYPE_CODE_OR_DATA|GDT_ACCESS_EXEC_ALLOW|GDT_ACCESS_DC_UP|GDT_ACCESS_RW_ALLOW
#define GDT_KTEXT_FLAGS  GDT_FLAG_GRANULARITY_4K|GDT_FLAG_LONG_MODE_ON
#define GDT_KDATA_ACCESS GDT_ACCESS_PRESENT|GDT_ACCESS_RING0|GDT_ACCESS_TYPE_CODE_OR_DATA|GDT_ACCESS_EXEC_FORBID|GDT_ACCESS_DC_UP|GDT_ACCESS_RW_ALLOW
#define GDT_KDATA_FLAGS  GDT_FLAG_GRANULARITY_4K|GDT_FLAG_SIZE_16_PMODE|GDT_FLAG_LONG_MODE_ON

#define GDT_UTEXT_ACCESS GDT_ACCESS_PRESENT|GDT_ACCESS_RING3|GDT_ACCESS_TYPE_CODE_OR_DATA|GDT_ACCESS_EXEC_ALLOW|GDT_ACCESS_DC_UP|GDT_ACCESS_RW_ALLOW
#define GDT_UTEXT_FLAGS  GDT_FLAG_GRANULARITY_4K|GDT_FLAG_LONG_MODE_ON
#define GDT_UDATA_ACCESS GDT_ACCESS_PRESENT|GDT_ACCESS_RING3|GDT_ACCESS_TYPE_CODE_OR_DATA|GDT_ACCESS_EXEC_FORBID|GDT_ACCESS_DC_UP|GDT_ACCESS_RW_ALLOW
#define GDT_UDATA_FLAGS  GDT_FLAG_GRANULARITY_4K|GDT_FLAG_SIZE_16_PMODE|GDT_FLAG_LONG_MODE_ON

// Global GDT variable, in (kernel) memory
struct GDTEntry g_GDT[] = {
	getGDTEntry(0x0, 0x0, 0, 0),										// Null descriptor
	getGDTEntry(0x0, 0xffffffff, GDT_KTEXT_ACCESS, GDT_KTEXT_FLAGS),	// Kernel 64-bit code segment
	getGDTEntry(0x0, 0xffffffff, GDT_KDATA_ACCESS, GDT_KDATA_FLAGS),	// Kernel 64-bit data segment
	getGDTEntry(0x0, 0xffffffff, GDT_UTEXT_ACCESS, GDT_UTEXT_FLAGS),	// Usermode 64-bit code segment
	getGDTEntry(0x0, 0xffffffff, GDT_UDATA_ACCESS, GDT_UDATA_FLAGS),	// Usermode 64-bit data segment
};

// Global GDT location descriptor, in (kernel) memory
struct GDTLocationDescriptor g_GDTLocationDescriptor = { sizeof(g_GDT)-1, (uint64_t) g_GDT };

// setGDT - Defined in GDT.asm
// Sets the GDT located at 'descriptor' and loads the segments registers accordingly (kcodeSegment & kdataSegment)
void setGDT(struct GDTLocationDescriptor* descriptor, uint16_t kcodeSegment, uint16_t kdataSegment);

void GDT_initialize(){
	setGDT(&g_GDTLocationDescriptor, GDT_SEGMENT_KTEXT, GDT_SEGMENT_KDATA);
}
