#include "Logging.h"
#include "Time/Time.h"
#include "CPU.h"
#include "Registers.h"

#include "Drivers/Timers/TSC.h"
#define MODULE "TSC"

struct TSC {
	struct SteadyTimer steadyTimer;
};

static struct TSC m_tsc = {
	.steadyTimer = {
		.name = "TSC",
		.score = -100 // this is still a stub
	}
};

// TSC.asm
uint64_t readTSC();

static uint64_t getTicksPerMs(){
	uint64_t begin = readTSC();
	udelay(500);
	uint64_t end = readTSC();

	return (end - begin) / 500;
}

// sigma² = 1/n sum(i=0,n) (xi - mean)²
uint64_t getVariance(uint64_t* arr, size_t size, uint64_t mean){
	uint64_t temp, sigma = 0;

	for (size_t i=0 ; i<size ; i++){
		temp = arr[i] - mean;
		debug("%ld", temp);
		sigma += temp*temp;
	}

	return sigma / size;
}

void TSC_init(){
	if (!g_CPU.features.bits.TSC){
		log(INFO, MODULE, "Chip is not present");
		return;
	}
	if (!g_CPU.extFeatures.bits.InvariantTSC){
		log(INFO, MODULE, "Chip is present but not invariant, which is unsupported");
		return;
	}

	// Enable the rdtsc/rdtscp instructions in usermode, since we have no reason not to.
	// We still provide higher level interfaces in Time.h
	union CR4 cr4;
	cr4.value = Registers_readCR4();
	cr4.bits.TSD = 0;
	Registers_writeCR4(cr4.value);

	// Now we need to measure the time which is reprensented by one tick of the TSC
	// TODO

	// uint64_t values[10];
	// uint64_t mean = 0;
	// for (int i=0 ; i<10 ; i++){
	// 	values[i] = getTicksPerMs();
	// 	mean += values[i];
	// 	debug("got value %ld", values[i]);
	// }
	// mean /= 10;
	// debug("mean: %ld variance=%ld", mean, getVariance(values, 10, mean));

	Time_registerSteadyTimer(&m_tsc.steadyTimer);

	log(SUCCESS, MODULE, "Initalization (stub) success");
}
