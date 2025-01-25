#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "Logging.h"

#include "CPU.h"

extern void cpuidWrapper(unsigned int code, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx); // CPU.asm

// CPU_identify usage example:
// CPU cpu;
// CPU_identify(&cpu);
// log(INFO, "MAIN", "cpu vendor: '%s'", cpu.vendor);
// log(INFO, "MAIN", "cpu family=%d model=%d type=%d stepping=%d", cpu.family, cpu.model, cpu.type, cpu.stepping);
// log(INFO, "MAIN", "cpu brandIndex=%d cflushLineSize=%d maxAddressableCpuIds=%d", cpu.brandIndex, cpu.cflushLineSize, cpu.maxAddressableCpuIds);
// log(INFO, "MAIN", "cpu features: %p %p", cpu.features0, cpu.features1);

bool CPU_identify(CPU* cpu){
	uint32_t eax, ebx, ecx, edx;

	if (!CPU_supportsCpuid()) return false;

	// Basic CPUID Informations
	uint32_t maxReturnValue;

	// cpuid.eax = 0x00: max input value for basic cpuid informations, and vendor string
	cpuidWrapper(0, &maxReturnValue, &ebx, &ecx, &edx);

	memcpy(cpu->vendor, &ebx, 4);
	memcpy(cpu->vendor+4, &edx, 4);
	memcpy(cpu->vendor+8, &ecx, 4);
	cpu->vendor[12] = '\0';

	switch (maxReturnValue){
		default:
		case 4:
		case 3:
		case 2:
			// 2: Cache and TLB informations
			// Not implemented
			// cpuidWrapper(2, &eax, &ebx, &ecx, &edx);
		case 1:
			// 1: version information
			cpuidWrapper(1, &eax, &ebx, &ecx, &edx);
			// EAX
			cpu->stepping = (eax & 0x0000000f);
			cpu->model = (eax & 0x000000f0) >> 4;
			cpu->family = (eax & 0x00000f00) >> 8;
			if ((cpu->family == 0x6) || (cpu->family == 0xf)){
				cpu->model += ((eax & 0x000f0000) >> 12);
			}
			if (cpu->family == 0xf){
				cpu->family += (eax & 0x0ff00000 >> 20);
			}
			cpu->type = (eax & 0x00003000) >> 12;
			// EBX
			cpu->brandIndex = (ebx & 0x000000ff);
			cpu->cflushLineSize = (ebx & 0x0000ff00) >> 8;
			cpu->maxAddressableCpuIds = (ebx & 0x00ff0000) >> 16;
			// cpu->localCpuApicId = (ebx & 0xff000000) >> 24; // current CPU specific
			// EDX & EXC
			cpu->features0 = edx;
			cpu->features1 = ecx;
		case 0:
			break;
	}

	// Extended CPUID informations
	uint32_t maxExtendedInformationValue;

	// cpuid.eax = 0x80000000: max input value for extended cpuid informations
	cpuidWrapper(0x80000000, &maxExtendedInformationValue, &ebx, &ecx, &edx);

	// Unimplemented
	switch (maxExtendedInformationValue){
		default:
		case 0x80000008:
		case 0x80000007:
		case 0x80000006:
		case 0x80000005:
		case 0x80000004:
		case 0x80000003:
		case 0x80000002:
		case 0x80000001:
		case 0x80000000:
			break;
	}

	return true;
}
