#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "Logging.h"

#include "CPU.h"

extern void cpuidWrapper(unsigned int code, uint32_t* rax, uint32_t* rbx, uint32_t* rcx, uint32_t* rdx); // CPU.asm

// CPU_identify usage example:
// CPU cpu;
// CPU_identify(&cpu);
// log(INFO, "MAIN", "cpu vendor: '%s'", cpu.vendor);
// log(INFO, "MAIN", "cpu family=%d model=%d type=%d stepping=%d", cpu.family, cpu.model, cpu.type, cpu.stepping);
// log(INFO, "MAIN", "cpu brandIndex=%d cflushLineSize=%d maxAddressableCpuIds=%d", cpu.brandIndex, cpu.cflushLineSize, cpu.maxAddressableCpuIds);
// log(INFO, "MAIN", "cpu features: %p %p", cpu.features0, cpu.features1);

bool CPU_identify(CPU* cpu){
	uint64_t rax, rbx, rcx, rdx;

	if (!CPU_supportsCpuid()) return false;

	// Basic CPUID Informations
	uint64_t maxReturnValue;

	// cpuid.rax = 0x00: max input value for basic cpuid informations, and vendor string
	cpuidWrapper(0, &maxReturnValue, &rbx, &rcx, &rdx);

	// The CPU vendor string is contained in rbx, rdx and rcx ; copy it into the structure
	memcpy(cpu->vendor, &rbx, 4);
	memcpy(cpu->vendor+4, &rdx, 4);
	memcpy(cpu->vendor+8, &rcx, 4);
	cpu->vendor[12] = '\0';

	switch (maxReturnValue){
		default:
		case 4:
		case 3:
		case 2:
			// 2: Cache and TLB informations
			// Not implemented
			// cpuidWrapper(2, &rax, &rbx, &rcx, &rdx);
		case 1:
			// 1: version information
			cpuidWrapper(1, &rax, &rbx, &rcx, &rdx);
			// EAX
			cpu->stepping = (rax & 0x0000000f);
			cpu->model = (rax & 0x000000f0) >> 4;
			cpu->family = (rax & 0x00000f00) >> 8;
			if ((cpu->family == 0x6) || (cpu->family == 0xf)){
				cpu->model += ((rax & 0x000f0000) >> 12);
			}
			if (cpu->family == 0xf){
				cpu->family += (rax & 0x0ff00000 >> 20);
			}
			cpu->type = (rax & 0x00003000) >> 12;
			// EBX
			cpu->brandIndex = (rbx & 0x000000ff);
			cpu->cflushLineSize = (rbx & 0x0000ff00) >> 8;
			cpu->maxAddressableCpuIds = (rbx & 0x00ff0000) >> 16;
			// cpu->localCpuApicId = (rbx & 0xff000000) >> 24; // current CPU specific
			// EDX & EXC
			cpu->features0 = rdx;
			cpu->features1 = rcx;
		case 0:
			break;
	}

	// Extended CPUID informations
	uint32_t maxExtendedInformationValue;

	// cpuid.rax = 0x80000000: max input value for extended cpuid informations
	cpuidWrapper(0x80000000, &maxExtendedInformationValue, &rbx, &rcx, &rdx);

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
