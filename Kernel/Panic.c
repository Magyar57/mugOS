#include <stddef.h>
#include "string.h"
#include "Logging.h"
#include "HAL/Halt.h"

#include "Panic.h"

[[noreturn]]
void panic(){
	log(PANIC, NULL, "Halting");
	haltAndCatchFire();
	unreachable();
}

void panicOnError(const char* errorStr, int errno){
	if (errno == 0)
		return;

	log(PANIC, NULL, "%s: %s", errorStr, strerror(errno));
	panic();
}

void panicForMissingFeature(const char* feature){
	log(PANIC, NULL, "mugOS requires the '%s' feature, which this CPU does not support !", feature);
	panic();
}
