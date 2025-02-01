#ifndef __LIMINE_REQUESTS_H__
#define __LIMINE_REQUESTS_H__

// LimineRequests.h: limine boot protocol requests for the kernel startup
// See https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#define REQUESTED_LIMINE_REVISION 3

// ================== Make the requests ==================

volatile LIMINE_REQUESTS_START_MARKER;

// Use protocol version 3
volatile LIMINE_BASE_REVISION(REQUESTED_LIMINE_REVISION)

// Ask for Limine version
volatile struct limine_bootloader_info_request limineInfo = {
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
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

#endif
