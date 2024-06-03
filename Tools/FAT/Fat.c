#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Fat.h"

bool readBootSector(FILE* disk, BootSector* output){
	size_t sizeRead = fread(output, sizeof(BootSector), 1, disk);
	return (sizeRead > 0);
}

bool readSectors(FILE* disk, BootSector bs, uint32_t lba, uint32_t count, void* buffer_out){
	
	// Set cursor position to where we have to read
	int fseek_res = fseek(disk, lba*bs.BytesPerSector, SEEK_SET);
	if (fseek_res != 0) return false;

	// Read
	size_t fread_res = fread(buffer_out, bs.BytesPerSector, count, disk);
	return (fread_res == count);
}

bool readFat(FILE* disk, BootSector bs, uint8_t** fat_out){
	int fatSize = bs.SectorsPerFat * bs.BytesPerSector;
	*fat_out = (uint8_t*) malloc(fatSize);

	// lba = bs.ReservedSectors because the FAT starts just after the reserved sectors
	bool couldRead = readSectors(disk, bs, bs.ReservedSectors, bs.SectorsPerFat, *fat_out); 
	return couldRead;
}

void printBootSectorInformations(BootSector bs){
	printf("Read bootsector informations:\n");
	printf("BootJumpInstruction\t %o %o %o \n", bs.BootJumpInstruction[0], bs.BootJumpInstruction[1], bs.BootJumpInstruction[2]);
	printf("OemIdentifier\t\t %s \n", 			bs.OemIdentifier);
	printf("BytesPerSector\t\t %u \n", 			bs.BytesPerSector);
	printf("SectorsPerCluster\t %u \n", 		bs.SectorsPerCluster);
	printf("ReservedSectors\t\t %u \n", 		bs.ReservedSectors);
	printf("FatCount\t\t %o \n", 				bs.FatCount);
	printf("DirEntryCount\t\t %u \n", 			bs.DirEntryCount);
	printf("TotalSectors\t\t %u \n", 			bs.TotalSectors);
	printf("MediaDescriptorType\t %u \n", 		bs.MediaDescriptorType);
	printf("SectorsPerFat\t\t %u \n", 			bs.SectorsPerFat);
	printf("SectorsPerTrack\t\t %u \n", 		bs.SectorsPerTrack);
	printf("Heads\t\t\t %u \n", 				bs.Heads);
	printf("HiddenSectors\t\t %u \n", 			bs.HiddenSectors);
	printf("LargeSectorCount\t %u \n", 			bs.LargeSectorCount);

	printf("DriveNumber\t\t %u \n", 			bs.DriveNumber);
	printf("_Reserved\t\t %u \n", 				bs._Reserved);
	printf("Signature\t\t %u \n", 				bs.Signature);
	printf("VolumeId\t\t %#x \n", 				bs.VolumeId);
	printf("VolumeLabel\t\t %.*s \n", 			(int) sizeof(bs.VolumeLabel), bs.VolumeLabel); // print a non null-terminated string
	printf("SystemId\t\t %.*s \n", 				(int) sizeof(bs.SystemId), bs.SystemId);
	
	printf("\n");
}

void parseDate(uint16_t date, int* year, int* month, int* day){
	*year = 1980 + 	((date & 0b1111111000000000) >> 9);
	*month =		 (date & 0b0000000111100000) >> 5;
	*day =			 (date & 0b0000000000011111) >> 0;
}

void parseTime(uint16_t time, int* hours, int* minutes, int* seconds){
	*hours =		(time & 0b1111100000000000) >> 11;
	*minutes =		(time & 0b0000011111100000) >> 5;
	*seconds =		(time & 0b0000000000011111) >> 0;
}

void parseLFNDirectoryEntryName(const LFN_DirectoryEntry entry, char* name_out){
	// Note: we assume that name_out is 13 elements long...
	// Note 2: we ignore the higher byte for simplicity sake (I don't want to implement wprintf and stuff)

	// Fill name with spaces
	int n = ( (sizeof(entry.name_begin) + sizeof(entry.name_middle) + sizeof(entry.name_end)) / sizeof(uint16_t)); // Should be 13
	for(int i=0 ; i<n; i++) name_out[i] = ' ';

	// Parse name
	int n1 = sizeof(entry.name_begin)/sizeof(uint16_t); // 5
	int n2 = sizeof(entry.name_middle)/sizeof(uint16_t); // 6
	int n3 = sizeof(entry.name_end)/sizeof(uint16_t); // 2
	for(int i=0 ; i<n1 ; i++) name_out[i] = ((char*)(entry.name_begin))[2*i]; // name_bytes[2*i+1] ignored
	for(int i=0 ; i<n2 ; i++) name_out[n1+i] = ((char*)(entry.name_middle))[2*i];
	for(int i=0 ; i<n3 ; i++) name_out[n1+n2+i] = ((char*)(entry.name_end))[2*i];
}

void printDirectoryEntryName(DirectoryEntry entry){
	bool isDirectory = ((entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0);
	bool isLongFileName = ((entry.Attributes == FAT_ATTRIBUTE_LFN));

	char dirChar = (isDirectory) ? 'd' : '-';

	// LFN entry
	if (isLongFileName) {
		LFN_DirectoryEntry lfn_entry = *((LFN_DirectoryEntry*) &entry); // casts uses pointers because direct cast throws error
		char lfn_name[26];
		parseLFNDirectoryEntryName(lfn_entry, lfn_name);
		printf("%c (LFN) '%.*s'\n", dirChar, (int) sizeof(lfn_name), lfn_name);
		return;
	}

	printf("%c '%.*s'\n", dirChar, (int) sizeof(entry.Name), entry.Name);
}

void printDirectoryEntryInformations(DirectoryEntry entry){
	bool isReadonly = ((entry.Attributes & FAT_ATTRIBUTE_READ_ONLY) != 0);
	bool isHidden = ((entry.Attributes & FAT_ATTRIBUTE_HIDDEN) != 0);
	bool isSystemFile = ((entry.Attributes & FAT_ATTRIBUTE_SYSTEM) != 0);
	bool isVolumeId = ((entry.Attributes & FAT_ATTRIBUTE_VOLUME_ID) != 0);
	bool isDirectory = ((entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0);
	bool isArchive = ((entry.Attributes & FAT_ATTRIBUTE_ARCHIVE) != 0);
	bool isLongFileName = ((entry.Attributes == FAT_ATTRIBUTE_LFN));
	
	if (isLongFileName) {
		LFN_DirectoryEntry lfn_entry = *((LFN_DirectoryEntry*) &entry); // casts uses pointers because direct cast throws error
		char name[13];
		parseLFNDirectoryEntryName(lfn_entry, name);

		const char* entry_type_string = "UNKNOWN";
		if (lfn_entry.entry_type == 0) entry_type_string = "NAME";
		
		printf("LFN DirectoryEntry '%.*s' (LFN entry type: %s)\n", (int) sizeof(name), name, entry_type_string);
		return;
	}

	printf("DirectoryEntry '%.*s' informations:\n", (int) sizeof(entry.Name), entry.Name);
	printf("Attributes: %#x => ", entry.Attributes);
	if (isReadonly) printf("READ_ONLY ");
	if (isHidden) printf("HIDDEN ");
	if (isSystemFile) printf("SYSTEM ");
	if (isVolumeId) printf("VOLUME_ID ");
	if (isDirectory) printf("DIRECTORY ");
	if (isArchive) printf("ARCHIVE ");
	printf("\n");

	// Creation date and time
	int createdYear, createdMonth, createdDay, createdHours, createdMinutes, createdSeconds;
	parseDate(entry.CreatedDate, &createdYear, &createdMonth, &createdDay);
	parseTime(entry.CreatedTime, &createdHours, &createdMinutes, &createdSeconds);

	printf("Created:\t %.2d/%.2d/%.4d at %.2d:%.2d (and %.2ds %.3d0ms)\n", 
			createdDay, createdMonth, createdYear, createdHours, createdMinutes, createdSeconds, entry.CreatedTimeTenths);

	int accessedYear, accessedMonth, accessedDay;
	parseDate(entry.AccessedDate, &accessedYear, &accessedMonth, &accessedDay);
	printf("Accessed:\t %.2d/%.2d/%.4d\n", accessedDay, accessedMonth, accessedYear);

	int modifiedYear, modifiedMonth, modifiedDay, modifiedHours, modifiedMinutes, modifiedSeconds;
	parseDate(entry.ModifiedDate, &modifiedYear, &modifiedMonth, &modifiedDay);
	parseTime(entry.ModifiedTime, &modifiedHours, &modifiedMinutes, &modifiedSeconds);
	printf("Modified:\t %.2d/%.2d/%.4d at %.2d:%.2d (and %.2ds)\n",
			modifiedDay, modifiedMonth, modifiedYear, modifiedHours, modifiedMinutes, modifiedSeconds);

	int clusterNumber = (entry.FirstClusterHigh << 16) + entry.FirstClusterLow;
	printf("Location on disk: cluster number %d\n", clusterNumber);

	printf("Size: %dB\n", entry.Size);
}

bool readRootDirectory(FILE* disk, BootSector bs, DirectoryEntry** dir_out){
	// Where we start reading
	uint32_t lba = bs.ReservedSectors + bs.FatCount*bs.SectorsPerFat;
	
	// Total size of the entries
	uint32_t size = sizeof(DirectoryEntry)*bs.DirEntryCount; // in bytes
	// Translate in sectors count for the read
	uint32_t sectors = size / bs.BytesPerSector;
	if (size % bs.BytesPerSector > 0) sectors++; // round up number of sectors

	// Allocate enough memory for all directories entries
	// Note: We do not allocate size bytes because we rounded up the number of sectors for the read
	// The read wouldn't read the last bytes but rather the entire sector, so it would be an overflow of our dir_out (seg fault)
	*dir_out = (DirectoryEntry*) malloc(sectors * bs.BytesPerSector);
	
	bool res = readSectors(disk, bs, lba, sectors, *dir_out);
	return res;
}

DirectoryEntry* findFile(BootSector bs, DirectoryEntry* dir, const char* name){
	for(uint16_t i = 0 ; i<bs.DirEntryCount ; i++){
		int compare_res = memcmp(name, dir[i].Name, 11);
		if (compare_res == 0){
			return &(dir[i]);
		}
	}
	return NULL;
}

bool readFile(FILE* disk, BootSector bs, uint8_t *fat, DirectoryEntry* fileEntry, uint8_t* output_file){
	// Calculate where the root directory ends
	uint32_t lba = bs.ReservedSectors + bs.FatCount*bs.SectorsPerFat;
	uint32_t size = sizeof(DirectoryEntry)*bs.DirEntryCount; // in bytes
	uint32_t sectors = size / bs.BytesPerSector;
	if (size % bs.BytesPerSector > 0) sectors++; // round up number of sectors
	uint32_t rootDirectoryEnd = lba + sectors;

	bool ok = true;
	uint16_t currentCluster = fileEntry->FirstClusterLow;
	do {
		// currentCluster-2 : the first two clusters are reserved
		uint32_t lba = rootDirectoryEnd + (currentCluster-2)*bs.SectorsPerCluster;
		ok = readSectors(disk, bs, lba, bs.SectorsPerCluster, output_file);
		output_file += bs.SectorsPerCluster; // update pointer to point at where the data ends

		// To determine where the next cluster is, we lookup in the FAT
		uint32_t fatIndex = currentCluster + currentCluster/2; // indexes are 12 bits (1 byte and a half)
		if (currentCluster % 2 == 0) currentCluster = (*(uint16_t*) (fat + fatIndex)) & 0x0fff; // select only bottom 12 bits
		else currentCluster = (*(uint16_t*) (fat + fatIndex)) >> 4; // select only upper 12 bits

	} while (ok && currentCluster < 0xff8); // A "next cluster" value of 0xff8 or bigger means that we reached the end of the file's cluster chain

	return ok;
}
