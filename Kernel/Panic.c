#include <stddef.h>
#include "Logging.h"
#include "HAL/CPU.h"

#include "Panic.h"

void panic(){
	log(PANIC, NULL, "Halting");
	haltAndCatchFire();
}
