#ifndef __ACPI_TABLES_H__
#define __ACPI_TABLES_H__

#include <stdint.h>
#include "Preprocessor.h"

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

// ACPI table: eXtended System Descriptor Table
struct XSDT {
	struct SDTHeader header;
	uint64_t tables[]; // size is (h.Length - sizeof(h)) / 8
} packed;

// ================ MADT: Multiple APIC Description Table ================

enum MADTEntryType {
	MADT_ENTRYTYPE_LAPIC = 0,					// Processor Local APIC
	MADT_ENTRYTYPE_IOAPIC = 1,					// I/O APIC
	MADT_ENTRYTYPE_IOAPIC_ISO = 2,				// I/O APIC Interrupt Sources Override
	MADT_ENTRYTYPE_IOAPIC_NMI_SRC = 3,			// I/O APIC Non-Maskable Interrupt Sources
	MADT_ENTRYTYPE_LAPIC_NMI = 4,				// Local APIC Non-Maskable Interrupt
	MADT_ENTRYTYPE_LAPIC_ADDR_OVERRIDE = 5,		// Local APIC Address Override (unique ; use if present)
	MADT_ENTRYTYPE_LX2APIC = 9,					// Processor Local x2APIC
};

struct MADTEntryHeader {
	uint8_t entryType;		// enum MADTEntryType
	uint8_t entryLength;
} packed;

union CommonAPICFlags {
	uint16_t value;
	struct {
		uint16_t reserved_0 : 1;
		uint16_t activeWhenLow : 1;
		uint16_t reserved_1 : 1;
		uint16_t levelTriggered : 1;
	} bits;
} packed;

// 0 MADT_ENTRYTYPE_LAPIC
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

// 1 MADT_ENTRYTYPE_IOAPIC
struct MADTEntry_IOAPIC {
	struct MADTEntryHeader header;
	uint8_t ID;
	uint8_t reserved;
	uint32_t address;
	uint32_t GSIBase;
} packed;

// 2 MADT_ENTRYTYPE_IOAPIC_ISO
struct MADTEntry_IOAPIC_ISO {
	struct MADTEntryHeader header;
	uint8_t busSource;
	uint8_t IRQSource;
	uint32_t GSI;
	union CommonAPICFlags flags;
} packed;

// 3 MADT_ENTRYTYPE_IOAPIC_NMI_SRC
struct MADTEntry_IOAPIC_NMI_SRC {
	struct MADTEntryHeader header;
	uint8_t source;
	uint8_t reserved;
	union CommonAPICFlags flags;
	uint32_t GSI;
} packed;

// 4 MADT_ENTRYTYPE_LAPIC_NMI
struct MADTEntry_LAPIC_NMI {
	struct MADTEntryHeader header;
	uint8_t ACPIProcessorID; // 0xff = all processors
	union CommonAPICFlags flags;
	uint8_t LINTi; // 0 or 1
} packed;

// 5 MADT_ENTRYTYPE_LAPIC_ADDR_OVERRIDE
struct MADTEntry_LAPIC_ADDR_OVERRIDE {
	struct MADTEntryHeader header;
	uint16_t reserved;
	uint64_t address;
} packed;

// 9 MADT_ENTRYTYPE_LX2APIC
struct MADTEntry_LX2APIC {
	struct MADTEntryHeader header;
	uint16_t reserved;
	uint32_t processorID;
	uint32_t flags_padding; // this flag section is 4 bytes instead of 2
	union CommonAPICFlags flags;
	uint32_t ACPI_ID;
} packed;

// ACPI table: Multiple APIC Description Table (parsed)
struct MADT {
	struct SDTHeader header;
	uint32_t localApicAddress;
	uint32_t localApicFlags;

	int nLAPIC;
	int nIOAPIC;
	int nIOAPIC_ISO;
	int nIOAPIC_NMI_SRC;
	int nLAPIC_NMI;
	int nLAPIC_ADDR_OVERRIDE;
	int nLX2APIC;
	struct MADTEntry_LAPIC* LAPICs;
	struct MADTEntry_IOAPIC* IOAPICs;
	struct MADTEntry_IOAPIC_ISO* IOAPIC_ISOs;
	struct MADTEntry_IOAPIC_NMI_SRC* IOAPIC_NMI_SRCs;
	struct MADTEntry_LAPIC_NMI* LAPIC_NMIs;
	struct MADTEntry_LAPIC_ADDR_OVERRIDE* LAPIC_ADDR_OVERRIDEs;
	struct MADTEntry_LX2APIC* LX2APICs;
};

#endif
