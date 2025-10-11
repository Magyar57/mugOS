#include "Logging.h"
#include "Registers.h"

#include "Drivers/Timers/TSC.h"
#define MODULE "TSC"

// TSC.asm
uint64_t readTSC();

void TSC_init(struct SteadyTimer* tsc){
	tsc->name = "TSC";

	// Enable the rdtsc/rdtscp instructions in usermode, since we have no reason not to.
	// We still provide higher level interfaces in Time.h
	union CR4 cr4;
	cr4.value = Registers_readCR4();
	cr4.bits.TSD = 0;
	Registers_writeCR4(cr4.value);

	// Now we need to measure the time which is reprensented by one tick of the TSC
	// TODO
	uint64_t cur_value = readTSC();

	log(SUCCESS, MODULE, "Initalization (stub) success, current value: %p", cur_value);
}
