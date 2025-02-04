#ifndef __EFI_H__
#define __EFI_H__

// EFI.h: Implementation of the UEFI specification
// https://uefi.org/specs/UEFI/2.11/

// Important notes on UEFI spec:
// - Unless otherwise specified, UEFI uses a standard CCITT32 CRC algorithm with
//   a seed polynomial value of 0x04c11db7 for its CRC calculations.

#include "EFI/DataTypes.h"
#include "EFI/StatusCodes.h"
#include "EFI/SystemTable.h"
#include "EFI/Services.h"

#endif
