#include <stddef.h>
#include "Logging.h"
#include "IRQ.h"
#include "HAL/HAL.h"
#include "HAL/SMP/PerCPU.h"
#include "Arch/x86_64/CPU.h"
#include "Arch/x86_64/GDT.h"
#include "Arch/x86_64/IDT.h"
#include "Arch/x86_64/ISR.h"

void HAL_init(){
	IRQ_disable();

	CPU_init(&g_CPU);

	GDT_init();
	GDT_setTSS();
	IDT_init();
	ISR_init();
	PerCPU_wake();

	log(SUCCESS, "x86_64 HAL", "Initalization success");
}
