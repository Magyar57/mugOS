#ifndef __HAL_H__
#define __HAL_H__

// HAL.h: Hardware Abstraction Layer, contains the generic functions

#if !defined(x86_64) && !defined(arm64)
#error "Invalid architecture provided ! Please define a valid architecture maccro from the supported list."
#endif

void HAL_initialize();

#endif
