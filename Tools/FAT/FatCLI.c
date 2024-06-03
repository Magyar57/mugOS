#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Fat.h"

// TODO
void printDir(BootSector bs, uint8_t* fat){
	// DirectoryEntry entry;

	// uint8_t* sector_buffer = (uint8_t*) malloc(bs.BytesPerSector * sizeof(uint8_t));
	// if (sector_buffer == NULL) {
	// 	fprintf(stderr, "Error (printDir): could not allocate enough memory to buffer a sector\n");
	// 	return;
	// }

	// for(uint16_t i = 0 ; i<bs.DirEntryCount ; i++){
	// 	if (directories[i].Name[0] == '\0') break; // If the first byte of an entry is 0, we reached the end of the entries
	// 	printDirectoryEntryInformations(directories[i]);
	// 	printf("\n");
	// }

	// free(sector_buffer);
}

int main(int argc, const char** argv){
	if (argc < 2){
		printf("Syntax error. Usage: %s <disk image>\n", argv[0]);
		return 1;
	}

	// Open the disk
	FILE* disk = fopen(argv[1], "rb");
	if (!disk){
		fprintf(stderr, "Could not open disk image %s !\n", argv[1]);
		return 2;
	}

	// Read the disk's bootsector
	BootSector bs;
	bool couldRead = readBootSector(disk, &bs);
	if (!couldRead){
		fprintf(stderr, "Could not read '%s' boot sector !\n", argv[1]);
		return 3;
	}
	
	// printBootSectorInformations(bs);

	// Read the disk's FAT
	uint8_t* fat = NULL;
	couldRead = readFat(disk, bs, &fat);
	if(!couldRead){
		fprintf(stderr, "Could not read FAT (File Allocation Table) !\n");
		free(fat);
		return 4;
	}

	printf("Successfully read FAT !\n");
	
	// Read the disk's root directory
	DirectoryEntry* directories = NULL;
	couldRead = readRootDirectory(disk, bs, &directories);
	if(!couldRead){
		fprintf(stderr, "Could not read root directory !\n");
		free(fat);
		if (directories != NULL) free(directories);
		return 5;
	}

	printf("Successfully read root directory !\n");

	printf("Welcome on the FAT CLI. Type 'help' for help informations.\n");
	
	char cwd[1024] = {'/'};
	char buffer[1024];
	while(true){
		printf("fat12 %s > ", cwd);
		char* res = fgets(buffer, sizeof(buffer), stdin);
		if (res == NULL) {printf("exit\n"); break;}

		buffer[strcspn(buffer, "\n")] = '\0'; // replace the '\n' with a '\0' to null-terminate the input

		if (strcmp(buffer, "exit") == 0) break;
		if (strcmp(buffer, "") == 0) continue;

		if (strcmp(buffer, "help") == 0){
			printf("Supported commands: help ls file cd pwd cat exit\n");
			continue;
		}

		if (strcmp(buffer, "pwd") == 0){
			printf("%s\n", cwd);
			continue;
		}

		if (strcmp(buffer, "ls")==0 || strncmp(buffer, "ls ", 3)==0){
			// TODO
			printf("todo implement properly ls\n");
			for(uint16_t i = 0 ; i<bs.DirEntryCount ; i++){
				if (directories[i].Name[0] == '\0') break; // If the first byte of an entry is 0, we reached the end of the entries
				printDirectoryEntryName(directories[i]);
			}
			continue;
		}

		if (strncmp(buffer, "file", 4) == 0){
			// if (buffer[5]=='\0'){
			// 	printf("Error, not file to concatenate provided. Usage: file <file.eg>\n");
			// 	continue;
			// }
			// TODO
			printf("todo implement file properly\n");
			for(uint16_t i = 0 ; i<bs.DirEntryCount ; i++){
				if (directories[i].Name[0] == '\0') break; // If the first byte of an entry is 0, we reached the end of the entries
				printDirectoryEntryInformations(directories[i]);
				printf("\n");
			}
			continue;
		}

		if (strncmp(buffer, "cat", 3) == 0){
			if (buffer[4]=='\0'){
				printf("Error, not file to concatenate provided. Usage: cat <file.eg>\n");
				continue;
			}
			// TODO
			printf("todo implement cat\n");
			continue;
		}

		printf("Error: unrecognized command '%s'\n", buffer);
	}

	return 0;

	free(fat);
	free(directories);
	return 0;
}
