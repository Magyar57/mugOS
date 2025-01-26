#ifndef __GDT_H__
#define __GDT_H__

#define GDT_SEGMENT_KTEXT 0x08
#define GDT_SEGMENT_KDATA 0x10

// Initialize the GDT
void GDT_initialize();

#endif
