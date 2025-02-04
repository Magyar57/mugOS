#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <stdint.h>
#include <stddef.h>
#include "ArchDataTypes.h"

// DataTypes.h: UEFI data types
// https://uefi.org/specs/UEFI/2.11/02_Overview.html#data-types

#define IN
#define OUT
#define OPTIONAL
#define CONST const
#define EFIAPI __attribute__((ms_abi))

typedef unsigned char BOOLEAN;
#define FALSE 0
#define TRUE 1

typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;
// typedef int128_t INT128;
// typedef uint128_t UINT128;

typedef char CHAR8;
typedef uint_least16_t char16_t;
typedef char16_t CHAR16;

typedef void VOID;

typedef struct s_EFI_GUID {
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	UINT8 Data4[8];
} EFI_GUID;

typedef UINTN EFI_STATUS;
typedef VOID* EFI_HANDLE;
typedef VOID* EFI_EVENT;
typedef UINT64 EFI_LBA;
typedef UINTN EFI_TPL;

// This is used in every table, so we consider it a "data type"
// as it contains only basic types
typedef struct s_EFI_TABLE_HEADER {
	UINT64 Signature;
	UINT32 Revision;
	UINT32 HeaderSize;
	UINT32 CRC32;
	UINT32 Reserved;
} EFI_TABLE_HEADER;

// Event Types
typedef VOID* EFI_EVENT;
#define EVT_TIMER							0x80000000
#define EVT_RUNTIME							0x40000000
#define EVT_NOTIFY_WAIT						0x00000100
#define EVT_NOTIFY_SIGNAL					0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES		0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE	0x60000202

typedef VOID (EFIAPI *EFI_EVENT_NOTIFY) (IN EFI_EVENT Event, IN VOID* Context);

// Reset types
typedef enum {
	EfiResetCold,
	EfiResetWarm,
	EfiResetShutdown,
	EfiResetPlatformSpecific
} EFI_RESET_TYPE;

#endif
