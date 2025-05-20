#include <stddef.h>
#include "Logging.h"
#include "IRQ.h"
#include "HAL/HAL.h"
#include "HAL/CPU.h"
#include "Arch/x86_64/GDT.h"
#include "Arch/x86_64/IDT.h"
#include "Arch/x86_64/ISR.h"

void HAL_initialize(){
	IRQ_disable();

	CPU_initialize(&g_CPU);

	GDT_initialize();
	GDT_setTSS();
	IDT_initialize();
	ISR_initialize();

	log(SUCCESS, "x86_64 HAL", "Initalization success");
}
