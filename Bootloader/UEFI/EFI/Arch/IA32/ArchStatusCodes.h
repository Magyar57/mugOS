#ifndef __ARCH_STATUS_CODE_H__
#define __ARCH_STATUS_CODE_H__

// ArchStatusCodes.h: Architecture-specific status codes

#define ERR(code) (code | 0x80000000)

#endif
