#include <stdint.h>
#include "Platform/GDT.h"

#include "HAL/Userspace/Usermode.h"

// Usermode.asm
extern void x86_jumpToUsermode(uint64_t ucodeSegment, uint64_t udataSegment);

void jumpToUsermode(){
	x86_jumpToUsermode(GDT_SEGMENT_UTEXT, GDT_SEGMENT_UDATA);
}
