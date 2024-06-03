#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// FAT12 BootSector informations
// Note: __attribute__((packed)) is a gcc compiler instruction to tell it not to pad the structure with empty spaces
// (it does so to improve some operation performances, but in our case it would read too much bytes of our FAT image file)
typedef struct {
	uint8_t BootJumpInstruction[3];
	uint8_t OemIdentifier[8];
	uint16_t BytesPerSector;
	uint8_t SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t FatCount;
	uint16_t DirEntryCount;
	uint16_t TotalSectors;
	uint8_t MediaDescriptorType;
	uint16_t SectorsPerFat;
	uint16_t SectorsPerTrack;
	uint16_t Heads;
	uint32_t HiddenSectors;
	uint32_t LargeSectorCount;

	// extended boot record
	uint8_t DriveNumber;
	uint8_t _Reserved;
	uint8_t Signature;
	uint32_t VolumeId; // serial number, value doesn't matter
	uint8_t VolumeLabel[11]; // 11 bytes, padded with spaces
	uint8_t SystemId[8];

	// ...and we don't care about code

} __attribute__((packed)) BootSector;

// FAT12 directory entry
typedef struct {
	uint8_t Name[11];
	uint8_t Attributes;
	uint8_t _Reserved;
	uint8_t CreatedTimeTenths;
	uint16_t CreatedTime;
	uint16_t CreatedDate;
	uint16_t AccessedDate;
	uint16_t FirstClusterHigh;
	uint16_t ModifiedTime;
	uint16_t ModifiedDate;
	uint16_t FirstClusterLow;
	uint32_t Size;
} __attribute__((packed)) DirectoryEntry;

typedef struct {
	uint8_t order;				// The order of this entry in the sequence of long file name entries
	uint16_t name_begin[5];		// The first 5, 2-byte characters of this entry
	uint8_t attribute;			// Attribute. Always equals 0x0f (the long file name attribute)
	uint8_t entry_type;			// Long entry type
	uint8_t checksum;			// Checksum generated of the short file name when the file was created
	uint16_t name_middle[6];	// The next 6, 2-byte characters of this entry
	uint8_t zero[2];			// Always zero
	uint16_t name_end[2];		// The final 2, 2-byte characters of this entry
} __attribute__((packed)) LFN_DirectoryEntry;

enum FAT_Attributes {
	FAT_ATTRIBUTE_READ_ONLY = 0x01,
	FAT_ATTRIBUTE_HIDDEN = 0x02,
	FAT_ATTRIBUTE_SYSTEM = 0x04,
	FAT_ATTRIBUTE_VOLUME_ID = 0x08,
	FAT_ATTRIBUTE_DIRECTORY = 0x10,
	FAT_ATTRIBUTE_ARCHIVE = 0x20,
	FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

bool readBootSector(FILE* disk, BootSector* output);

// Reads "count" (n) sectors sectors from the disk (with the associated BootSector bs), starting from lba, into buffer_out
bool readSectors(FILE* disk, BootSector bs, uint32_t lba, uint32_t count, void* buffer_out);

// Reads the FAT from disk (with the associated BootSector bs) into fat_out
// *fat_out will be reallocated, so its memory needs to be freed or saved elsewhere
bool readFat(FILE* disk, BootSector bs, uint8_t** fat_out);

void printBootSectorInformations(BootSector bs);
void parseDate(uint16_t date, int* year, int* month, int* day);
void parseTime(uint16_t time, int* hours, int* minutes, int* seconds);
void parseLFNDirectoryEntryName(const LFN_DirectoryEntry entry, char* name_out);
void printDirectoryEntryName(DirectoryEntry entry);
void printDirectoryEntryInformations(DirectoryEntry entry);

// Reads the disk's root directory (with the associated BootSector bs) into dir_out
// *dir_out will be reallocated, so its memory needs to be freed or saved elsewhere
// Note: It will be allocated to the number of directories on the disk !
bool readRootDirectory(FILE* disk, BootSector bs, DirectoryEntry** dir_out);

// Finds a file named "name" in a directory array *dir, on a disk descried by a BootSector bs
DirectoryEntry* findFile(BootSector bs, DirectoryEntry* dir, const char* name);

// Reads a file from an entry fileEntry, into the bytes buffer ouput_file. Does so from a disk disk described by a BootSector bs and with the FAT fat
bool readFile(FILE* disk, BootSector bs, uint8_t *fat, DirectoryEntry* fileEntry, uint8_t* output_file);
