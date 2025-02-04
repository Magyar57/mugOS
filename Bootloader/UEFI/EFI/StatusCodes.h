#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

#include "ArchStatusCodes.h"

// UEFI Error, warning and success codes
// https://uefi.org/specs/UEFI/2.10/Apx_D_Status_Codes.html

#define EFI_SUCCESS 0

#define EFI_ERROR(code) (((INTN) code) < 0)

// Error codes (<0)
#define EFI_LOAD_ERROR				ERR(1)
#define EFI_INVALID_PARAMETER		ERR(2)
#define EFI_UNSUPPORTED				ERR(3)
#define EFI_BAD_BUFFER_SIZE			ERR(4)
#define EFI_BUFFER_TOO_SMALL		ERR(5)
#define EFI_NOT_READY				ERR(6)
#define EFI_DEVICE_ERROR			ERR(7)
#define EFI_WRITE_PROTECTED			ERR(8)
#define EFI_OUT_OF_RESOURCES		ERR(9)
#define EFI_VOLUME_CORRUPTED 		ERR(10)
#define EFI_VOLUME_FULL 			ERR(11)
#define EFI_NO_MEDIA 				ERR(12)
#define EFI_MEDIA_CHANGED 			ERR(13)
#define EFI_NOT_FOUND 				ERR(14)
#define EFI_ACCESS_DENIED 			ERR(15)
#define EFI_NO_RESPONSE 			ERR(16)
#define EFI_NO_MAPPING 				ERR(17)
#define EFI_TIMEOUT 				ERR(18)
#define EFI_NOT_STARTED 			ERR(19)
#define EFI_ALREADY_STARTED 		ERR(20)
#define EFI_ABORTED 				ERR(21)
#define EFI_ICMP_ERROR 				ERR(22)
#define EFI_TFTP_ERROR 				ERR(23)
#define EFI_PROTOCOL_ERROR 			ERR(24)
#define EFI_INCOMPATIBLE_VERSION 	ERR(25)
#define EFI_SECURITY_VIOLATION 		ERR(26)
#define EFI_CRC_ERROR 				ERR(27)
#define EFI_END_OF_MEDIA 			ERR(28)
#define EFI_END_OF_FILE 			ERR(31)
#define EFI_INVALID_LANGUAGE 		ERR(32)
#define EFI_COMPROMISED_DATA 		ERR(33)
#define EFI_IP_ADDRESS_CONFLICT 	ERR(34)
#define EFI_HTTP_ERROR 				ERR(35)

// Warnings (>0)
#define EFI_WARN_UNKNOWN_GLYPH		1
#define EFI_WARN_DELETE_FAILURE		2
#define EFI_WARN_WRITE_FAILURE		3
#define EFI_WARN_BUFFER_TOO_SMALL	4
#define EFI_WARN_STALE_DATA			5
#define EFI_WARN_FILE_SYSTEM		6
#define EFI_WARN_RESET_REQUIRED		7

#endif
