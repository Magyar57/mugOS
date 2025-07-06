#include <stddef.h>
#include "Logging.h"
#include "HAL/Halt.h"

#include "Panic.h"

void panic(){
	log(PANIC, NULL, "Halting");
	haltAndCatchFire();
}

void panicForMissingFeature(const char* feature){
	log(PANIC, NULL, "mugOS requires the '%s' feature, which this CPU does not support !", feature);
	panic();
}
