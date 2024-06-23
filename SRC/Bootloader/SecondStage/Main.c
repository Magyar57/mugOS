#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "Tests.h"

void end(){
	// Just an infinite loop
	while(true){}
}

// Entry point for the second-stage bootloader C code.
// It is called by the Main.asm code (which is the entry point of the 2nd stage bootloader)
void __attribute__((cdecl)) start(uint32_t bootDrive){
	clear_screen();
	puts("Loading bootloader stage 2...\n");

	DISK d;
	bool res = DISK_initalize(&d, bootDrive);
	if(!res){
		puts("Error initalizing disk ; aborting.");
		end();
	}
	printf("bootDrive: %d\n", bootDrive);
	printf("Disk: id=%d cylinders=%d heads=%d sectors=%d\n\n", d.id, d.cylinders, d.heads, d.sectors);

	res = FAT_initalize(&d);
	if (!res){
		printf("Error initalizing FAT driver from disk (id=%i)\n", d.id);
		end();
	}

	// // Print all the files in the root directory
	FAT_File* fd = FAT_open(&d, "/");
	FAT_DirectoryEntry entry;
	int i = 0; // hard limit on number of entries to print
	while(FAT_readEntry(&d, fd, &entry) && i<6){
		bool isLFN = ((entry.attributes & FAT_ATTRIBUTE_LFN) != 0);
		if (isLFN) continue;
		
		bool isDir = ((entry.attributes & FAT_ATTRIBUTE_DIRECTORY) != 0);
		const char* type_prefix = (isDir) ? "d - " : "f - "; // defaults to file
		printf("%s", type_prefix);
		for(int j=0 ; j<8 ; j++) putc(entry.name[j]);
		putc(' ');
		for(int j=8 ; j<11 ; j++) putc(entry.name[j]);
		putc('\n');
		i++;
	}
	FAT_close(fd);
	puts("");

	// Read some files (note: rn reading two files in a row loops indefinitely, but subdirectory reading works)

	char buffer[256];
	uint32_t read;
	char* file;

	// file = "test.txt";
	// printf("Reading '%s' file:\n", file);
	// fd = FAT_open(&d, file);
	// if (fd != NULL){
	// 	while(read = FAT_read(&d, fd, sizeof(buffer), buffer)){
	// 		for(uint32_t i=0 ; i<read ; i++) putc(buffer[i]);
	// 	}
	// }
	// FAT_close(fd);
	// puts("");

	file = "dir/test_sub.txt";
	printf("Reading '%s' file:\n", file);
	fd = FAT_open(&d, file);
	if (fd != NULL){
		do {
			read = FAT_read(&d, fd, sizeof(buffer), buffer);
			for(uint32_t i=0 ; i<read ; i++) putc(buffer[i]);
		} while(read);
	}
	FAT_close(fd);
	puts("");

	// test_printf();
	end();
}
