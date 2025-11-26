#include "Logging.h"
#include "Time/Time.h"
#include "CPU/CPU.h"
#include "CPU/Registers.h"

#include "Drivers/Timers/TSC.h"
#define MODULE "TSC"

static struct SteadyTimer m_tsc = {
	.name = "TSC",
	.score = 100,
	.read = TSC_read
};

// Fallback frequency calibration method, measures the TSC with the help of other timers
static uint64_t measureFrequency(){
	// The calibration is two-fold: we measure using both the system's Event timer (mdelay: freq1)
	// and Steady timer (Time_get: freq2, t0, tf, dt)
	uint64_t freq_temp1, freq_temp2, freq1 = UINT64_MAX, freq2 = UINT64_MAX;
	uint64_t tscTicks_t0, tscTicks_tf, tscTicks_delta;
	ktime_t t0, tf, dt;

	for (int i=0 ; i<5 ; i++){
		t0 = Time_get();
		tscTicks_t0 = TSC_read();
		mdelay(50);
		tscTicks_tf = TSC_read();
		tf = Time_get();

		tscTicks_delta = tscTicks_tf - tscTicks_t0;
		dt = tf - t0;

		freq_temp1 = tscTicks_delta * (1000 / 50);
		freq_temp2 = tscTicks_delta * 1000000000 + dt-1 / dt;

		freq1 = min(freq1, freq_temp1);
		freq2 = min(freq2, freq_temp2);
	}

	uint64_t freq = min(freq1, freq2);
	return (freq == UINT64_MAX) ? 0 : freq;
}

// Try to get the TSC frequency from CPUID
static uint64_t findCpuidFrequency(){
	if (g_CPU.features.bits.TscClockRatioNumerator == 0)
		return 0;
	if (g_CPU.features.bits.TscClockRatioDenominator == 0)
		return 0;
	if (g_CPU.features.bits.TscCrystalClockFrequency == 0)
		return 0;

	// TSC frequency = TSC crystal clock frequency * TSC core crystal clock ratio
	uint64_t freq = g_CPU.features.bits.TscCrystalClockFrequency;
	freq *= g_CPU.features.bits.TscClockRatioNumerator;
	freq /= g_CPU.features.bits.TscClockRatioDenominator;

	return freq;
}

static uint64_t findFrequency(){
	uint64_t freq;

	// Prioritize getting the frequency from CPUID
	freq = findCpuidFrequency();
	if (freq != 0){
		log(INFO, MODULE, "TSC frequency provided by CPUID");
		return freq;
	}

	// Fallback: measure it
	log(INFO, MODULE, "Fell back to manual TSC frequency calibration");
	freq = measureFrequency();

	return freq;
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

	uint64_t freq = findFrequency();
	if (freq == 0){
		log(ERROR, MODULE, "Couldn't find the TSC's frequency");
		return;
	}

	m_tsc.frequency = freq;
	m_tsc.mask = 0xffffffffffffffff;

	// Enable the rdtsc/rdtscp instructions in usermode, since we have no reason not to.
	// We still provide higher level interfaces in Time.h
	union CR4 cr4;
	cr4.value = Registers_readCR4();
	cr4.bits.TSD = 0;
	Registers_writeCR4(cr4.value);

	Time_registerSteadyTimer(&m_tsc);

	log(SUCCESS, MODULE, "Initalization success, frequency is %lu.%03lu MHz",
		freq / 1000000, freq % 1000000 / 1000);
}

uint64_t TSC_getFrequency(){
	return m_tsc.frequency;
}
