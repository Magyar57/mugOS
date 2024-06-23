#include <stddef.h>
#include <stdint.h>
#include "stdio.h"
#include "x86.h"

#include "disk.h"

bool DISK_initalize(DISK* disk, uint8_t driveNumber){
	if (disk == NULL) return false;
	
	uint8_t driveType;
	uint16_t cylinders, sectors, heads;
	bool res = x86_Disk_GetDriveParameters(driveNumber, &driveType, &cylinders, &sectors, &heads);
	if(!res) return false;

	disk->id = driveNumber;
	disk->cylinders = cylinders;
	disk->sectors = sectors;
	disk->heads = heads;
	return true;
}

static void DISK_LBAtoCHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut){
	if (disk==NULL || cylinderOut==NULL || sectorOut==NULL || headOut==NULL) return;
	
	// sector = (LBA % sectors per track + 1)
	*sectorOut = lba % disk->sectors + 1;
	
	// cylinder = (LBA / sectors per track) / heads
	*cylinderOut = (lba / disk->sectors) / disk->heads;

	// head = (LBA / sectors per track) % heads
	*headOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_readSectors(DISK* disk, uint32_t lba, uint8_t n_sectors, uint8_t* dataOut){
	if (dataOut == NULL) return false;

	uint16_t cylinder, sector, head;
	DISK_LBAtoCHS(disk, lba, &cylinder, &sector, &head);

	// As recommended, we try to read the floppy at least 3 times before giving up
	for(int i = 0; i<3 ; i++){
		bool ok = x86_Disk_Read(disk->id, cylinder, sector, head, n_sectors, dataOut);
		if (ok) return true;

		x86_Disk_Reset(disk->id); // if previous read failed, reset the disk before retrying
	}

	return false;
}
