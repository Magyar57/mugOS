#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

typedef uintptr_t address_t;
typedef address_t physical_address_t;
typedef address_t virtual_address_t;

#define SIZE_4KB 0x00001000 // 4 KiB is 4 * 2^10 = 4*1024
#define SIZE_2MB 0x00200000 // 2 MiB is 2 * 2^20 = 2*1024*1024
#define SIZE_1GB 0x40000000 // 1 GiB is 1 * 2^30 = 1*1024*1024*1024

#define PAGE_SIZE				SIZE_4KB

#define PAGE_SHIFT				12
#define PAGE_MASK 				~(PAGE_SIZE-1)
#define getPage(addr)			((address_t)(addr) & PAGE_MASK)
#define getOffset(addr)			((address_t)(addr) & ~PAGE_MASK)
#define roundToPage(size)		(((size) + PAGE_SIZE-1) / PAGE_SIZE)

#define toCanonical(addr) \
	(((addr) & 0x0000800000000000) ? ((addr) | 0xffff000000000000) : ((addr) & 0x0000ffffffffffff))

#endif
