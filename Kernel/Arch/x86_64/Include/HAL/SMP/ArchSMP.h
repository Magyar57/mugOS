#ifndef __ARCH_SMP_H__
#define __ARCH_SMP_H__

#include <stddef.h>
#include <stdint.h>

extern int g_nCPUs;

void ArchSMP_init();
void ArchSMP_startCPUs();

#endif
