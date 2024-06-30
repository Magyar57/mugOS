#ifndef __FAT_H__
#define __FAT_H__

#include <stdint.h>

#include "disk.h"

enum FAT_Attributes {
	FAT_ATTRIBUTE_READ_ONLY = 0x01,
	FAT_ATTRIBUTE_HIDDEN = 0x02,
	FAT_ATTRIBUTE_SYSTEM = 0x04,
	FAT_ATTRIBUTE_VOLUME_ID = 0x08,
	FAT_ATTRIBUTE_DIRECTORY = 0x10,
	FAT_ATTRIBUTE_ARCHIVE = 0x20,
	FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

// FAT12 directory entry
typedef struct {
	uint8_t name[11];
	uint8_t attributes;
	uint8_t _reserved;
	uint8_t createdTimeTenths;
	uint16_t createdTime;
	uint16_t createdDate;
	uint16_t accessedDate;
	uint16_t firstClusterHigh;
	uint16_t modifiedTime;
	uint16_t modifiedDate;
	uint16_t firstClusterLow;
	uint32_t size;
} __attribute__((packed)) FAT_DirectoryEntry;

// FAT12 File handle
typedef struct {
	int handle;
	bool isDirectory;
	uint32_t position;
	uint32_t size;
}  __attribute__((packed)) FAT_File;

// Initalizes a FAT formatted disk
bool FAT_initalize(DISK* disk);

// Open a file (given its file 'filepath')
FAT_File* FAT_open(DISK* disk, const char* filepath);

// Read byteCount bytes from a file on disk, into dataOut
uint32_t FAT_read(DISK* disk, FAT_File* file, uint32_t byteCount, void* dataOut);

// Read the next entry from the directory 'file', into dirOut
bool FAT_readEntry(DISK* disk, FAT_File* file, FAT_DirectoryEntry* dirOut);

// Closes a file
void FAT_close(FAT_File* file);

#endif
