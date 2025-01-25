#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "EFI/DataTypes.h"

// Memory.h: UEFI Memory definitions

typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

typedef struct {
	UINT32 Type;
	EFI_PHYSICAL_ADDRESS PhysicalStart;
	EFI_VIRTUAL_ADDRESS VirtualStart;
	UINT64 NumberOfPages;
	UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef enum {
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiUnacceptedMemoryType,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

// Memory attributes
#define EFI_MEMORY_UC				0x0000000000000001
#define EFI_MEMORY_WC				0x0000000000000002
#define EFI_MEMORY_WT				0x0000000000000004
#define EFI_MEMORY_WB				0x0000000000000008
#define EFI_MEMORY_UCE				0x0000000000000010
#define EFI_MEMORY_WP				0x0000000000001000
#define EFI_MEMORY_RP				0x0000000000002000
#define EFI_MEMORY_XP				0x0000000000004000
#define EFI_MEMORY_NV				0x0000000000008000
#define EFI_MEMORY_MORE_RELIABLE	0x0000000000010000
#define EFI_MEMORY_RO				0x0000000000020000
#define EFI_MEMORY_SP				0x0000000000040000
#define EFI_MEMORY_CPU_CRYPTO		0x0000000000080000
#define EFI_MEMORY_HOT_PLUGGABLE	0x0000000000100000
#define EFI_MEMORY_RUNTIME			0x8000000000000000
#define EFI_MEMORY_ISA_VALID		0x4000000000000000
#define EFI_MEMORY_ISA_MASK			0x0FFFF00000000000

// When published by firmware, this table provides additional information
// to the BootServices.GetMemoryMap() function
// Note: currently, a consumer must ignore Type entries that are NOT EfiRuntimeServicesData and EfiRuntimeServicesCode

#define EFI_MEMORY_ATTRIBUTES_TABLE_GUID {0xdcfa911d,0x26eb,0x469f,{0xa2,0x20,0x38,0xb7,0xdc,0x46,0x12,0x20}}

typedef struct s_EFI_MEMORY_ATTRIBUTES_TABLE {
	UINT32 Version;
	UINT32 NumberOfEntries;
	UINT32 DescriptorSize;
	UINT32 Flags;
	EFI_MEMORY_DESCRIPTOR Entry[1]; // Array of size NumberOfEntries
} EFI_MEMORY_ATTRIBUTES_TABLE;

#endif
