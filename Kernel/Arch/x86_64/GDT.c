#include <stdint.h>
#include "string.h"
#include "HAL/CPU.h"

#include "GDT.h"

// ================ GDT Entries ================

// A GDT entry is either a Segment Descriptor (describing memory segments, loaded in the
// segment registers), or a System Segment Descriptor (describing TSS or LDT).

// Base  (32-bit value): linear address where the segment begins
// Limit (20-bit value): tells maximum addressable unit (controlled by the granularity)
struct SegmentDescriptor {
	uint16_t limit_0to15;				// Limit (bit 0-15)
	uint16_t base_0to15;				// Base (bit 0-15)
	uint8_t base_16to23;				// Base (bit 16-23)
	uint8_t accessByte;					// Access byte (see SDAccess or SSDAccess)
	uint8_t limit_16to19_and_flags;		// Limit (bit 16-19) followed by flags (on 4 bits)
	uint8_t base_24to31;				// Base (bit 24-31)
} packed;

// Note: SystemSegmentDescriptor is twice as big as a SegmentDescriptor
struct SystemSegmentDescriptor {
	struct SegmentDescriptor common; // this is the same as segment descriptors, so we reuse the struct
	uint32_t base_32to63;
	uint32_t reserved;
} packed;

// [System] Segment Descriptor Access byte
enum Access {
	// These are common to SD and SSD
	ACCESS_PRESENT =				0b10000000, // bit 7: set = valid entry, clear = invalid entr
	ACCESS_RING0 =					0b00000000, // bits 6-5: Ring 0 (kernel)
	ACCESS_RING1 =					0b00100000, // bits 6-5: Ring 1 (driver)
	ACCESS_RING2 =					0b01000000, // bits 6-5: Ring 2 (driver)
	ACCESS_RING3 =					0b01100000, // bits 6-5: Ring 3 (user mode)
	ACCESS_TYPE_SSD =				0b00000000, // bit 4: Clear: Descriptor is a System Segment Descriptor
	ACCESS_TYPE_SD =				0b00010000, // bit 4: Set: Descriptor is a Segment Descriptor
	// These are exclusive to Segments Descriptors
	ACCESS_SD_EXEC_FORBID =			0b00000000, // bit 3: Segment not executable (=> data segment)
	ACCESS_SD_EXEC_ALLOW =			0b00001000, // bit 3: Segment executable (=> code segment)
	ACCESS_SD_DC_UP =				0b00000000, // bit 2: (FOR DATA SEGMENTS) Segment grows up   || (FOR CODE SEGMENTS) Lower privilege execution forbidden
	ACCESS_SD_DC_DOWN =				0b00000100, // bit 2: (FOR DATA SEGMENTS) Segment grows down || (FOR CODE SEGMENTS) Lower privilege execution allowed
	ACCESS_SD_RW_FORBID =			0b00000000, // bit 1: (FOR CODE SEGMENTS) Read forbidden || (FOR DATA SEGMENTS) Write forbidden
	ACCESS_SD_RW_ALLOW =			0b00000010, // bit 1: (FOR CODE SEGMENTS) Read allowed   || (FOR DATA SEGMENTS) Write allowed
	ACCESS_SD_BIT =					0b00000001, // bit 0: Accessed bit. CPU will set it bit when accessing the segment
	// These are exclusive to System Segments Descriptors (note: there are only long mode bits here)
	ACCESS_SSD_TYPE_LDT = 			0b00000010, // bits 3-0: System Segment type = Local Descriptor Table
	ACCESS_SSD_TYPE_TSS_AVAILABLE = 0b00001001, // bits 3-0: System Segment type = TSS (Available)
	ACCESS_SSD_TYPE_TSS_BUSY = 		0b00001011, // bits 3-0: System Segment type = TSS (Busy)
};

// System descriptor flags, bits:
enum Flags {
	SD_FLAG_GRANULARITY_1B =	0b0000, // bit 3: Segment size granularity of 1 Byte
	SD_FLAG_GRANULARITY_4K =	0b1000, // bit 3: Segment size granularity of 4 KiB
	SD_FLAG_SIZE_16PMODE =		0b0000, // bit 2: Segment is 16-bit protected mode
	SD_FLAG_SIZE_32PMODE =		0b0100, // bit 2: Segment is 32-bit protected mode
	SD_FLAG_LONGMODE_OFF =		0b0000, // bit 1: Segment is anything but 64-bit code
	SD_FLAG_LONGMODE_ON =		0b0010, // bit 1: Segment is 64-bit code. If set, SIZE should be clear (set to SDFLAG_SIZE_16_PMODE)
	// bit 0 is reserved
};

// ================ GDT Descriptor ================

// GDT Descriptor (long mode)
struct GDTDescriptor {
	uint16_t size;		// GDT size -1
	uint64_t offset;	// GDT address in memory
} packed;

// ================ [System] Segment descriptor generation maccros ================

#define getLimit0to15(limit)						(uint16_t) (limit & 0xffff)
#define getBase0to15(base)							(uint16_t) (base & 0xffff)
#define getBase16to23(base)							(uint8_t)  ((base >> 16) & 0xff)
#define GDT_getLimit16to19AndFlags(limit, flags)	(uint8_t)  (((limit >> 16) & 0x0f) | (((flags)<<4) & 0xf0))
#define getBase24to31(base)							(uint8_t)  ((base >> 24) & 0xff)
#define getBase32to63(base)							(uint32_t) ((base & 0xffffffff00000000) >> 32)

#define getSegmentDescriptor(base, limit, access, flags) \
	{ getLimit0to15(limit), getBase0to15(base), getBase16to23(base), access, GDT_getLimit16to19AndFlags(limit, flags), getBase24to31(base) }

#define getEmptySystemSegmentDescriptor() \
	getSegmentDescriptor(0x0, 0x0, 0, 0), getSegmentDescriptor(0x0, 0x0, 0, 0)

// ================ Declare GDT ================

#define GDT_KTEXT_ACCESS ACCESS_PRESENT|ACCESS_RING0|ACCESS_TYPE_SD|ACCESS_SD_EXEC_ALLOW|ACCESS_SD_DC_UP|ACCESS_SD_RW_ALLOW
#define GDT_KTEXT_FLAGS  SD_FLAG_GRANULARITY_4K|SD_FLAG_LONGMODE_ON
#define GDT_KDATA_ACCESS ACCESS_PRESENT|ACCESS_RING0|ACCESS_TYPE_SD|ACCESS_SD_EXEC_FORBID|ACCESS_SD_DC_UP|ACCESS_SD_RW_ALLOW
#define GDT_KDATA_FLAGS  SD_FLAG_GRANULARITY_4K|SD_FLAG_SIZE_16PMODE|SD_FLAG_LONGMODE_ON

#define GDT_UTEXT_ACCESS ACCESS_PRESENT|ACCESS_RING3|ACCESS_TYPE_SD|ACCESS_SD_EXEC_ALLOW|ACCESS_SD_DC_UP|ACCESS_SD_RW_ALLOW
#define GDT_UTEXT_FLAGS  SD_FLAG_GRANULARITY_4K|SD_FLAG_LONGMODE_ON
#define GDT_UDATA_ACCESS ACCESS_PRESENT|ACCESS_RING3|ACCESS_TYPE_SD|ACCESS_SD_EXEC_FORBID|ACCESS_SD_DC_UP|ACCESS_SD_RW_ALLOW
#define GDT_UDATA_FLAGS  SD_FLAG_GRANULARITY_4K|SD_FLAG_SIZE_16PMODE|SD_FLAG_LONGMODE_ON

// The actual GDT
static struct SegmentDescriptor m_GDT[] = {
	getSegmentDescriptor(0x0, 0x0, 0, 0),										// Null descriptor
	getSegmentDescriptor(0x0, 0xffffffff, GDT_KTEXT_ACCESS, GDT_KTEXT_FLAGS),	// Kernel 64-bit code segment
	getSegmentDescriptor(0x0, 0xffffffff, GDT_KDATA_ACCESS, GDT_KDATA_FLAGS),	// Kernel 64-bit data segment
	getSegmentDescriptor(0x0, 0xffffffff, GDT_UTEXT_ACCESS, GDT_UTEXT_FLAGS),	// Usermode 64-bit code segment
	getSegmentDescriptor(0x0, 0xffffffff, GDT_UDATA_ACCESS, GDT_UDATA_FLAGS),	// Usermode 64-bit data segment
	getEmptySystemSegmentDescriptor()											// Placeholder for a TSS descriptor
};

static struct GDTDescriptor m_GDTLocationDescriptor = { sizeof(m_GDT)-1, (uint64_t) m_GDT };

// (GDT.asm) Sets the GDT located at 'descriptor' and loads the segments registers accordingly (kcodeSegment & kdataSegment)
void setGDT(struct GDTDescriptor* descriptor, uint16_t kcodeSegment, uint16_t kdataSegment);

void GDT_initialize(){
	setGDT(&m_GDTLocationDescriptor, GDT_SEGMENT_KTEXT, GDT_SEGMENT_KDATA);
}

// ================ TSS ================

// 64 bits (long mode) Task State Segment descriptor
// Note: the IST (Interrupt Stack Table) are used when the IST bits are !=0 in the IDT that is beeing run
struct TSS {
	uint32_t reserved0;
	uint64_t rsp0;		// ring 0 stack pointer
	uint64_t rsp1;		// ring 1 stack pointer
	uint64_t rsp2;		// ring 2 stack pointer
	uint64_t reserved1;
	uint64_t ist1;		// Interrupt Stack Table: a stack to use for an ISR, if (IST bit==1) in the IDT entry for this ISR
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iobp;		// IO map base address (a 16 bit offset from the base of the TSS to the IO permission bit map)
} packed;

// 4 KiB stacks (TODO change for kmalloc allocations when kmalloc implemented)
uint8_t stack0[4096];
uint8_t stack1[4096];
uint8_t stack2[4096];

static struct TSS m_TSS = {
	.reserved0 = 0,
	.rsp0 = (uint64_t) stack0,
	.rsp1 = (uint64_t) stack1,
	.rsp2 = (uint64_t) stack2,
	.reserved1 = 0,
	.ist1 = (uint64_t) NULL,
	.ist2 = (uint64_t) NULL,
	.ist3 = (uint64_t) NULL,
	.ist4 = (uint64_t) NULL,
	.ist5 = (uint64_t) NULL,
	.ist6 = (uint64_t) NULL,
	.ist7 = (uint64_t) NULL,
	.reserved2 = 0,
	.reserved3 = 0,
	.iobp = 0,
};

#define TSS_CPU0_FLAGS		0x0
#define TSS_CPU0_ACCESS		ACCESS_PRESENT|ACCESS_RING0|ACCESS_TYPE_SSD|ACCESS_SSD_TYPE_TSS_AVAILABLE

// (GDT.asm)
void setTSS(uint16_t TSS_descriptor);

void GDT_setTSS(){
	struct SystemSegmentDescriptor* entry = (struct SystemSegmentDescriptor*) &m_GDT[5];
	const uint64_t base = (uint64_t) &m_TSS;
	const uint16_t limit = sizeof(struct TSS)-1;
	const uint8_t access = TSS_CPU0_ACCESS;
	const uint8_t flags = TSS_CPU0_FLAGS;

	entry->common.limit_0to15 = getLimit0to15(limit);
	entry->common.base_0to15 = getBase0to15(base);
	entry->common.base_16to23 = getBase16to23(base);
	entry->common.accessByte = access;
	entry->common.limit_16to19_and_flags = GDT_getLimit16to19AndFlags(limit, flags);
	entry->common.base_24to31 = getBase24to31(base);
	entry->base_32to63 = getBase32to63(base);

	setTSS(GDT_SEGMENT_TSS_CPU0);
}
