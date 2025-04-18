#include <stddef.h>
#include <limine.h>

#include "Boot/LimineRequests.h"

volatile LIMINE_REQUESTS_START_MARKER;

// Use protocol version 3
volatile LIMINE_BASE_REVISION(3)

// Ask for Limine version
volatile struct limine_bootloader_info_request infoReq = {
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0,
	.response = NULL,
};

// Ask for the system's memory map
volatile struct limine_memmap_request memmapReq = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = NULL,
};

// Ask for the Higher Half Direct Map (get the virtual address offset)
volatile struct limine_hhdm_request hhdmReq = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = NULL,
};

// Ask for (a) framebuffer(s)
volatile struct limine_framebuffer_request framebufferReq = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = NULL
};

volatile LIMINE_REQUESTS_END_MARKER;
