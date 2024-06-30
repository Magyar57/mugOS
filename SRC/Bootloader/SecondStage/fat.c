#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "ctype.h"
#include "stdio.h"
#include "string.h"

#include "memdefs.h"

#include "fat.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIR_HANDLE -1

// FAT12 BootSector informations
typedef struct __s_FAT_BootSector {
	uint8_t bootJumpInstruction[3];
	uint8_t oemIdentifier[8];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t reservedSectors;
	uint8_t fatCount;
	uint16_t dirEntryCount;
	uint16_t totalSectors;
	uint8_t mediaDescriptorType;
	uint16_t sectorsPerFat;
	uint16_t sectorsPerTrack;
	uint16_t heads;
	uint32_t hiddenSectors;
	uint32_t largeSectorCount;

	// extended boot record
	uint8_t DriveNumber;
	uint8_t _Reserved;
	uint8_t Signature;
	uint32_t VolumeId; // serial number, value doesn't matter
	uint8_t VolumeLabel[11]; // 11 bytes, padded with spaces
	uint8_t SystemId[8];

	// ...and we don't care about code

}  __attribute__((packed)) __FAT_BootSector;

// Internal file stucture
typedef struct __s_FAT_FileInternal {
	FAT_File metadata; // public metadata, accessible by the user
	bool isOpened;
	uint32_t firstCluster;
	uint32_t curCluster;
	uint32_t curSectorInCluster;
	uint8_t buffer[SECTOR_SIZE];
} __attribute__((packed)) __FAT_FileInternal;

// Internal disk's data: bootsector, root directory, and opened files handles
typedef struct __s_FAT_Data {
	union {
		__FAT_BootSector bootSector;
		uint8_t bootSectorBytes[SECTOR_SIZE];
	} bs;
	__FAT_FileInternal rootDir;
	__FAT_FileInternal openedFiles[MAX_FILE_HANDLES];
} __attribute__((packed)) __FAT_Data;

// Global variables. These are located in the bootloader memory, that we reserved in a special FAT Driver memory space (see memdefs.h)
static __FAT_Data* g_data; // __FAT_Data
static uint8_t* g_fat = NULL; // Disk's FAT
static uint32_t g_dataSectionLba;

// Reads the boot sector of the disk into the global "__FAT_Data g_data" variable
bool __FAT_readBootSector(DISK* disk){
	// Note: The FAT starts just after the reservedSectors data => read from reservedSectors's lba
	return DISK_readSectors(disk, 0, 1, g_data->bs.bootSectorBytes);
}

// Reads the disk's FAT into the global "g_fat" variable
bool __FAT_readFat(DISK* disk){
	return DISK_readSectors(disk, g_data->bs.bootSector.reservedSectors, g_data->bs.bootSector.sectorsPerFat, g_fat);
}

// Returns the calculated LBA number from the cluster number
uint32_t __FAT_clusterToLba(uint32_t cluster){
	return g_dataSectionLba + (cluster - 2) * g_data->bs.bootSector.sectorsPerCluster;
}

// Returns the next cluster of the currentCluster (by a lookup in the FAT)
uint32_t __FAT_nextCluster(uint32_t currentCluster){
	uint32_t fatIndex = currentCluster * 3/2;
	if (currentCluster % 2 == 0) return (*(uint16_t*)(g_fat + fatIndex)) & 0x0fff; // todo tester de remplacer la somme de pointeurs par &g_fat[fatIndex]
	else return (*(uint16_t*)(g_fat + fatIndex)) >> 4;
}

FAT_File* __FAT_openEntry(DISK* disk, FAT_DirectoryEntry* entry){
	int handle = -1;

	// Find free handle
	int i = 0;
	while(i<MAX_FILE_HANDLES && handle < 0){
		if(!g_data->openedFiles[i].isOpened) handle = i;
		i++;
	}

	// Out of handles
	if (handle < 0){
		printf("FAT Error: could't open directory '%s', because we're out of handles ", entry->name);
		printf("(files opened: %u/%u).\n", MAX_FILE_HANDLES, MAX_FILE_HANDLES);
		printf("Please close some files and try again.\n");
		return NULL;
	}

	// Setup file data and metadatas
	__FAT_FileInternal* fd = &g_data->openedFiles[handle];
	fd->metadata.handle = handle;
	fd->metadata.isDirectory = (bool) ((entry->attributes & FAT_ATTRIBUTE_DIRECTORY) != 0);
	fd->metadata.position = 0;
	fd->metadata.size = entry->size;
	fd->firstCluster = entry->firstClusterLow + (((uint32_t) entry->firstClusterHigh) << 16); // first cluster is split in two 2B integers, low and high
	fd->curCluster = fd->firstCluster;
	fd->curSectorInCluster = 0;

	bool res = DISK_readSectors(disk, __FAT_clusterToLba(fd->curCluster), 1, fd->buffer);
	if (!res){
		printf("FAT Error: Couldn't read directory '%s'\n", entry->name);
		return NULL;
	}

	fd->isOpened = true;
	return &fd->metadata;
}

bool __FAT_findFile(DISK* disk, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut){

	// Convert from name to fat name

	// converted name variable
	char fatName[] = "           ";

	const char* ext = strchr(name, '.'); // note: we only get the first '.'. If there are more, this probably messes up (a fix is in WIP just over)
	if (ext==NULL) ext = name + 11;

	// make the transformation
	for(int i=0 ; i<8 && name[i] && name+i<ext ; i++) fatName[i] = toupper(name[i]);
	// if we have a file extension, copy it
	if (ext != NULL) for(int i=0 ; i<3 && ext[i+1] ; i++) fatName[i+8] = toupper(ext[i+1]);
	
	// Search for the file

	FAT_DirectoryEntry entry;
	bool res;
	int i = 0; // hard limit on number of entries
	do {
		res = FAT_readEntry(disk, file, &entry);
		if (!res) return false;

		// Is it the entry we're looking for ?
		if (memcmp(fatName, entry.name, 11) == 0){
			*entryOut = entry;
			return true;
		}
	} while(i++ < 256);

	return false;
}

bool FAT_initalize(DISK* disk){
	g_data = (__FAT_Data*) MEMORY_FAT_ADDR;

	// Read disk's boot sector
	bool res = __FAT_readBootSector(disk);
	if(!res){
		printf("FAT error: readBootSector failed\n");
		return false;
	}

	// Verify that reading the FAT won't overflow the FAT Driver's reserved memory
	// Note: in the future, we should cache the FAT instead of aborting
	g_fat = (uint8_t*) g_data + sizeof(__FAT_Data);
	uint32_t fatSize = g_data->bs.bootSector.sectorsPerFat * g_data->bs.bootSector.bytesPerSector;
	if(sizeof(__FAT_Data) + fatSize >= MEMORY_FAT_SIZE){
		printf("FAT error: FAT is too big to fit into this driver's memory (required: %d, available: %d)\n", sizeof(__FAT_Data)+fatSize, MEMORY_FAT_SIZE);
		return false;
	}

	// Read FAT
	res = __FAT_readFat(disk);
	if(!res){
		printf("FAT error: reading disk's FAT failed\n");
		return false;
	}

	// Init root dir file
	uint32_t rootDirLba = g_data->bs.bootSector.reservedSectors + g_data->bs.bootSector.sectorsPerFat * g_data->bs.bootSector.fatCount;
	uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_data->bs.bootSector.dirEntryCount;
	g_data->rootDir.metadata.handle = ROOT_DIR_HANDLE;
	g_data->rootDir.metadata.isDirectory = true;
	g_data->rootDir.metadata.position = 0;
	g_data->rootDir.metadata.size = sizeof(FAT_DirectoryEntry) * g_data->bs.bootSector.dirEntryCount; // we do not account for the internal fragmentation (align)
	g_data->rootDir.isOpened = true;
	g_data->rootDir.firstCluster = rootDirLba;
	g_data->rootDir.curCluster = rootDirLba;
	g_data->rootDir.curSectorInCluster = 0;
	res = DISK_readSectors(disk, rootDirLba, 1, g_data->rootDir.buffer);
	if (!res){
		printf("FAT error: error reading root directory\n");
		return false;
	}

	// Compute data section
	uint32_t rootDirSectors = (rootDirSize + g_data->bs.bootSector.bytesPerSector - 1) / g_data->bs.bootSector.bytesPerSector;
	g_dataSectionLba = rootDirLba + rootDirSectors;

	// Reset opened files
	for(int i=0 ; i<MAX_FILE_HANDLES ; i++) g_data->openedFiles[i].isOpened = false;

	return true;
}

FAT_File* FAT_open(DISK* disk, const char* filepath){
	char name[MAX_PATH_SIZE];
	memset(name, '\0', MAX_PATH_SIZE); // Otherwise it's fucked during different executions

	if (filepath[0] == '/') filepath++; // ignore leading slash

	// We start searching from the root directory
	FAT_File* cur = &g_data->rootDir.metadata;

	while(*filepath) {
		bool isLast = false;
		const char* delim = strchr(filepath, '/');

		// There is another slash forward, so we have to extract the next filename
		if (delim != NULL){
			memcpy(name, filepath, delim - filepath);
			name[delim - filepath + 1] = '\0';
			filepath = delim + 1;
		}
		// There is no slash foward, we're at the last file or directory
		else {
			size_t len = strlen(filepath);
			memcpy(name, filepath, len);
			name[len + 1] = '\0';
			filepath += len;
			isLast = true;
		}

		// Find directory entry in current directory
		FAT_DirectoryEntry entry;
		bool res = __FAT_findFile(disk, cur, name, &entry);
		FAT_close(cur); // Close parent, we don't need it anymore
		if(!res){
			printf("FAT Error: couldn't find '%s'\n", name);
			return NULL;
		}

		// Check if directory
		if (!isLast && ((entry.attributes & FAT_ATTRIBUTE_DIRECTORY) == 0)){
			printf("FAT Error: '%s' isn't a directory !\n", name);
			return NULL;
		}

		// Open new entry
		cur = __FAT_openEntry(disk, &entry);
	}

	return cur;
}

uint32_t FAT_read(DISK* disk, FAT_File* file, uint32_t byteCount, void* dataOut){
	// get file data
	__FAT_FileInternal* fd = (file->handle == ROOT_DIR_HANDLE) ? &g_data->rootDir : &g_data->openedFiles[file->handle];
	uint8_t* u8dataOut = (uint8_t*) dataOut;

	// Don't read past the end of file
	// byteCount = min(byteCount, size-position)
	if (!fd->metadata.isDirectory){
		uint32_t temp = fd->metadata.size - fd->metadata.position;
		byteCount = (byteCount < temp) ? byteCount : temp;
	}

	while(byteCount > 0){
		uint32_t leftInBuffer = SECTOR_SIZE - (fd->metadata.position % SECTOR_SIZE);
		// take: the number of bytes to read
		uint32_t take = (byteCount < leftInBuffer) ? byteCount : leftInBuffer;

		memcpy(u8dataOut, fd->buffer + fd->metadata.position % SECTOR_SIZE, take);
		u8dataOut += take;
		fd->metadata.position += take;
		byteCount -= take;

		// See if we need to read more data

		if(leftInBuffer != take) continue;

		// Handle the special case of the root directory
		if (fd->metadata.handle == ROOT_DIR_HANDLE){
			++fd->curCluster;

			// read next sector
			bool res = DISK_readSectors(disk, fd->curCluster, 1, fd->buffer);
			if (!res){
				printf("FAT Error: couldn't read disk, at root directory level ");
				printf("(lba=%u n_sectors=%u dataOut=%p)\n", fd->curCluster, 1, fd->buffer);
				return 0;
			}
		}

		// Not root dir
		else {

			// Compute next cluster and sector to read
			if( ++fd->curCluster >= g_data->bs.bootSector.sectorsPerCluster){
				fd->curSectorInCluster = 0;
				fd->curCluster = __FAT_nextCluster(fd->curCluster);
			}

			// We are reading after the end of the file. This should not happen if the "size" field is correct.
			if (fd->curCluster >= 0xff8){
				fd->metadata.size = fd->metadata.position;
				break;
			}

			// read next sector
			bool res = DISK_readSectors(disk, __FAT_clusterToLba(fd->curCluster) + fd->curSectorInCluster, 1, fd->buffer);
			if (!res){
				// TODO handle error
				printf("FAT Error: couldn't read disk\n");
				break;
			}
		}
	}

	// Return bytes read
	return u8dataOut - (uint8_t*) dataOut;
}

bool FAT_readEntry(DISK* disk, FAT_File* file, FAT_DirectoryEntry* dirOut){
	// Commentary: "*forgot to read first cluster here"
	uint32_t sizeRead = FAT_read(disk, file, sizeof(FAT_DirectoryEntry), dirOut);
	return sizeRead == sizeof(FAT_DirectoryEntry);
}

void FAT_close(FAT_File* file){
	if (file == NULL) return;

	// Root directory
	if (file->handle == ROOT_DIR_HANDLE){
		file->position = 0;
		g_data->rootDir.curCluster = g_data->rootDir.firstCluster;
		// TODO read first cluster into the buffer
		return;
	}

	// Any other file
	g_data->openedFiles[file->handle].isOpened = false;
}
