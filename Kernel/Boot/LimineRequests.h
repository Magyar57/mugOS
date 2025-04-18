#ifndef __LIMINE_REQUESTS_H__
#define __LIMINE_REQUESTS_H__

#include <limine.h>

// LimineRequests.h: limine boot protocol requests for the kernel startup
// See https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md

extern volatile uint64_t limine_base_revision[3];

extern volatile struct limine_bootloader_info_request infoReq;
extern volatile struct limine_memmap_request memmapReq;
extern volatile struct limine_hhdm_request hhdmReq;
extern volatile struct limine_framebuffer_request framebufferReq;

#endif
