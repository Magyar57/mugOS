#ifndef __MEMDEFS_H__
#define __MEMDEFS_H__

// Memory scheme (from low to high addresses):

// 0x00000000 - 0x000003ff - IVT
// 0x00000400 - 0x000004ff - BIOS data area

#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

// 0x00000500 - 0x00010500 - FAT Driver
#define MEMORY_FAT_ADDR ((void far*) 0x00500000) // 0x00000500 (in C, pointers are as "segment:offset" "SSSS:OOOO")
#define MEMORY_FAT_SIZE 0x00010000

// 0x0002000 - 0x00030000 - Stage 2 bootloader

// 0x00030000 - 0x00080000 - Free

// 0x00080000 - 0x0009ffff - Extended BIOS data area
// 0x000a0000 - 0x000c7fff - Video memory (mapped)
// 0x000c8000 - 0x000fffff - BIOS

#endif
