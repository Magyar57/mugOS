#ifndef __CAPSULE_H__
#define __CAPSULE_H__

#include "EFI/DataTypes.h"
#include "EFI/Memory.h"

// Capsule.h: Capsule definitions
// https://uefi.org/specs/UEFI/2.10/08_Services_Runtime_Services.html#updatecapsule

typedef struct {
	UINT64 Length;
	union {
		EFI_PHYSICAL_ADDRESS DataBlock;
		EFI_PHYSICAL_ADDRESS ContinuationPointer;
	} Union;
} EFI_CAPSULE_BLOCK_DESCRIPTOR;

typedef struct {
	EFI_GUID CapsuleGuid;
	UINT32 HeaderSize;
	UINT32 Flags;
	UINT32 CapsuleImageSize;
} EFI_CAPSULE_HEADER;

typedef struct {
	UINT32 CapsuleArrayNumber;
	VOID* CapsulePtr[1];
} EFI_CAPSULE_TABLE;

#endif
