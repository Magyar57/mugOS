#ifndef __GDT_H__
#define __GDT_H__

#define GDT_SEGMENT_KTEXT		0x08
#define GDT_SEGMENT_KDATA		0x10
#define GDT_SEGMENT_UTEXT		0x18
#define GDT_SEGMENT_UDATA		0x20
#define GDT_SEGMENT_TSS_CPU0	0x28

// Initialize the GDT
void GDT_init();

// Initialize the TSS for CPU0
void GDT_setTSS();

#endif
