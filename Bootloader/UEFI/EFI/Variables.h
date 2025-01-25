#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include "EFI/DataTypes.h"

// Variable Attributes
// https://uefi.org/specs/UEFI/2.10/08_Services_Runtime_Services.html#getvariable
#define EFI_VARIABLE_NON_VOLATILE							0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS						0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS		 					0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD					0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS				0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS	0x00000020
#define EFI_VARIABLE_APPEND_WRITE							0x00000040
#define EFI_VARIABLE_ENHANCED_AUTHENTICATED_ACCESS			0x00000080

// EFI_VARIABLE_AUTHENTICATION_3_CERT_ID descriptor
// An extensible structure to identify a unique x509 cert associated with a given variable
#define EFI_VARIABLE_AUTHENTICATION_3_CERT_ID_SHA256 1
typedef struct {
   UINT8 Type;
   UINT32 IdSize;
   UINT8* Id; // Id[IdSize] ; "not a formal structure member" according to the specification
} EFI_VARIABLE_AUTHENTICATION_3_CERT_ID;

/// @deprecated
// typedef struct {
// 	UINT64 MonotonicCount;
// 	WIN_CERTIFICATE_UEFI_GUID AuthInfo;
// } EFI_VARIABLE_AUTHENTICATION;

// Commented because I couldn't find the WIN_CERTIFICATE_UEFI_GUID defintion in the specification
// typedef struct {
// 	EFI_TIME TimeStamp;
// 	WIN_CERTIFICATE_UEFI_GUID AuthInfo;
// } EFI_VARIABLE_AUTHENTICATION_2;

#define EFI_VARIABLE_AUTHENTICATION_3_TIMESTAMP_TYPE 1
#define EFI_VARIABLE_AUTHENTICATION_3_NONCE_TYPE 2

typedef struct {
	UINT8 Version;
	UINT8 Type;
	UINT32 MetadataSize;
	UINT32 Flags;
} EFI_VARIABLE_AUTHENTICATION_3;

// This structure is always followed by a WIN_CERTIFICATE_UEFI_GUID struct
typedef struct {
	UINT32 NonceSize;
	UINT8* Nonce;
} EFI_VARIABLE_AUTHENTICATION_3_NONCE;

#endif
