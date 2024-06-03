#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "Tests.h"

void end(){
	while(true){
		// Just an infinite loop
	}
}

// Entry point for the second-stage bootloader C code.
// It is called by the Main.asm code (which is the entry point of the 2nd stage bootloader)
void _cdecl cstart_(uint16_t bootDrive){
	puts("Loading bootloader stage 2...\r\n\n");

	DISK d;
	bool res = DISK_initalize(&d, bootDrive);
	if(!res){
		printf("Error initalizing disk ; aborting.\r\n");
		end();
	}
	printf("bootDrive: %d\r\n", bootDrive);
	printf("Disk : id=%d cylinders=%d heads=%d sectors=%d\r\n\n", d.id, d.cylinders, d.heads, d.sectors);

	res = FAT_initalize(&d);
	if (!res){
		printf("Error initalizing FAT driver from disk (id=%i)\r\n", d.id);
		end();
	}

	// Print all the files in the root directory
	FAT_File far* fd = FAT_open(&d, "/");
	FAT_DirectoryEntry entry;
	int i = 0; // hard limit on number of entries to print
	while(FAT_readEntry(&d, fd, &entry) && i<6){
		bool isDir = ((entry.attributes & FAT_ATTRIBUTE_DIRECTORY) != 0);
		puts(isDir ? "d - " : "f - ");
		for(int j=0 ; j<11 ; j++) putc(entry.name[j]);
		puts("\r\n");
		i++;
	}
	puts("\r\n");
	FAT_close(fd);

	// Read some files (note: rn reading two files in a row loops indefinitely, but subdirectory reading works)

	char buffer[256];
	uint32_t read;
	char* file;
	
	file = "test.txt";
	printf("Reading '%s' file:\r\n", file);
	fd = FAT_open(&d, file);
	if (fd != NULL){
		while(read = FAT_read(&d, fd, sizeof(buffer), buffer)){
			for(uint32_t i=0 ; i<read ; i++) putc(buffer[i]);
		}
	}
	puts("\r\n");
	FAT_close(fd);

	// file = "dir/test_sub.txt";
	// printf("Reading '%s' file:\r\n", file);
	// fd = FAT_open(&d, file);
	// if (fd != NULL){
	// 	while(read = FAT_read(&d, fd, sizeof(buffer), buffer)){
	// 		for(uint32_t i=0 ; i<read ; i++) putc(buffer[i]);
	// 	}
	// }
	// puts("\r\n");
	// FAT_close(fd);

	// test_printf();
	end();
}
