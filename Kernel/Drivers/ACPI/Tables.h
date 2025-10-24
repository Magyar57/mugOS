#ifndef __ACPI_TABLES_H__
#define __ACPI_TABLES_H__

#include <stdint.h>
#include "mugOS/Preprocessor.h"

// The System Descriptor Table header. Common almost all ACPI tables
struct SDTHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t creatorID;
	uint32_t creatorRevision;
} packed;

compile_assert(sizeof(struct SDTHeader) == 36);

// The Generic Address Structure (GAS) is a standard way to describes
// system register locations and data structures
struct GenericAddressStructure {
	// The address space where the data structure or register exists. Defined values are:
	// - 0x00: System Memory space
	// - 0x01: System I/O space
	// - 0x02: PCI Configuration space
	// - 0x03: Embedded Controller
	// - 0x04: SMBus
	// - 0x05: SystemCMOS
	// - 0x06: PciBarTarget
	// - 0x07: IPMI
	// - 0x08: General PurposeIO
	// - 0x09: GenericSerialBus
	// - 0x0a: Platform Communications Channel (PCC)
	// - 0x0b: Platform Runtime Mechanism (PRM)
	// - 0x0c: to 0x7E Reserved
	// - 0x7f: Functional Fixed Hardware
	// - 0x80: to 0xFF OEM Defined
	uint8_t addressSpace;

	// The size in bits of the given register.
	// If entry doesn't describes a register, must be 0
	uint8_t bitWidth;

	// The bit offset of the given register at the given address.
	// If entry doesn't describes a register, must be 0
	uint8_t bitOffset;

	// Specifies access size. Unless otherwise defined by the Address Space ID:
	// - 0 Undefined (legacy reasons)
	// - 1 Byte access
	// - 2 Word access
	// - 3 Dword access
	// - 4 QWord access
	uint8_t accessSize;

	// The 64-bit address of the data structure or register in the given address space
	// (relative to the processor). Format depends on the address space
	// Note: we can't put a uint64_t because of alignment+pack constraints
	uint32_t address[2];
} packed;

compile_assert(sizeof(struct GenericAddressStructure) == 12);

// ================ RSDP: Root System Description Pointer ================

// ACPI table: Root System Description Pointer
struct RSDP {
	char signature[8];
	uint8_t checksum;
	char OEMID[6];
	uint8_t revision;
	uint32_t rsdtAddress; // Deprecated in ACPI v2.0
	uint32_t length;
	uint64_t xsdtAddress;
	uint8_t extendedChecksum;
	uint8_t reserved[3];
} packed;

// ================ XSDT: eXtended System Descriptor Table ================

// ACPI table: eXtended System Descriptor Table
struct XSDT {
	struct SDTHeader header;
	uint64_t tables[]; // size is (h.Length - sizeof(h)) / 8
} packed;

// ================ FADT: Fixed ACPI Description Table ================

struct FADT {
	struct SDTHeader header;
	uint32_t firmwareControl;
	uint32_t DSDT;

	uint8_t reserved_0; // deprecated, used in ACPI 1.0
	uint8_t preferredPowerManagementProfile;
	uint16_t SCIInterrupt;
	uint32_t SMICommandPort;
	uint8_t ACPIEnable;
	uint8_t ACPIDisable;
	uint8_t S4BIOSReq;
	uint8_t PSTATEControl;
	uint32_t PM1aEventBlock;
	uint32_t PM1bEventBlock;
	uint32_t PM1aControlBlock;
	uint32_t PM1bControlBlock;
	uint32_t PM2ControlBlock;
	uint32_t PMTimerBlock;
	uint32_t GPE0Block;
	uint32_t GPE1Block;
	uint8_t PM1EventLength;
	uint8_t PM1ControlLength;
	uint8_t PM2ControlLength;
	uint8_t PMTimerLength;
	uint8_t GPE0Length;
	uint8_t GPE1Length;
	uint8_t GPE1Base;
	uint8_t CStateControl;
	uint16_t worstC2Latency;
	uint16_t worstC3Latency;
	uint16_t flushSize;
	uint16_t flushStride;
	uint8_t dutyOffset;
	uint8_t dutyWidth;
	uint8_t dayAlarm;
	uint8_t monthAlarm;
	uint8_t century;
	// IA-PC boot architecture flags, since ACPI 2.0+
	struct BootArchitectureFlags {
		uint16_t legacyDevices : 1;
		uint16_t i8042 : 1; // set if i8042 (or equivalent) is present on motherboard
		uint16_t vgaNotPresent : 1;
		uint16_t msiNotSupported : 1;
		uint16_t pcieAspmControls : 1;
		uint16_t cmosRtcNotPresent : 1;
		uint16_t reserved : 10;
	} bootArchitectureFlags;
	uint8_t reserved_1;
	struct FixedFeatureFlags {
		uint32_t WBINVD : 1;
		uint32_t WBINVD_FLUSH : 1;
		uint32_t PROC_C1 : 1;
		uint32_t P_LVL2_UP : 1;
		uint32_t PWR_BUTTON : 1;
		uint32_t SLP_BUTTON : 1;
		uint32_t FIX_RTC : 1;
		uint32_t RTC_S4 : 1;
		uint32_t TMR_VAL_EXT : 1;
		uint32_t DCK_CAP : 1;
		uint32_t RESET_REG_SUP : 1;
		uint32_t SEALED_CASE : 1;
		uint32_t HEADLESS : 1;
		uint32_t CPU_SW_SLP : 1;
		uint32_t PCI_EXP_WAK : 1;
		uint32_t USE_PLATFORM_CLOCK : 1;
		uint32_t S4_RTC_STS_VALID : 1;
		uint32_t REMOTE_POWER_ON_CAPABLE : 1;
		uint32_t FORCE_APIC_CLUSTER_MODEL : 1;
		uint32_t FORCE_APIC_PHYSICAL_DESTINATION_MODE : 1;
		uint32_t HW_REDUCED_ACPI : 1;
		uint32_t LOW_POWER_S0_IDLE_CAPABLE : 1;
		uint32_t PERSISTENT_CPU_CACHES : 2;
		uint32_t reserved : 8;
	} fixedFeatureFlags;

	struct GenericAddressStructure resetReg;

	uint8_t  resetValue;
	uint8_t  reserved_2[3];

	uint64_t X_firmwareControl; // since ACPI 2.0+
	uint64_t X_DSDT; // since ACPI 2.0+
	struct GenericAddressStructure X_PM1aEventBlock;
	struct GenericAddressStructure X_PM1bEventBlock;
	struct GenericAddressStructure X_PM1aControlBlock;
	struct GenericAddressStructure X_PM1bControlBlock;
	struct GenericAddressStructure X_PM2ControlBlock;
	struct GenericAddressStructure X_PMTimerBlock;
	struct GenericAddressStructure X_GPE0Block;
	struct GenericAddressStructure X_GPE1Block;
	struct GenericAddressStructure X_sleepControlRegister;
	struct GenericAddressStructure X_sleepStatusRegister;

	uint64_t hypervisorVendorIdentity;
} packed;

compile_assert(sizeof(struct BootArchitectureFlags) == 2);
compile_assert(sizeof(struct FixedFeatureFlags) == 4);

// ================ HPETT: HPET (High Precision Event Timers) Table ================

// ACPI table: HPET (High Precision Event Timers) Table
struct HPETT {
	struct SDTHeader header;
	uint32_t eventTimerBlockId;
	struct GenericAddressStructure eventTimerBlockAddress;
	uint8_t number; // HPET sequence number
	uint16_t minClockTick; // Minimum Clock_tick in Periodic mode
	uint8_t pageProtectionAndOemAttribute;
} packed;

// ================ MADT: Multiple APIC Description Table ================

enum MADTEntryType {
	MADT_ENTRYTYPE_LAPIC =					0x00, // Processor Local APIC
	MADT_ENTRYTYPE_IOAPIC =					0x01, // I/O APIC
	MADT_ENTRYTYPE_IOAPIC_ISO =				0x02, // I/O APIC Interrupt Source Override
	MADT_ENTRYTYPE_IOAPIC_NMI_SRC =			0x03, // I/O APIC Non-Maskable Interrupt Sources
	MADT_ENTRYTYPE_LAPIC_NMI =				0x04, // Local APIC Non-Maskable Interrupt
	MADT_ENTRYTYPE_LAPIC_ADDR_OVERRIDE =	0x05, // Local APIC Address Override
	MADT_ENTRYTYPE_IOSAPIC =				0x06, // I/O SAPIC (Itanium, don't care)
	MADT_ENTRYTYPE_LSAPIC =					0x07, // Local SAPIC (Itanium)
	MADT_ENTRYTYPE_PIS =					0x08, // Plateform Interrupt Sources (Itanium)
	MADT_ENTRYTYPE_X2APIC =					0x09, // Processor Local x2APIC
	MADT_ENTRYTYPE_X2APIC_NMI =				0x0a, // Local x2APIC NMI
	// More stuff, unused in the OS
};

struct MADTEntryHeader {
	uint8_t entryType;		// enum MADTEntryType
	uint8_t entryLength;
} packed;

union CommonAPICFlags {
	uint16_t value;
	struct {
		uint16_t reserved_0 : 1;
		uint16_t pinPolarity : 1; // Pin polarity (0=high 1=low)
		uint16_t reserved_1 : 1;
		uint16_t triggerMode : 1; // Trigger mode (0=edge 1=level)
	} bits;
} packed;

// 0x00 MADT_ENTRYTYPE_LAPIC
struct MADTEntry_LAPIC {
	struct MADTEntryHeader header;
	uint8_t processorID;
	uint8_t lapicID;
	union {
		uint32_t value;
		struct {
			uint32_t enabled : 1;
			uint32_t onlineCapable : 1;
		} bits;
	} flags;
} packed;

// 0x01 MADT_ENTRYTYPE_IOAPIC
struct MADTEntry_IOAPIC {
	struct MADTEntryHeader header;
	uint8_t ID;
	uint8_t reserved;
	uint32_t address;
	uint32_t GSIBase;
} packed;

// 0x02 MADT_ENTRYTYPE_IOAPIC_ISO
struct MADTEntry_IOAPIC_ISO {
	struct MADTEntryHeader header;
	uint8_t busSource;
	uint8_t IRQSource;
	uint32_t GSI;
	union CommonAPICFlags flags;
} packed;

// 0x03 MADT_ENTRYTYPE_IOAPIC_NMI_SRC
struct MADTEntry_IOAPIC_NMI_SRC {
	struct MADTEntryHeader header;
	uint8_t source;
	uint8_t reserved;
	union CommonAPICFlags flags;
	uint32_t GSI;
} packed;

// 0x04 MADT_ENTRYTYPE_LAPIC_NMI
struct MADTEntry_LAPIC_NMI {
	struct MADTEntryHeader header;
	uint8_t ACPIProcessorID; // 0xff = all processors
	union CommonAPICFlags flags;
	uint8_t LINTi; // 0 or 1
} packed;

// 0x05 MADT_ENTRYTYPE_LAPIC_ADDR_OVERRIDE
struct MADTEntry_LAPIC_ADDR_OVERRIDE {
	struct MADTEntryHeader header;
	uint16_t reserved;
	uint64_t address;
} packed;

// 0x09 MADT_ENTRYTYPE_X2APIC
struct MADTEntry_LX2APIC {
	struct MADTEntryHeader header;
	uint16_t reserved;
	uint32_t processorID;
	uint32_t flags_padding; // this flag section is 4 bytes instead of 2
	union CommonAPICFlags flags;
	uint32_t ACPI_ID;
} packed;

// 0x0a MADT_ENTRYTYPE_X2APIC_NMI
struct MADTEntry_LX2APIC_NMI {
	struct MADTEntryHeader header;
	union CommonAPICFlags flags;
	uint32_t processorID;
	uint8_t LINTi;
	uint8_t reserved[3];
};

// ACPI table: Multiple APIC Description Table (parsed)
struct MADT {
	// Begins the same way as the raw ACPI MADT table
	struct SDTHeader header;
	uint32_t localApicAddress;
	uint32_t localApicFlags;

	// For the entries, they are parsed and organized in a nice way
	int nLAPIC;
	int nIOAPIC;
	int nIOAPIC_ISO;
	int nIOAPIC_NMI_SRC;
	int nLAPIC_NMI;
	int nLAPIC_ADDR_OVERRIDE;
	int nX2APIC;
	int nX2APIC_NMI;
	struct MADTEntry_LAPIC* LAPICs;
	struct MADTEntry_IOAPIC* IOAPICs;
	struct MADTEntry_IOAPIC_ISO* IOAPIC_ISOs;
	struct MADTEntry_IOAPIC_NMI_SRC* IOAPIC_NMI_SRCs;
	struct MADTEntry_LAPIC_NMI* LAPIC_NMIs;
	struct MADTEntry_LAPIC_ADDR_OVERRIDE* LAPIC_ADDR_OVERRIDEs;
	struct MADTEntry_LX2APIC* X2APICs;
	struct MADTEntry_LX2APIC_NMI* X2APIC_NMIs;
};

#endif
