#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>

typedef struct {
	uint8_t id;
	uint16_t cylinders;
	uint16_t sectors;
	uint16_t heads;
} DISK;

// Initalize a DISK structure to the parameters of the disk referenced by driveNumber
bool DISK_initalize(DISK* disk, uint8_t driveNumber);

// Read "n_sectors" sectors located at the lba "lba", on the disk "disk", into "dataOut" (dataOut MUST BE in the lower memory, aka < 1MB)
bool DISK_readSectors(DISK* disk, uint32_t lba, uint8_t n_sectors, uint8_t* dataOut);

#endif
