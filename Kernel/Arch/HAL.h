#ifndef __HAL_H__
#define __HAL_H__

// HAL.h:
// Defines the Hardware Abstraction Layer (HAL),
// and implements it depending on the requested architecture ARCH_*

#if !defined(ARCH_x86) && !defined(ARCH_arm64)
#error "Invalid architecture provided ! Please define a valid architecture maccro from the supported list."
#endif

void HAL_initialize();

#endif
