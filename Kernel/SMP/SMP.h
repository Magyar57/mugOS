#ifndef __SMP_H__
#define __SMP_H__

#include "HAL/SMP/PerCPU.h"
#include "HAL/SMP/ArchSMP.h"

void SMP_init();
void SMP_startCPUs();

#define SMP_getCpuId() PerCPU_getCpuId()

#endif
