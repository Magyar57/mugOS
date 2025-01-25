#ifndef __TIME_H__
#define __TIME_H__

#include "EFI/DataTypes.h"

// Time.h: EFI Time structures and definitions

// EFI_TIME: current time informations
typedef struct {
	UINT16 Year;		// 1900 - 9999
	UINT8 Month;		// 1 - 12
	UINT8 Day;			// 1 - 31
	UINT8 Hour;			// 0 - 23
	UINT8 Minute;		// 0 - 59
	UINT8 Second;		// 0 - 59
	UINT8 Pad1;
	UINT32 Nanosecond;	// 0 - 999 999 999
	INT16 TimeZone;		// -1440 to 1440 or 2047 ; LocalTime=UTC-TimeZone
	UINT8 Daylight;
	UINT8 Pad2;
} EFI_TIME;

#define EFI_UNSPECIFIED_TIMEZONE   0x07FF	// EFI_TIME.TimeZone value, meaning that timezone is unspecified

// Capabilities of the RTC (through the EFI interfaces)
typedef struct {
	UINT32 Resolution;
	UINT32 Accuracy;
	BOOLEAN SetsToZero;
} EFI_TIME_CAPABILITIES;

#endif
