#include <stdio.h>

#include "Fat.h"

int main(int argc, const char** argv){
	if (argc < 3){
		printf("Syntax error. Usage: %s <disk image> </path/to/file.txt>\n", argv[0]);
		return 1;
	}

	// Open the disk
	FILE* disk = fopen(argv[1], "rb");
	if (!disk){
		fprintf(stderr, "Could not open disk image %s !\n", argv[1]);
		return 2;
	}

	// Read the disk's bootsector
	BootSector fat_bs;
	bool couldRead = readBootSector(disk, &fat_bs);
	if (!couldRead){
		fprintf(stderr, "Could not read '%s' boot sector !\n", argv[1]);
		return 3;
	}
	
	printBootSectorInformations(fat_bs);

	// Read the disk's FAT
	uint8_t* fat = NULL;
	couldRead = readFat(disk, fat_bs, &fat);
	if(!couldRead){
		fprintf(stderr, "Could not read FAT (File Allocation Table) !\n");
		free(fat);
		return 4;
	}

	printf("Successfully read FAT !\n");
	
	// Read the disk's root directory
	DirectoryEntry* directories = NULL;
	couldRead = readRootDirectory(disk, fat_bs, &directories);
	if(!couldRead){
		fprintf(stderr, "Could not read root directory !\n");
		free(fat);
		if (directories != NULL) free(directories);
		return 5;
	}

	printf("Successfully read root directory !\n");

	// Find the file in the directories entries
	const char* filename = argv[2];
	DirectoryEntry* file_location = findFile(fat_bs, directories, filename);
	if (file_location == NULL){
		fprintf(stderr, "The file '%s' does not exist on the disk ! \n", filename);
		fprintf(stderr, "Maybe the filename is wrong ? It should be FAT12 compatible, aka 11 char long, padded with spaces.\n");
		fprintf(stderr, "For example: 'TEST    TXT'\n");
		free(fat);
		free(directories);
		return 6;
	}

	printf("Successfully found '%s' on disk !\n", filename);

	// Read the file contents from disk
	uint8_t* file_buffer = (uint8_t*) malloc(file_location->Size + fat_bs.BytesPerSector);
	bool res = readFile(disk, fat_bs, fat, file_location, file_buffer);
	if (!res){
		fprintf(stderr, "An error occured during the reading of the file '%s' !\n", filename);
		free(fat);
		free(directories);
		free(file_buffer);
		return 7;
	}

	printf("Successfully read file from disk !\n");
	
	// Print the file content
	printf("Here is the file contents:\n");
	for(size_t i=0 ; i<file_location->Size ; i++){
		if (isprint(file_buffer[i])) fputc(file_buffer[i], stdout); // if the char is printable, print it
		else printf("<%02x>", file_buffer[i]); // else print its hex value
	}
	printf("\n");

	free(fat);
	free(directories);
	free(file_buffer);
	return 0;
}
