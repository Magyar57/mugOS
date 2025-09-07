#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include "Preprocessor.h"

// CPU.h: Architecture related flags & features (CPUID...)

union Features {
	uint32_t ints[16];
	struct FeaturesBits {
		// ECX values for CPUID EAX=0x01
		uint32_t SSE3 : 1;
		uint32_t PCLMULQDQ : 1;
		uint32_t DTES64 : 1;
		uint32_t MONITOR : 1;
		uint32_t DS_CPL : 1;
		uint32_t VMX : 1;
		uint32_t SMX : 1;
		uint32_t EIST : 1;
		uint32_t TM2 : 1;
		uint32_t SSSE3 : 1;
		uint32_t CNXT_ID : 1;
		uint32_t SDBG : 1;
		uint32_t FMA : 1;
		uint32_t CMPXCHG16B : 1;
		uint32_t xTPR : 1;
		uint32_t PDCM : 1;
		uint32_t reserved_0_0 : 1;
		uint32_t PCID : 1;
		uint32_t DCA : 1;
		uint32_t SSE4_1 : 1;
		uint32_t SSE4_2 : 1;
		uint32_t x2APIC : 1;
		uint32_t MOVBE : 1;
		uint32_t POPCNT : 1;
		uint32_t TSC_Deadline : 1;
		uint32_t AES : 1;
		uint32_t XSAVE : 1;
		uint32_t OSXSAVE : 1;
		uint32_t AVX : 1;
		uint32_t F16C : 1;
		uint32_t RDRAND : 1;
		uint32_t reserved_0_1 : 1;
		// EDX values for CPUID EAX=0x01
		uint32_t FPU : 1;
		uint32_t VME : 1;
		uint32_t DE : 1;
		uint32_t PSE : 1;
		uint32_t TSC : 1;
		uint32_t MSR_RDMSR : 1;
		uint32_t PAE : 1;
		uint32_t MCE : 1;
		uint32_t CX8 : 1;
		uint32_t APIC : 1;
		uint32_t reserved_1_0 : 1;
		uint32_t SEP : 1;
		uint32_t MTRR : 1;
		uint32_t PGE : 1;
		uint32_t MCA : 1;
		uint32_t CMOV : 1;
		uint32_t PAT : 1;
		uint32_t PSE_36 : 1;
		uint32_t PSN : 1;
		uint32_t CLFSH : 1;
		uint32_t reserved_1_1 : 1;
		uint32_t DS : 1;
		uint32_t ACPI : 1;
		uint32_t MMX : 1;
		uint32_t FXSR : 1;
		uint32_t SSE : 1;
		uint32_t SSE2 : 1;
		uint32_t SS : 1;
		uint32_t HTT : 1;
		uint32_t TM : 1;
		uint32_t reserved_1_2 : 1;
		uint32_t PBE : 1;

		// EBX values for CPUID EAX=0x07, ECX=0
		uint32_t FSGSBASE : 1;
		uint32_t IA32_TSC_ADJUST : 1;
		uint32_t SGX : 1;
		uint32_t BMI1 : 1;
		uint32_t HLE : 1;
		uint32_t AVX2 : 1;
		uint32_t FDP_EXCPTN_ONLY : 1;
		uint32_t SMEP : 1;
		uint32_t BMI2 : 1;
		uint32_t Enhanced_MOVSB_STOSB : 1;
		uint32_t INVPCID : 1;
		uint32_t RTM : 1;
		uint32_t RDT_M : 1;
		uint32_t Deprecated_FPU_CS_DS : 1;
		uint32_t MPX : 1;
		uint32_t RDT_A : 1;
		uint32_t AVX512F : 1;
		uint32_t AVX512DQ : 1;
		uint32_t RDSEED : 1;
		uint32_t ADX : 1;
		uint32_t SMAP : 1;
		uint32_t AVX512_IFMA : 1;
		uint32_t reserved_7_0 : 1;
		uint32_t CLFLUSHOPT : 1;
		uint32_t CLWB : 1;
		uint32_t IntelProcessorTrace : 1;
		uint32_t AVX512PF : 1;
		uint32_t AVX512ER : 1;
		uint32_t AVX512CD : 1;
		uint32_t SHA : 1; // SHAAAWWW
		uint32_t AVX512BW : 1;
		uint32_t AVX512VL : 1;
		// ECX values for CPUID EAX=0x07, ECX=0
		uint32_t PREFETCHWT1 : 1;
		uint32_t AVX512_VBMI : 1;
		uint32_t UMIP : 1;
		uint32_t PKU : 1;
		uint32_t OSPKE : 1;
		uint32_t WAITPKG : 1;
		uint32_t AVX512_VBMI2 : 1;
		uint32_t CET_SS : 1;
		uint32_t GFNI : 1;
		uint32_t VAES : 1;
		uint32_t VPCLMULQDQ : 1;
		uint32_t AVX512_VNNI : 1;
		uint32_t AVX512_BITALG : 1;
		uint32_t TME_EN : 1;
		uint32_t AVX512_VPOPCNTDQ : 1;
		uint32_t reserved_7_1 : 1;
		uint32_t LA57 : 1;
		uint32_t MAWAU_value : 5;
		uint32_t RDPID : 1;
		uint32_t KL : 1;
		uint32_t BUS_LOCK_DETECT : 1;
		uint32_t CLDEMOTE : 1;
		uint32_t reserved_7_2 : 1;
		uint32_t MOVDIRI : 1;
		uint32_t MOVDIR64B : 1;
		uint32_t ENQCMD : 1;
		uint32_t SGX_LC : 1;
		uint32_t PKS : 1;
		// EDX values for CPUID EAX=0x07, ECX=0
		uint32_t reserved_7_3 : 1;
		uint32_t SGX_KEYS : 1;
		uint32_t AVX512_4VNNIW : 1;
		uint32_t AVX512_4FMAPS : 1;
		uint32_t FastShortREP_MOV : 1;
		uint32_t UINTR : 1;
		uint32_t reserved_7_4 : 2;
		uint32_t AVX512_VP2INTERSECT : 1;
		uint32_t SRBDS_CTRL : 1;
		uint32_t MD_CLEAR : 1;
		uint32_t RTM_ALWAYS_ABORT : 1;
		uint32_t reserved_7_5 : 1;
		uint32_t RTM_FORCE_ABORT : 1;
		uint32_t SERIALIZE : 1;
		uint32_t Hybrid : 1;
		uint32_t TSXLDTRK : 1;
		uint32_t reserved_7_6 : 1;
		uint32_t PCONFIG : 1;
		uint32_t ArchitecturalLBRs : 1;
		uint32_t CET_IBT : 1;
		uint32_t reserved_7_7 : 1;
		uint32_t AMX_BF16 : 1;
		uint32_t AVX512_FP16 : 1;
		uint32_t AMX_TILE : 1;
		uint32_t AMX_INT8 : 1;
		uint32_t IBRS_IBPB : 1;
		uint32_t STIBP : 1;
		uint32_t L1D_FLUSH : 1;
		uint32_t IA32_ARCH_CAPABILITIES : 1;
		uint32_t IA32_CORE_CAPABILITIES : 1;
		uint32_t SSBD : 1;

		// EAX values for CPUID EAX=0x07, ECX=1
		uint32_t reserved_7_8 : 4;
		uint32_t AVX_VNNI : 1;
		uint32_t AVX512_BF16 : 1;
		uint32_t reserved_7_9 : 4;
		uint32_t FastZeroLength_REP_MOVSB : 1;
		uint32_t FastShort_REP_STOSB : 1;
		uint32_t FastShort_REP_CMPSB_SCASB : 1;
		uint32_t reserved_7_10 : 9;
		uint32_t HRESET : 1;
		uint32_t reserved_7_11 : 7;
		uint32_t INVD_DISABLE_POST_BIOS_DONE : 1;
		uint32_t reserved_7_12 : 1;
		// EBX values for CPUID EAX=0x07, ECX=1
		uint32_t IA32_PPIN : 1;
		uint32_t reserved_7_13 : 31;
		// ECX values for CPUID EAX=0x07, ECX=1
		uint32_t reserved_7_14 : 32;
		// EDX values for CPUID EAX=0x07, ECX=1
		uint32_t reserved_7_15 : 18;
		uint32_t CET_SSS : 1;
		uint32_t reserved_7_16 : 13;

		// EAX values for CPUID EAX=0x07, ECX=2
		uint32_t reserved_7_17 : 32;
		// EBX values for CPUID EAX=0x07, ECX=2
		uint32_t reserved_7_18 : 32;
		// ECX values for CPUID EAX=0x07, ECX=2
		uint32_t reserved_7_19 : 32;
		// EDX values for CPUID EAX=0x07, ECX=2
		uint32_t PSFD : 1;
		uint32_t IPRED_CTRL : 1;
		uint32_t RRSBA_CTRL : 1;
		uint32_t DDPD_U : 1;
		uint32_t BHI_CTRL : 1;
		uint32_t MCDT_NO : 1;
		uint32_t reserved_7_20 : 1;
		uint32_t MONITOR_MITG_NO : 1;
		uint32_t reserved_7_21 : 24;

		// EAX values for CPUID EAX=0x15
		uint32_t TscClockRatioDenominator : 32;
		// EBX values for CPUID EAX=0x15
		uint32_t TscClockRatioNumerator : 32;
		// ECX values for CPUID EAX=0x15
		uint32_t TscFrequency : 32;
	} bits;
};

union ExtendedFeatures {
	uint32_t ints[6];
	struct ExtendedFeaturesBits {
		// ECX values for CPUID EAX=0x80000001
		uint32_t LAHF_SAHF : 1;
		uint32_t reserved_1_0 : 4;
		uint32_t LZCNT : 1;
		uint32_t reserved_1_1 : 2;
		uint32_t PREFETCHW : 1;
		uint32_t reserved_1_2 : 23;
		// EDX values for CPUID EAX=0x80000001
		uint32_t reserved_1_3 : 11;
		uint32_t SYSCALL_SYSRET : 1;
		uint32_t reserved_1_4 : 8;
		uint32_t NX : 1; // Execute disable bit
		uint32_t reserved_1_5 : 5;
		uint32_t PAGES_1GB : 1;
		uint32_t RDTSCP_and_IA32_TSC_AUX : 1;
		uint32_t reserved_1_6 : 1;
		uint32_t LongMode : 1;
		uint32_t reserved_1_7 : 2;

		// ECX values for CPUID EAX=0x80000006
		uint32_t CacheLineSize : 8; // in bytes
		uint32_t reserved_2_0 : 4;
		uint32_t L2Associativity : 4;
		uint32_t CacheSize : 16; // in 1K units

		// EDX values for CPUID EAX=0x80000007
		uint32_t reserved_3_0 : 8;
		uint32_t InvariantTSC : 1;
		uint32_t reserved_3_1 : 23;

		// EAX values for CPUID EAX=0x80000008
		uint32_t NumberOfPhysicalAddressBits : 8;
		uint32_t NumberOfLinearAddressBits : 8;
		uint32_t reserved_4_0 : 16;
		// EBX values for CPUID EAX=0x80000008
		uint32_t reserved_4_1 : 9;
		uint32_t WBNOINVD : 1;
		uint32_t reserved_4_2 : 22;
	} bits;
};

enum Vendor {
	Vendor_Unsupported,
	Vendor_Intel,
	Vendor_AMD
};

struct CPU {
	enum Vendor vendor;
	char vendorStr[13];
	char brand[49];
	uint8_t model;
	uint16_t family;
	uint8_t type;
	uint8_t stepping;
	uint8_t brandIndex;
	uint8_t cflushLineSize;
	uint8_t maxAddressableCpuIds;
	union Features features;
	union ExtendedFeatures extFeatures;

	int maxInformation; // Max input value for basic CPUID information
	int maxExtendedInformation; // Max input value for extended CPUID information
};

extern struct CPU g_CPU;

/// @brief Fill the CPU struct with CPU identification informations
/// @note Acts as an assertion to some arch-specific mugOS-required features
void CPU_init(struct CPU* cpu);

void CPU_print(struct CPU* cpu);

#endif
