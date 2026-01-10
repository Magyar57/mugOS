#include <stdint.h>
#include "string.h"
#include "Logging.h"
#include "Panic.h"
#include "Registers.h"

#include "CPU.h"
#define MODULE "CPU"

struct CPU g_CPU;

bool supportsCpuid();
void cpuidWrapper(int leaf, uint32_t* eaxOut, uint32_t* ebxOut, uint32_t* ecxOut, uint32_t* edxOut);
void cpuidWrapperWithSubleaf(int leaf, int subleaf,
	uint32_t* eaxOut, uint32_t* ebxOut, uint32_t* ecxOut, uint32_t* edxOut);

// CPUID.EAX = 0x00: Max input value for basic cpuid informations & vendor string
static void parseCpuid_0x00(struct CPU* cpu){
	uint32_t ebx, ecx, edx;
	cpuidWrapper(0x00, (uint32_t*) &cpu->maxInformation, &ebx, &ecx, &edx);

	// The CPU vendor string is contained in ebx, edx and ecx ; copy it into the structure
	memcpy(cpu->vendorStr, &ebx, 4);
	memcpy(cpu->vendorStr+4, &edx, 4);
	memcpy(cpu->vendorStr+8, &ecx, 4);
	cpu->vendorStr[12] = '\0';

	if (strncmp(cpu->vendorStr, "GenuineIntel", 13) == 0)
		cpu->vendor = VENDOR_INTEL;
	else if (strncmp(cpu->vendorStr, "AuthenticAMD", 13) == 0)
		cpu->vendor = VENDOR_AMD;
	else
		cpu->vendor = VENDOR_UNSUPPORTED;
}

// CPUID.EAX = 0x01: Version informations
static void parseCpuid_0x01(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;
	cpuidWrapper(0x01, &eax, &ebx, &ecx, &edx);

	// EAX
	cpu->stepping = (eax & 0x0000000f);
	cpu->model = (eax & 0x000000f0) >> 4;
	cpu->family = (eax & 0x00000f00) >> 8;
	if ((cpu->family == 0x6) || (cpu->family == 0xf)){
		cpu->model += ((eax & 0x000f0000) >> 12); // extended model ID
	}
	if (cpu->family == 0xf){
		cpu->family += (eax & 0x0ff00000 >> 20); // extended family ID
	}
	cpu->type = (eax & 0x00003000) >> 12;
	// EBX
	cpu->brandIndex = (ebx & 0x000000ff);
	cpu->cflushLineSize = (ebx & 0x0000ff00) >> 8;
	cpu->maxAddressableCpuIds = (ebx & 0x00ff0000) >> 16;
	// EDX & EXC
	cpu->features.leaves.leaf_0x01.ecx = ecx;
	cpu->features.leaves.leaf_0x01.edx = edx;
}

// CPUID.EAX = 0x07: Structured extended feature flags enumeration
static void parseCpuid_0x07(struct CPU* cpu){
	uint32_t maxSubLeaves;
	uint32_t eax, ebx, ecx, edx;
	cpuidWrapper(0x07, &maxSubLeaves, &ebx, &ecx, &edx);
	cpu->features.leaves.leaf_0x07_0.ebx = ebx;
	cpu->features.leaves.leaf_0x07_0.ecx = ecx;
	cpu->features.leaves.leaf_0x07_0.edx = edx;

	switch (maxSubLeaves){
	default:
	case 2:
		cpuidWrapperWithSubleaf(0x07, 2, &eax, &ebx, &ecx, &edx);
		cpu->features.leaves.leaf_0x07_2.eax = eax;
		cpu->features.leaves.leaf_0x07_2.ebx = ebx;
		cpu->features.leaves.leaf_0x07_2.ecx = ecx;
		cpu->features.leaves.leaf_0x07_2.edx = edx;
		// Fall through
	case 1:
		cpuidWrapperWithSubleaf(0x07, 1, &eax, &ebx, &ecx, &edx);
		cpu->features.leaves.leaf_0x07_1.eax = eax;
		cpu->features.leaves.leaf_0x07_1.ebx = ebx;
		cpu->features.leaves.leaf_0x07_1.ecx = ecx;
		cpu->features.leaves.leaf_0x07_1.edx = edx;
		// Fall through
	case 0:
		// Done already
	}
}

// CPUID.EAX = 0x15: Time Stamp Counter & Nominal Core Crystal Clock information
static void parseCpuid_0x15(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;
	cpuidWrapper(0x15, &eax, &ebx, &ecx, &edx);
	cpu->features.leaves.leaf_0x15.eax = eax;
	cpu->features.leaves.leaf_0x15.ebx = ebx;
	cpu->features.leaves.leaf_0x15.ecx = ecx;
	cpu->features.leaves.leaf_0x15.edx = edx;
}

// CPUID.EAX = 0x16: Processor Frequency Information
static void parseCpuid_0x16(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;
	cpuidWrapper(0x16, &eax, &ebx, &ecx, &edx);
	cpu->features.leaves.leaf_0x16.eax = eax;
	cpu->features.leaves.leaf_0x16.ebx = ebx;
	cpu->features.leaves.leaf_0x16.ecx = ecx;
	cpu->features.leaves.leaf_0x16.edx = edx;
}

// CPUID.EAX = 0x80000000: Max input value for extended cpuid informations
static void parseCpuid_0x80000000(struct CPU* cpu){
	uint32_t ebx, ecx, edx;

	cpuidWrapper(0x80000000, (uint32_t*) &cpu->maxExtendedInformation, &ebx, &ecx, &edx);
}

// CPUID.EAX = 0x80000001: Extended flags
static void parseCpuid_0x80000001(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;

	cpuidWrapper(0x80000001, &eax, &ebx, &ecx, &edx);
	cpu->extFeatures.leaves.leaf_0x80000001.eax = eax;
	cpu->extFeatures.leaves.leaf_0x80000001.ebx = ebx;
	cpu->extFeatures.leaves.leaf_0x80000001.ecx = ecx;
	cpu->extFeatures.leaves.leaf_0x80000001.edx = edx;
}

// CPUID.EAX = 0x80000004 to 0x80000002: processor brand string
static void parseCpuid_0x80000002to0x80000004(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;

	for (int i=0 ; i<3 ; i++){
		cpuidWrapper(0x80000002 + i, &eax, &ebx, &ecx, &edx);
		memcpy(cpu->brand + 16*i +  0, &eax, 4);
		memcpy(cpu->brand + 16*i +  4, &ebx, 4);
		memcpy(cpu->brand + 16*i +  8, &ecx, 4);
		memcpy(cpu->brand + 16*i + 12, &edx, 4);
	}
	cpu->brand[48] = '\0';
}

// CPUID.EAX = 0x80000006: Extended cache information
static void parseCpuid_0x80000006(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;

	cpuidWrapper(0x80000006, &eax, &ebx, &ecx, &edx);
	cpu->extFeatures.leaves.leaf_0x80000006.eax = eax;
	cpu->extFeatures.leaves.leaf_0x80000006.ebx = ebx;
	cpu->extFeatures.leaves.leaf_0x80000006.ecx = ecx;
	cpu->extFeatures.leaves.leaf_0x80000006.edx = edx;
}

// CPUID.EAX = 0x80000007: InvariantTSC
static void parseCpuid_0x80000007(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;

	cpuidWrapper(0x80000007, &eax, &ebx, &ecx, &edx);
	cpu->extFeatures.leaves.leaf_0x80000007.eax = eax;
	cpu->extFeatures.leaves.leaf_0x80000007.ebx = ebx;
	cpu->extFeatures.leaves.leaf_0x80000007.ecx = ecx;
	cpu->extFeatures.leaves.leaf_0x80000007.edx = edx;
}

// CPUID.EAX = 0x80000008: More extended flags
static void parseCpuid_0x80000008(struct CPU* cpu){
	uint32_t eax, ebx, ecx, edx;

	cpuidWrapper(0x80000008, &eax, &ebx, &ecx, &edx);
	cpu->extFeatures.leaves.leaf_0x80000008.eax = eax;
	cpu->extFeatures.leaves.leaf_0x80000008.ebx = ebx;
	cpu->extFeatures.leaves.leaf_0x80000008.ecx = ecx;
	cpu->extFeatures.leaves.leaf_0x80000008.edx = edx;
}

void CPU_init(struct CPU* cpu){
	if (!supportsCpuid())
		panicForMissingFeature("CPUID instruction");

	// Setup CR0 features
	union CR0 cr0;
	cr0.value = Registers_readCR0();
	cr0.bits.CD = 0; // enable cache
	cr0.bits.NW = 0; // disable write-through
	cr0.bits.WP = 1; // inhibits kernel writes to read-only pages
	Registers_writeCR0(cr0.value);

	parseCpuid_0x00(cpu);
	if (cpu->vendor == VENDOR_UNSUPPORTED){
		log(PANIC, MODULE, "Unsupported CPU vendor '%s'", cpu->vendorStr);
		panic();
	}

	parseCpuid_0x01(cpu);
	if (!cpu->features.bits.MSR_RDMSR)
		panicForMissingFeature("RDMSR and WRMSR instructions");

	// Leaves above 1 are enabled if both:
	// - maxInformation > 1
	// - on Intel manufactured CPUs, in the IA32_MISC_ENABLE MSR,
	//   LimitCPUIDMaxval (bit) is 0 by default
	bool leaf_condition_met = (cpu->maxInformation >= 7);
	if (cpu->vendor == VENDOR_INTEL){
		union MSR_IA32_MISC_ENABLE misc_enable;
		misc_enable.value = Registers_readMSR(MSR_ADDR_IA32_MISC_ENABLE);
		leaf_condition_met = leaf_condition_met && !misc_enable.bits.LimitCPUIDMaxval;
		if (misc_enable.bits.LimitCPUIDMaxval)
			panicForMissingFeature("RDMSR and WRMSR instructions");
	}
	if (!leaf_condition_met)
		panicForMissingFeature("CPUID leaves >= 7");

	switch (cpu->maxInformation){
	default:
	case 0x16:
		// CPU Frequencies
		parseCpuid_0x16(cpu);
		// Fall through
	case 0x15:
		// TSC informations
		parseCpuid_0x15(cpu);
		// Fall through
	case 0x14:
	case 0x13:
	case 0x12:
	case 0x11:
	case 0x10:
	case 0x0f:
	case 0x0e:
	case 0x0d:
	case 0x0c:
	case 0x0b:
	case 0x0a:
	case 0x09:
	case 0x08:
	case 0x07:
		// Structured extended feature flags enumeration
		parseCpuid_0x07(cpu);
		// Fall through
	case 0x06:
		// Thermal and power management
	case 0x05:
		// MONITOR/MWAIT
	case 0x04:
		// Deterministic cache parameters
	case 0x03:
		// Serial number (valid on pentium 3 only, which we don't support)
	case 0x02:
		// Cache and TLB informations
	case 0x01:
		// Version informations (done already)
	case 0x00:
		// Max basic input value & vendor string (done already)
		break;
	}

	parseCpuid_0x80000000(cpu);

	switch (cpu->maxExtendedInformation){
	default:
	case 0x80000008:
		// More extended flags
		parseCpuid_0x80000008(cpu);
		// Fall through
	case 0x80000007:
		// InvariantTSC
		parseCpuid_0x80000007(cpu);
		// Fall through
	case 0x80000006:
		// Extended cache information
		parseCpuid_0x80000006(cpu);
		// Fall through
	case 0x80000005:
		// Reserved
	case 0x80000004:
		// From 0x80000004 to 0x80000002: processor brand string
		parseCpuid_0x80000002to0x80000004(cpu);
		// Fall through
	case 0x80000003:
	case 0x80000002:
	case 0x80000001:
		// Extended flags
		parseCpuid_0x80000001(cpu);
		// Fall through
	case 0x80000000:
		// Max extended input value (done already)
		break;
	}
}

void CPU_print(struct CPU* cpu){
	if (!cpu) return;

	log(INFO, MODULE, "CPU vendor: '%s'", cpu->vendorStr);
	log(INFO, MODULE, "CPU family=%d model=%d type=%d stepping=%d",
		cpu->family, cpu->model, cpu->type, cpu->stepping);
	log(INFO, MODULE, "CPU brandIndex=%d cflushLineSize=%d maxAddressableCpuIds=%d",
		cpu->brandIndex, cpu->cflushLineSize, cpu->maxAddressableCpuIds);

	log(INFO, MODULE, "CPU brand: '%s'", cpu->brand);
	log(INFO, MODULE, "Address sizes: %d bits physical, %d bits virtual",
		cpu->extFeatures.bits.NumberOfPhysicalAddressBits,
		cpu->extFeatures.bits.NumberOfLinearAddressBits);
	log(INFO, MODULE, "Cache size: %d KB (L2 associativity: %d, line size: %d bytes)",
		cpu->extFeatures.bits.CacheSize, cpu->extFeatures.bits.L2Associativity,
		cpu->extFeatures.bits.CacheLineSize);

	log(INFO, MODULE, "CPUID max values: basic=%#hhx extended=%#hhx",
		cpu->maxInformation, cpu->maxExtendedInformation);

	const char* SSE3 = (cpu->features.bits.SSE3) ? "SSE3 " : "";
	const char* PCLMULQDQ = (cpu->features.bits.PCLMULQDQ) ? "PCLMULQDQ " : "";
	const char* DTES64 = (cpu->features.bits.DTES64) ? "DTES64 " : "";
	const char* MONITOR = (cpu->features.bits.MONITOR) ? "MONITOR " : "";
	const char* DS_CPL = (cpu->features.bits.DS_CPL) ? "DS_CPL " : "";
	const char* VMX = (cpu->features.bits.VMX) ? "VMX " : "";
	const char* SMX = (cpu->features.bits.SMX) ? "SMX " : "";
	const char* EIST = (cpu->features.bits.EIST) ? "EIST " : "";
	const char* TM2 = (cpu->features.bits.TM2) ? "TM2 " : "";
	const char* SSSE3 = (cpu->features.bits.SSSE3) ? "SSSE3 " : "";
	const char* CNXT_ID = (cpu->features.bits.CNXT_ID) ? "CNXT_ID " : "";
	const char* SDBG = (cpu->features.bits.SDBG) ? "SDBG " : "";
	const char* FMA = (cpu->features.bits.FMA) ? "FMA " : "";
	const char* CMPXCHG16B = (cpu->features.bits.CMPXCHG16B) ? "CMPXCHG16B " : "";
	const char* xTPR = (cpu->features.bits.xTPR) ? "xTPR " : "";
	const char* PDCM = (cpu->features.bits.PDCM) ? "PDCM " : "";
	const char* PCID = (cpu->features.bits.PCID) ? "PCID " : "";
	const char* DCA = (cpu->features.bits.DCA) ? "DCA " : "";
	const char* SSE4_1 = (cpu->features.bits.SSE4_1) ? "SSE4_1 " : "";
	const char* SSE4_2 = (cpu->features.bits.SSE4_2) ? "SSE4_2 " : "";
	const char* x2APIC = (cpu->features.bits.x2APIC) ? "x2APIC " : "";
	const char* MOVBE = (cpu->features.bits.MOVBE) ? "MOVBE " : "";
	const char* POPCNT = (cpu->features.bits.POPCNT) ? "POPCNT " : "";
	const char* TSC_Deadline = (cpu->features.bits.TSC_Deadline) ? "TSC_Deadline " : "";
	const char* AES = (cpu->features.bits.AES) ? "AES " : "";
	const char* XSAVE = (cpu->features.bits.XSAVE) ? "XSAVE " : "";
	const char* OSXSAVE = (cpu->features.bits.OSXSAVE) ? "OSXSAVE " : "";
	const char* AVX = (cpu->features.bits.AVX) ? "AVX " : "";
	const char* F16C = (cpu->features.bits.F16C) ? "F16C " : "";
	const char* RDRAND = (cpu->features.bits.RDRAND) ? "RDRAND " : "";

	const char* FPU = (cpu->features.bits.FPU) ? "FPU " : "";
	const char* VME = (cpu->features.bits.VME) ? "VME " : "";
	const char* DE = (cpu->features.bits.DE) ? "DE " : "";
	const char* PSE = (cpu->features.bits.PSE) ? "PSE " : "";
	const char* TSC = (cpu->features.bits.TSC) ? "TSC " : "";
	const char* MSR_RDMSR = (cpu->features.bits.MSR_RDMSR) ? "MSR_RDMSR " : "";
	const char* PAE = (cpu->features.bits.PAE) ? "PAE " : "";
	const char* MCE = (cpu->features.bits.MCE) ? "MCE " : "";
	const char* CX8 = (cpu->features.bits.CX8) ? "CX8 " : "";
	const char* APIC = (cpu->features.bits.APIC) ? "APIC " : "";
	const char* SEP = (cpu->features.bits.SEP) ? "SEP " : "";
	const char* MTRR = (cpu->features.bits.MTRR) ? "MTRR " : "";
	const char* PGE = (cpu->features.bits.PGE) ? "PGE " : "";
	const char* MCA = (cpu->features.bits.MCA) ? "MCA " : "";
	const char* CMOV = (cpu->features.bits.CMOV) ? "CMOV " : "";
	const char* PAT = (cpu->features.bits.PAT) ? "PAT " : "";
	const char* PSE_36 = (cpu->features.bits.PSE_36) ? "PSE_36 " : "";
	const char* PSN = (cpu->features.bits.PSN) ? "PSN " : "";
	const char* CLFSH = (cpu->features.bits.CLFSH) ? "CLFSH " : "";
	const char* DS = (cpu->features.bits.DS) ? "DS " : "";
	const char* ACPI = (cpu->features.bits.ACPI) ? "ACPI " : "";
	const char* MMX = (cpu->features.bits.MMX) ? "MMX " : "";
	const char* FXSR = (cpu->features.bits.FXSR) ? "FXSR " : "";
	const char* SSE = (cpu->features.bits.SSE) ? "SSE " : "";
	const char* SSE2 = (cpu->features.bits.SSE2) ? "SSE2 " : "";
	const char* SS = (cpu->features.bits.SS) ? "SS " : "";
	const char* HTT = (cpu->features.bits.HTT) ? "HTT " : "";
	const char* TM = (cpu->features.bits.TM) ? "TM " : "";
	const char* PBE = (cpu->features.bits.PBE) ? "PBE " : "";

	const char* FSGSBASE = (cpu->features.bits.FSGSBASE) ? "FSGSBASE " : "";
	const char* IA32_TSC_ADJUST = (cpu->features.bits.IA32_TSC_ADJUST) ? "IA32_TSC_ADJUST " : "";
	const char* SGX = (cpu->features.bits.SGX) ? "SGX " : "";
	const char* BMI1 = (cpu->features.bits.BMI1) ? "BMI1 " : "";
	const char* HLE = (cpu->features.bits.HLE) ? "HLE " : "";
	const char* AVX2 = (cpu->features.bits.AVX2) ? "AVX2 " : "";
	const char* FDP_EXCPTN_ONLY = (cpu->features.bits.FDP_EXCPTN_ONLY) ? "FDP_EXCPTN_ONLY " : "";
	const char* SMEP = (cpu->features.bits.SMEP) ? "SMEP " : "";
	const char* BMI2 = (cpu->features.bits.BMI2) ? "BMI2 " : "";
	const char* Enhanced_MOVSB_STOSB = (cpu->features.bits.Enhanced_MOVSB_STOSB) ?
		"Enhanced_MOVSB_STOSB " : "";
	const char* INVPCID = (cpu->features.bits.INVPCID) ? "INVPCID " : "";
	const char* RTM = (cpu->features.bits.RTM) ? "RTM " : "";
	const char* RDT_M = (cpu->features.bits.RDT_M) ? "RDT_M " : "";
	const char* Deprecated_FPU_CS_DS = (cpu->features.bits.Deprecated_FPU_CS_DS) ?
		"Deprecated_FPU_CS_DS " : "";
	const char* MPX = (cpu->features.bits.MPX) ? "MPX " : "";
	const char* RDT_A = (cpu->features.bits.RDT_A) ? "RDT_A " : "";
	const char* AVX512F = (cpu->features.bits.AVX512F) ? "AVX512F " : "";
	const char* AVX512DQ = (cpu->features.bits.AVX512DQ) ? "AVX512DQ " : "";
	const char* RDSEED = (cpu->features.bits.RDSEED) ? "RDSEED " : "";
	const char* ADX = (cpu->features.bits.ADX) ? "ADX " : "";
	const char* SMAP = (cpu->features.bits.SMAP) ? "SMAP " : "";
	const char* AVX512_IFMA = (cpu->features.bits.AVX512_IFMA) ? "AVX512_IFMA " : "";
	const char* CLFLUSHOPT = (cpu->features.bits.CLFLUSHOPT) ? "CLFLUSHOPT " : "";
	const char* CLWB = (cpu->features.bits.CLWB) ? "CLWB " : "";
	const char* IntelProcessorTrace = (cpu->features.bits.IntelProcessorTrace) ?
		"IntelProcessorTrace " : "";
	const char* AVX512PF = (cpu->features.bits.AVX512PF) ? "AVX512PF " : "";
	const char* AVX512ER = (cpu->features.bits.AVX512ER) ? "AVX512ER " : "";
	const char* AVX512CD = (cpu->features.bits.AVX512CD) ? "AVX512CD " : "";
	const char* SHA = (cpu->features.bits.SHA) ? "SHA " : "";
	const char* AVX512BW = (cpu->features.bits.AVX512BW) ? "AVX512BW " : "";
	const char* AVX512VL = (cpu->features.bits.AVX512VL) ? "AVX512VL " : "";

	const char* PREFETCHWT1 = (cpu->features.bits.PREFETCHWT1) ? "PREFETCHWT1 " : "";
	const char* AVX512_VBMI = (cpu->features.bits.AVX512_VBMI) ? "AVX512_VBMI " : "";
	const char* UMIP = (cpu->features.bits.UMIP) ? "UMIP " : "";
	const char* PKU = (cpu->features.bits.PKU) ? "PKU " : "";
	const char* OSPKE = (cpu->features.bits.OSPKE) ? "OSPKE " : "";
	const char* WAITPKG = (cpu->features.bits.WAITPKG) ? "WAITPKG " : "";
	const char* AVX512_VBMI2 = (cpu->features.bits.AVX512_VBMI2) ? "AVX512_VBMI2 " : "";
	const char* CET_SS = (cpu->features.bits.CET_SS) ? "CET_SS " : "";
	const char* GFNI = (cpu->features.bits.GFNI) ? "GFNI " : "";
	const char* VAES = (cpu->features.bits.VAES) ? "VAES " : "";
	const char* VPCLMULQDQ = (cpu->features.bits.VPCLMULQDQ) ? "VPCLMULQDQ " : "";
	const char* AVX512_VNNI = (cpu->features.bits.AVX512_VNNI) ? "AVX512_VNNI " : "";
	const char* AVX512_BITALG = (cpu->features.bits.AVX512_BITALG) ? "AVX512_BITALG " : "";
	const char* TME_EN = (cpu->features.bits.TME_EN) ? "TME_EN " : "";
	const char* AVX512_VPOPCNTDQ = (cpu->features.bits.AVX512_VPOPCNTDQ) ? "AVX512_VPOPCNTDQ " : "";
	const char* LA57 = (cpu->features.bits.LA57) ? "LA57 " : "";
	const char* MAWAU_value = (cpu->features.bits.MAWAU_value) ? "MAWAU_value " : "";
	const char* RDPID = (cpu->features.bits.RDPID) ? "RDPID " : "";
	const char* KL = (cpu->features.bits.KL) ? "KL " : "";
	const char* BUS_LOCK_DETECT = (cpu->features.bits.BUS_LOCK_DETECT) ? "BUS_LOCK_DETECT " : "";
	const char* CLDEMOTE = (cpu->features.bits.CLDEMOTE) ? "CLDEMOTE " : "";
	const char* MOVDIRI = (cpu->features.bits.MOVDIRI) ? "MOVDIRI " : "";
	const char* MOVDIR64B = (cpu->features.bits.MOVDIR64B) ? "MOVDIR64B " : "";
	const char* ENQCMD = (cpu->features.bits.ENQCMD) ? "ENQCMD " : "";
	const char* SGX_LC = (cpu->features.bits.SGX_LC) ? "SGX_LC " : "";
	const char* PKS = (cpu->features.bits.PKS) ? "PKS " : "";

	const char* SGX_KEYS = (cpu->features.bits.SGX_KEYS) ? "SGX_KEYS " : "";
	const char* AVX512_4VNNIW = (cpu->features.bits.AVX512_4VNNIW) ? "AVX512_4VNNIW " : "";
	const char* AVX512_4FMAPS = (cpu->features.bits.AVX512_4FMAPS) ? "AVX512_4FMAPS " : "";
	const char* FastShortREP_MOV = (cpu->features.bits.FastShortREP_MOV) ? "FastShortREP_MOV " : "";
	const char* UINTR = (cpu->features.bits.UINTR) ? "UINTR " : "";
	const char* AVX512_VP2INTERSECT = (cpu->features.bits.AVX512_VP2INTERSECT) ?
		"AVX512_VP2INTERSECT " : "";
	const char* SRBDS_CTRL = (cpu->features.bits.SRBDS_CTRL) ? "SRBDS_CTRL " : "";
	const char* MD_CLEAR = (cpu->features.bits.MD_CLEAR) ? "MD_CLEAR " : "";
	const char* RTM_ALWAYS_ABORT = (cpu->features.bits.RTM_ALWAYS_ABORT) ? "RTM_ALWAYS_ABORT " : "";
	const char* RTM_FORCE_ABORT = (cpu->features.bits.RTM_FORCE_ABORT) ? "RTM_FORCE_ABORT " : "";
	const char* SERIALIZE = (cpu->features.bits.SERIALIZE) ? "SERIALIZE " : "";
	const char* Hybrid = (cpu->features.bits.Hybrid) ? "Hybrid " : "";
	const char* TSXLDTRK = (cpu->features.bits.TSXLDTRK) ? "TSXLDTRK " : "";
	const char* PCONFIG = (cpu->features.bits.PCONFIG) ? "PCONFIG " : "";
	const char* ArchitecturalLBRs = (cpu->features.bits.ArchitecturalLBRs) ?
		"ArchitecturalLBRs " : "";
	const char* CET_IBT = (cpu->features.bits.CET_IBT) ? "CET_IBT " : "";
	const char* AMX_BF16 = (cpu->features.bits.AMX_BF16) ? "AMX_BF16 " : "";
	const char* AVX512_FP16 = (cpu->features.bits.AVX512_FP16) ? "AVX512_FP16 " : "";
	const char* AMX_TILE = (cpu->features.bits.AMX_TILE) ? "AMX_TILE " : "";
	const char* AMX_INT8 = (cpu->features.bits.AMX_INT8) ? "AMX_INT8 " : "";
	const char* IBRS_IBPB = (cpu->features.bits.IBRS_IBPB) ? "IBRS_IBPB " : "";
	const char* STIBP = (cpu->features.bits.STIBP) ? "STIBP " : "";
	const char* L1D_FLUSH = (cpu->features.bits.L1D_FLUSH) ? "L1D_FLUSH " : "";
	const char* IA32_ARCH_CAPABILITIES = (cpu->features.bits.IA32_ARCH_CAPABILITIES) ?
		"IA32_ARCH_CAPABILITIES " : "";
	const char* IA32_CORE_CAPABILITIES = (cpu->features.bits.IA32_CORE_CAPABILITIES) ?
		"IA32_CORE_CAPABILITIES " : "";
	const char* SSBD = (cpu->features.bits.SSBD) ? "SSBD " : "";

	const char* AVX_VNNI = (cpu->features.bits.AVX_VNNI) ? "AVX_VNNI " : "";
	const char* AVX512_BF16 = (cpu->features.bits.AVX512_BF16) ? "AVX512_BF16 " : "";
	const char* FastZeroLength_REP_MOVSB = (cpu->features.bits.FastZeroLength_REP_MOVSB) ?
		"FastZeroLength_REP_MOVSB " : "";
	const char* FastShort_REP_STOSB = (cpu->features.bits.FastShort_REP_STOSB) ?
		"FastShort_REP_STOSB " : "";
	const char* FastShort_REP_CMPSB_SCASB = (cpu->features.bits.FastShort_REP_CMPSB_SCASB) ?
		"FastShort_REP_CMPSB_SCASB " : "";
	const char* HRESET = (cpu->features.bits.HRESET) ? "HRESET " : "";
	const char* INVD_DISABLE_POST_BIOS_DONE = (cpu->features.bits.INVD_DISABLE_POST_BIOS_DONE) ?
		"INVD_DISABLE_POST_BIOS_DONE " : "";
	const char* IA32_PPIN = (cpu->features.bits.IA32_PPIN) ? "IA32_PPIN " : "";
	const char* CET_SSS = (cpu->features.bits.CET_SSS) ? "CET_SSS " : "";

	const char* PSFD = (cpu->features.bits.PSFD) ? "PSFD " : "";
	const char* IPRED_CTRL = (cpu->features.bits.IPRED_CTRL) ? "IPRED_CTRL " : "";
	const char* RRSBA_CTRL = (cpu->features.bits.RRSBA_CTRL) ? "RRSBA_CTRL " : "";
	const char* DDPD_U = (cpu->features.bits.DDPD_U) ? "DDPD_U " : "";
	const char* BHI_CTRL = (cpu->features.bits.BHI_CTRL) ? "BHI_CTRL " : "";
	const char* MCDT_NO = (cpu->features.bits.MCDT_NO) ? "MCDT_NO " : "";
	const char* MONITOR_MITG_NO = (cpu->features.bits.MONITOR_MITG_NO) ? "MONITOR_MITG_NO " : "";

	// Note: TSC handled directly later in the log

	log(INFO, MODULE, "Features 1: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		SSE3,PCLMULQDQ,DTES64,MONITOR,DS_CPL,VMX,SMX,EIST,TM2,SSSE3,CNXT_ID,SDBG,FMA,CMPXCHG16B,
		xTPR,PDCM,PCID,DCA,SSE4_1,SSE4_2,x2APIC,MOVBE,POPCNT,TSC_Deadline,AES,XSAVE,OSXSAVE,AVX,
		F16C,RDRAND);
	log(INFO, MODULE, "Features 2: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		FPU,VME,DE,PSE,TSC,MSR_RDMSR,PAE,MCE,CX8,APIC,SEP,MTRR,PGE,MCA,CMOV,PAT,PSE_36,PSN,CLFSH,
		DS,ACPI,MMX,FXSR,SSE,SSE2,SS,HTT,TM,PBE);

	log(INFO, MODULE,
		"Features 7-0-ebx: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		FSGSBASE,IA32_TSC_ADJUST,SGX,BMI1,HLE,AVX2,FDP_EXCPTN_ONLY,SMEP,BMI2,Enhanced_MOVSB_STOSB,
		INVPCID,RTM,RDT_M,Deprecated_FPU_CS_DS,MPX,RDT_A,AVX512F,AVX512DQ,RDSEED,ADX,SMAP,
		AVX512_IFMA,CLFLUSHOPT,CLWB,IntelProcessorTrace,AVX512PF,AVX512ER,AVX512CD,SHA,AVX512BW,
		AVX512VL);
	log(INFO, MODULE, "Features 7-0-ecx: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		PREFETCHWT1,AVX512_VBMI,UMIP,PKU,OSPKE,WAITPKG,AVX512_VBMI2,CET_SS,GFNI,VAES,VPCLMULQDQ,
		AVX512_VNNI,AVX512_BITALG,TME_EN,AVX512_VPOPCNTDQ,LA57,MAWAU_value,RDPID,KL,
		BUS_LOCK_DETECT,CLDEMOTE,MOVDIRI,MOVDIR64B,ENQCMD,SGX_LC,PKS);
	log(INFO, MODULE, "Features 7-0-edx: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		SGX_KEYS,AVX512_4VNNIW,AVX512_4FMAPS,FastShortREP_MOV,UINTR,AVX512_VP2INTERSECT,SRBDS_CTRL,
		MD_CLEAR,RTM_ALWAYS_ABORT,RTM_FORCE_ABORT,SERIALIZE,Hybrid,TSXLDTRK,PCONFIG,
		ArchitecturalLBRs,CET_IBT,AMX_BF16,AVX512_FP16,AMX_TILE,AMX_INT8,IBRS_IBPB,STIBP,L1D_FLUSH,
		IA32_ARCH_CAPABILITIES,IA32_CORE_CAPABILITIES,SSBD);

	log(INFO, MODULE, "Features 7-1: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		AVX_VNNI,AVX512_BF16,FastZeroLength_REP_MOVSB,FastShort_REP_STOSB,FastShort_REP_CMPSB_SCASB,
		HRESET,INVD_DISABLE_POST_BIOS_DONE,IA32_PPIN,CET_SSS,PSFD,IPRED_CTRL,RRSBA_CTRL,DDPD_U,
		BHI_CTRL,MCDT_NO,MONITOR_MITG_NO);

	if (cpu->features.bits.TscCrystalClockFrequency != 0){
		log(INFO, MODULE, "TSC crystal clock ratio: %u/%u & crystal frequency: %d MHz",
			cpu->features.bits.TscClockRatioNumerator, cpu->features.bits.TscClockRatioDenominator,
			cpu->features.bits.TscCrystalClockFrequency);
	}

	if (cpu->features.bits.BaseFrequency != 0){
		log(INFO, MODULE, "CPU frequencies: base %d MHz, max %d MHz, bus %d Mhz",
		cpu->features.bits.BaseFrequency, cpu->features.bits.MaxFrequency,
		cpu->features.bits.BusFrequency);
	}

	const char* LAHF_SAHF = (cpu->extFeatures.bits.LAHF_SAHF) ? "LAHF_SAHF " : "";
	const char* LZCNT = (cpu->extFeatures.bits.LZCNT) ? "LZCNT " : "";
	const char* PREFETCHW = (cpu->extFeatures.bits.PREFETCHW) ? "PREFETCHW " : "";
	const char* SYSCALL_SYSRET = (cpu->extFeatures.bits.SYSCALL_SYSRET) ? "SYSCALL_SYSRET " : "";
	const char* NX = (cpu->extFeatures.bits.NX) ? "NX " : "";
	const char* PAGES_1GB = (cpu->extFeatures.bits.PAGES_1GB) ? "PAGES_1GB " : "";
	const char* RDTSCP_and_IA32_TSC_AUX = (cpu->extFeatures.bits.RDTSCP_and_IA32_TSC_AUX) ?
		"RDTSCP_and_IA32_TSC_AUX " : "";
	const char* LongMode = (cpu->extFeatures.bits.LongMode) ? "LongMode " : "";

	const char* InvariantTSC = (cpu->extFeatures.bits.InvariantTSC) ? "InvariantTSC " : "";

	const char* WBNOINVD = (cpu->extFeatures.bits.WBNOINVD) ? "WBNOINVD " : "";

	log(INFO, MODULE, "Features extended: %s%s%s%s%s%s%s%s%s%s",
		LAHF_SAHF,LZCNT,PREFETCHW,SYSCALL_SYSRET,NX,PAGES_1GB,
		RDTSCP_and_IA32_TSC_AUX,LongMode,InvariantTSC,WBNOINVD);
}
