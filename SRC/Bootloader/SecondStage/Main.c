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

	DISK disk;
	bool res = DISK_initalize(&disk, bootDrive);
	if(!res){
		puts("Error initalizing disk ; aborting.");
		end();
	}
	printf("bootDrive: %d\n", bootDrive);
	printf("Disk: id=%d cylinders=%d heads=%d sectors=%d\n\n", disk.id, disk.cylinders, disk.heads, disk.sectors);

	res = FAT_initalize(&disk);
	if (!res){
		printf("Error initalizing FAT driver from disk (id=%i)\n", disk.id);
		end();
	}

	// test_printf();
	test_printDirectory(disk, "/", 5);
	test_printDirectory(disk, "/dir", 4);
	test_FAT_Open_and_Read(disk, "test.txt");
	test_FAT_Open_and_Read(disk, "dir/test_sub.txt");
	end();
}
