#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

#define SIZE_4KB 0x00001000 // 4 KiB is 4 * 2^10 = 4*1024
#define SIZE_2MB 0x00200000 // 2 MiB is 2 * 2^20 = 2*1024*1024
#define SIZE_1GB 0x40000000 // 1 GiB is 1 * 2^30 = 1*1024*1024*1024

#define PAGE_SIZE		SIZE_4KB
#define PAGE_MASK 		~(PAGE_SIZE-1)
#define getPage(addr)	(addr & ~(PAGE_SIZE-1))

#define toCanonical(addr) \
	(addr & 0x0000800000000000) ? (addr | 0xffff000000000000) : (addr & 0x0000ffffffffffff)

typedef uint64_t physical_address_t;
typedef uint64_t virtual_address_t;

#endif
