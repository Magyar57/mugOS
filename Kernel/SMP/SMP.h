#ifndef __SMP_H__
#define __SMP_H__

#include "HAL/SMP/ArchSMP.h"

void SMP_init();
void SMP_startCPUs();

#define SMP_getCpuId() ArchSMP_getCpuId()

#endif
