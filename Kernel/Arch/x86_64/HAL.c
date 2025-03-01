#include <stddef.h>
#include "Logging.h"
#include "IRQ.h"

#include "HAL/HAL.h"

// ================ x86_64 ================
#ifdef x86_64

#include "Arch/x86_64/GDT.h"
#include "Arch/x86_64/IDT.h"
#include "Arch/x86_64/ISR.h"
#include "Arch/x86_64/IRQ.h"

void HAL_initialize(){
	IRQ_disable();

	GDT_initialize();
	GDT_setTSS();
	IDT_initialize();
	ISR_initialize();
	IRQ_initialize();

	IRQ_enable();
	log(SUCCESS, "x86_64 HAL", "Initalization success");
}

#endif // x86_64

// ================ arm64 ================
#ifdef arm64

// Example, not yet implemented
// void HAL_Initialize(){}

#endif // arm64
