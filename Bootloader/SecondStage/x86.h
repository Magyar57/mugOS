#ifndef __H__
#define __H__

#include <stdint.h>
#include <stdbool.h>

void __attribute__((cdecl)) x86_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) x86_inb(uint16_t port);

// Reset the disk identified by the drive parameter
// @param drive The disk identifier
// @return True on success, false on failure
bool __attribute__((cdecl)) x86_Disk_Reset(uint8_t drive);

// Read the disk identified by the drive parameter
bool __attribute__((cdecl)) x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t* dataOut);

// Retrieve disk parmeters
// @param drive 		The disk identifier
// @param driveTypeOut 	Output for the drive's type (AT/PS2 floppies only)
// @param cylindersOut 	Number of cylinders
// @param sectorsOut 	Output for the drive's sectors count
// @param headsOut 		Number of heads
// @return 				True on success, false on failure
bool __attribute__((cdecl)) x86_Disk_GetDriveParameters(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, 
														uint16_t* sectorsOut, uint16_t* headsOut);

#endif
