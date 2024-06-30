#ifndef __MEMDEFS_H__
#define __MEMDEFS_H__

// See https://wiki.osdev.org/Memory_Map_(x86)
// Memory scheme (from low to high addresses):

// 0x00000000 - 0x000003ff - IVT
// 0x00000400 - 0x000004ff - BIOS data area

#define MEMORY_MIN 0x00000500 // Beginning of usable memory

// 0x00000500 - 0x00020000 - Stage 2 bootloader (the exact size varies, if it exeeds 0x00020000-0x00000500 we're in troubles)

// 0x00020000 - 0x00030000 - FAT Driver
#define MEMORY_FAT_ADDR ((void*) 0x00020000)
#define MEMORY_FAT_SIZE 0x00010000

// 0x00030000 - 0x00040000 - Kernel loading space
#define MEMORY_LOAD_KERNEL ((void*)0x30000)
#define MEMORY_LOAD_SIZE 0x00010000

// 0x00030000 - 0x00080000 - Free

#define MEMORY_MAX 0x00080000 // End of usable memory ("Low memory" part)

// 0x00080000 - 0x0009ffff - Extended BIOS data area
// 0x000a0000 - 0x000c7fff - Video memory (mapped)
// 0x000c8000 - 0x000fffff - BIOS

// ============= END OF LOW MEMORY =============
// - Below is only available in protected mode -

#define MEMORY_KERNEL_ADDR ((void*) 0x00100000)

#endif
