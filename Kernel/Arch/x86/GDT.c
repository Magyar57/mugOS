#include <stdint.h>
#include "stdio.h"
#include "CPU.h"

#include "GDT.h"

// =========== GDT Entries ===========

// A GDT_Entry is a segment descriptor
// Base  (32-bit value): linear address where the segment begins
// Limit (20-bit value): tells maximum addressable unit (controlled by the granularity)
typedef struct {
	uint16_t limit_0to15;				// Limit (bit 0-15)
	uint16_t base_0to15;				// Base (bit 0-15)
	uint8_t base_16to23;				// Base (bit 16-23)
	uint8_t accessByte;					// Access byte (see GDT_ACCESS)
	uint8_t limit_16to19_and_flags;		// Limit (bit 16-19) followed by flags (on 4 bits)
	uint8_t base_24to31;				// Base (bit 24-31)
} __attribute__((packed)) GDT_Entry;

// Access bytes, bits:
// 7:   Present
// 6-5: Descriptor privilege level
// 4:   Descriptor type
// 3:   Executable bit
// 2:   Direction/Conforming bit
// 1:   Readable/writable bit
// 0:   Accessed bit
enum GDT_ACCESS {
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

	GDT_ACCESS_BIT =				0b00000001  // Set this to 1 if the segment is read only, otherwise CPU will generate a page fault by trying to set this bit
};

// Flags, bits:
// 3: Granularity flag
// 2: Size flag
// 1: Long-mode code flag
// 0: Reserved
enum GDT_Flags {
	GDT_FLAG_GRANULARITY_1B =	0b0000, // Segment size granularity of 1 Byte
	GDT_FLAG_GRANULARITY_4K =	0b1000, // Segment size granularity of 4 KiB

	GDT_FLAG_SIZE_16_PMODE =	0b0000, // Segment is 16-bit protected mode
	GDT_FLAG_SIZE_32_PMODE =	0b0100, // Segment is 32-bit protected mode

	GDT_FLAG_LONG_MODE_OFF =	0b0000, // Segment is anything but 64-bit code
	GDT_FLAG_LONG_MODE_ON =		0b0010, // Segment is 64-bit code. If set, SIZE should be clear (set to GDT_FLAG_SIZE_16_PMODE)
};

// =========== Descriptors ===========

// GDT Descriptor (32 bits mode)
typedef struct {
	uint16_t size;		// GDT size -1
	uint32_t offset;	// GDT address in memory
} __attribute__((packed)) GDT_LocationDescriptor_32;

// GDT Descriptor (64 bits mode)
typedef struct {
	uint16_t size;		// GDT size -1
	uint64_t offset;	// GDT address in memory
} __attribute__((packed)) GDT_LocationDescriptor_64;

// =========== GDT generation maccros ===========

#define GDT_get_limit_0to15(limit)						(uint16_t) (limit & 0xffff)
#define GDT_get_base_0to15(base)						(uint16_t) (base & 0xffff)
#define GDT_get_base_16to23(base)						(uint8_t)  ((base >> 16) & 0xff)
#define GDT_get_limit_16to19_and_flags(limit, flags)	(uint8_t)  (((limit >> 16) & 0x0f) | (((flags)<<4) & 0xf0))
#define GDT_get_base_24to31(base)						(uint8_t)  ((base >> 24) & 0xff)

#define GDT_get_GDT_Entry(base, limit, access, flags) \
	{ GDT_get_limit_0to15(limit), GDT_get_base_0to15(base), GDT_get_base_16to23(base), access, GDT_get_limit_16to19_and_flags(limit, flags), GDT_get_base_24to31(base) }

// =========== Declare GDT and assign GDT entries ===========

#define GDT_KERNEL_CODE_ACCESS GDT_ACCESS_PRESENT|GDT_ACCESS_RING0|GDT_ACCESS_TYPE_CODE_OR_DATA|GDT_ACCESS_EXEC_ALLOW|GDT_ACCESS_DC_UP|GDT_ACCESS_RW_ALLOW
#define GDT_KERNEL_DATA_ACCESS GDT_ACCESS_PRESENT|GDT_ACCESS_RING0|GDT_ACCESS_TYPE_CODE_OR_DATA|GDT_ACCESS_EXEC_FORBID|GDT_ACCESS_DC_UP|GDT_ACCESS_RW_ALLOW
#define GDT_KERNEL_CODE_FLAGS  GDT_FLAG_GRANULARITY_4K|GDT_FLAG_SIZE_32_PMODE|GDT_FLAG_LONG_MODE_OFF
#define GDT_KERNEL_DATA_FLAGS  GDT_FLAG_GRANULARITY_4K|GDT_FLAG_SIZE_32_PMODE|GDT_FLAG_LONG_MODE_OFF

// Global GDT variable, in (kernel) memory
GDT_Entry g_GDT[] = {
	// Null descriptor
	GDT_get_GDT_Entry(0x0, 0x0, 0, 0),
	// Kernel 32-bit code segment
	GDT_get_GDT_Entry(0x0, 0xfffff, GDT_KERNEL_CODE_ACCESS, 0xc),
	// Kernel 32-bit data segment
	GDT_get_GDT_Entry(0x0, 0xfffff, GDT_KERNEL_DATA_ACCESS, GDT_KERNEL_DATA_FLAGS)
};

// Global GDT location descriptor, in (kernel) memory
GDT_LocationDescriptor_32 g_GDTLocationDescriptor32 = { sizeof(g_GDT)-1, (uint32_t) g_GDT };

// setGDT - Defined in GDT.asm
// Sets the GDT located at 'descriptor' and loads the segments registers accordingly (kcodeSegment & kdataSegment)
void __attribute__((cdecl)) setGDT(GDT_LocationDescriptor_32* descriptor, uint16_t kcodeSegment, uint16_t kdataSegment);

void GDT_initialize(){
	setGDT(&g_GDTLocationDescriptor32, GDT_SEGMENT_KTEXT, GDT_SEGMENT_KDATA);
}
