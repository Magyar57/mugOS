#include "Logging.h"
#include "Time/Time.h"
#include "CPU.h"
#include "Registers.h"

#include "Drivers/Timers/TSC.h"
#define MODULE "TSC"

struct TSC {
	struct SteadyTimer steadyTimer;
};

// TSC.asm
uint64_t TSC_read();

static struct TSC m_tsc = {
	.steadyTimer = {
		.name = "TSC",
		.score = 100,
		.read = TSC_read
	}
};

static uint64_t measureFrequency(){
	// Try to get the TSC's frequency
	// -> We do not trust cpuid leaf 0x15, as it is particularly unreliable
	// -> We could use leaf 0x16, but it has to be validated afterwards anyway
	// => So we just measure it once and for all. Simple, nice and easy

	uint64_t temp;
	uint64_t freq = UINT64_MAX;
	uint64_t begin, end;

	for (int i=0 ; i<10 ; i++){
		begin = TSC_read();
		mdelay(1);
		end = TSC_read();

		temp = (end - begin) * 1000;
		freq = min(freq, temp);
	}

	return (freq == UINT64_MAX) ? 0 : freq;
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

	uint64_t freq = measureFrequency();
	if (freq == 0){
		log(INFO, MODULE, "Couldn't measure TSC frequency");
		return;
	}

	m_tsc.steadyTimer.frequency = freq;
	m_tsc.steadyTimer.mask = 0xffffffffffffffff;

	// Enable the rdtsc/rdtscp instructions in usermode, since we have no reason not to.
	// We still provide higher level interfaces in Time.h
	union CR4 cr4;
	cr4.value = Registers_readCR4();
	cr4.bits.TSD = 0;
	Registers_writeCR4(cr4.value);

	Time_registerSteadyTimer(&m_tsc.steadyTimer);

	log(SUCCESS, MODULE, "Initalization success, frequency is %lu.%03lu MHz",
		freq / 1000000, freq % 1000000 / 1000);
}
