#include <stddef.h>
#include "Logging.h"
#include "HAL/HAL.h"
#include "HAL/SMP/PerCPU.h"
#include "Arch/x86_64/CPU/CPU.h"
#include "Arch/x86_64/Platform/GDT.h"
#include "Arch/x86_64/Platform/IDT.h"
#include "Arch/x86_64/Platform/ISR.h"

void HAL_init(){
	CPU_init(&g_CPU);

	GDT_init();
	GDT_setTSS();
	IDT_init();
	ISR_init();
	PerCPU_wake();

	log(SUCCESS, "x86_64 HAL", "Initalization success");
}
