#ifndef __CONFIGURATION_TABLES_H__
#define __CONFIGURATION_TABLES_H__

#include "DataTypes.h"

// ConfigurationTables.h: UEFI configuration tables
// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-configuration-table-properties-table

typedef struct s_EFI_CONFIGURATION_TABLE {
	EFI_GUID VendorGuid;
	VOID* VendorTable;
} EFI_CONFIGURATION_TABLE;

// ==== Industry standard configuration tables ====
#define EFI_ACPI_20_TABLE_GUID	{0x8868e871,0xe4f1,0x11d3,{0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}
#define ACPI_TABLE_GUID			{0xeb9d2d30,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SAL_SYSTEM_TABLE_GUID	{0xeb9d2d32,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SMBIOS_TABLE_GUID		{0xeb9d2d31,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SMBIOS3_TABLE_GUID		{0xf2fd1544,0x9794,0x4a2c,{0x99,0x2e,0xe5,0xbb,0xcf,0x20,0xe3,0x94}}
#define MPS_TABLE_GUID			{0xeb9d2d2f,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
// ACPI 2.0 or newer tables should use EFI_ACPI_TABLE_GUID
#define EFI_ACPI_TABLE_GUID		{0x8868e871,0xe4f1,0x11d3,{0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}
// #define EFI_ACPI_20_TABLE_GUID	EFI_ACPI_TABLE_GUID
#define ACPI_TABLE_GUID			{0xeb9d2d30,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define ACPI_10_TABLE_GUID ACPI_TABLE_GUID*

// ==== JSON Configuration tables ====
#define EFI_JSON_CONFIG_DATA_TABLE_GUID		{0x87367f87,0x1119,0x41ce,{0xaa,0xec,0x8b,0xe0,0x11,0x1f,0x55,0x8a}}
#define EFI_JSON_CAPSULE_DATA_TABLE_GUID	{0x35e7a725,0x8dd2,0x4cac,{0x80,0x11,0x33,0xcd,0xa8,0x10,0x90,0x56}}
#define EFI_JSON_CAPSULE_RESULT_TABLE_GUID	{0xdbc461c3,0xb3de,0x422a,{0xb9,0xb4,0x98,0x86,0xfd,0x49,0xa1,0xe5}}

// ==== Devicetree tables ====
// in Flattened Devicetree Blob (DTB) format
#define EFI_DTB_TABLE_GUID {0xb1b621d5,0xf19c,0x41a5,{0x83,0x0b,0xd9,0x15,0x2c,0x69,0xaa,0xe0}}

// ==== RT Properties table ====
// The plateform (firmware) is allowed to not support some runtime services once
// ExitBootServices() has been called. This table allows it to inform the OS which
// services are unsupported.

#define EFI_RT_PROPERTIES_TABLE_GUID {0xeb66918a,0x7eef,0x402a,{0x84,0x2e,0x93,0x1d,0x21,0xc3,0x8a,0xe9}}

typedef struct s_EFI_RT_PROPERTIES_TABLE {
	UINT16 Version;
	UINT16 Length;
	UINT32 RuntimeServicesSupported;
} EFI_RT_PROPERTIES_TABLE;

#define EFI_RT_PROPERTIES_TABLE_VERSION 0x1
#define EFI_RT_SUPPORTED_GET_TIME						0x0001
#define EFI_RT_SUPPORTED_SET_TIME						0x0002
#define EFI_RT_SUPPORTED_GET_WAKEUP_TIME				0x0004
#define EFI_RT_SUPPORTED_SET_WAKEUP_TIME				0x0008
#define EFI_RT_SUPPORTED_GET_VARIABLE					0x0010
#define EFI_RT_SUPPORTED_GET_NEXT_VARIABLE_NAME			0x0020
#define EFI_RT_SUPPORTED_SET_VARIABLE					0x0040
#define EFI_RT_SUPPORTED_SET_VIRTUAL_ADDRESS_MAP		0x0080
#define EFI_RT_SUPPORTED_CONVERT_POINTER				0x0100
#define EFI_RT_SUPPORTED_GET_NEXT_HIGH_MONOTONIC_COUNT	0x0200
#define EFI_RT_SUPPORTED_RESET_SYSTEM					0x0400
#define EFI_RT_SUPPORTED_UPDATE_CAPSULE					0x0800
#define EFI_RT_SUPPORTED_QUERY_CAPSULE_CAPABILITIES		0x1000
#define EFI_RT_SUPPORTED_QUERY_VARIABLE_INFO			0x2000

// ==== Memory attributes table ====
// See Memory.h

// ==== EFI Conformance profile table ====
// If this table is not present, the plateform is conformat with the UEFI specification.

#define EFI_CONFORMANCE_PROFILES_TABLE_GUID		{0x36122546,0xf7e7,0x4c8f,{0xbd,0x9b,0xeb,0x85,0x25,0xb5,0x0c,0x0b}}
#define EFI_CONFORMANCE_PROFILES_UEFI_SPEC_GUID	{0x523c91af,0xa195,0x4382,{0x81,0x8d,0x29,0x5f,0xe4,0x00,0x64,0x65}}

#define EFI_CONFORMANCE_PROFILES_TABLE_VERSION 0x1

typedef struct {
	UINT16 Version;
	UINT16 NumberOfProfiles;
	EFI_GUID ConformanceProfiles[];
} EFI_CONFORMANCE_PROFILES_TABLE;

// Un-implemented tables:
// https://uefi.org/specs/UEFI/2.11/04_EFI_System_Table.html#other-configuration-tables
// > EFI_MEMORY_RANGE_CAPSULE_GUID, EFI_DEBUG_IMAGE_INFO_TABLE, EFI_SYSTEM_RESOURCE_TABLE, 
// > EFI_IMAGE_EXECUTION_INFO_TABLE, User Information Table, HII Database export buffer

#endif
