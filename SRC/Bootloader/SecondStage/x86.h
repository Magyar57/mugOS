#ifndef __X86_H__
#define __X86_H__

#include "stdint.h"

// Note: the label in the assembly file has an '_' in front of the function name.
// It is because of the cdecl convention, which stipulates that compiled assembly function labels must have an underscore prepended to the function name.

void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);

// Write a character c to the screen
void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);

// Reset the disk identified by the drive parameter
// @param drive The disk identifier
// @return True on success, false on failure
bool _cdecl x86_Disk_Reset(uint8_t drive);

// Read the disk identified by the drive parameter
bool _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t far* dataOut);

// Retrieve disk parmeters
// @param drive 		The disk identifier
// @param driveTypeOut 	Output for the drive's type (AT/PS2 floppies only)
// @param cylindersOut 	Logical last index of cylinders (= number of cylinders - 1)
// @param sectorsOut 	Output for the drive's sectors count
// @param headsOut 		Logical last index of heads (= number of heads - 1)
// @return 				True on success, false on failure
bool _cdecl x86_Disk_GetDriveParameters(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut);

#endif
