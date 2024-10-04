#ifndef __GDT_H__
#define __GDT_H__

#define GDT_SEGMENT_KTEXT 0x08
#define GDT_SEGMENT_KDATA 0x10

// Initalize the GDT
void x86_GDT_Initialize();

#endif
