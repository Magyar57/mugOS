#ifndef __PAGING_H__
#define __PAGING_H__

// Flags for the map method
#define PAGE_READ					0x00000000 // Protection flag: page is readable (default)
#define PAGE_WRITE					0x00000001 // Protection flag: page is writable
#define PAGE_EXEC					0x00000002 // Protection flag: page is executable
#define PAGE_KERNEL					0x00000000
#define PAGE_USER					0x00000004
#define PAGE_CACHE_DISABLED			0x00000008 // Map flag: cache is disabled
#define PAGE_CACHE_WRITETHROUGH		0x00000010 // Map flag: cache is write-through
#define PAGE_CACHE_WRITEBACK		0x00000000 // Map flag: cache is write-back (default)

void Paging_initialize();

#endif
