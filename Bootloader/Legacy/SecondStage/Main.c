#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "stdio.h"
#include "string.h"
#include "disk.h"
#include "fat.h"
#include "Tests.h"
#include "memdefs.h"

typedef void (*KernelMain)();

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

	// // Load the kernel
	FAT_File* kernel_file = FAT_open(&disk, "/kernel.bin");
	uint32_t read;
	uint8_t* positionInTargetMemory = (uint8_t*) MEMORY_KERNEL_ADDR;
	printf("Loading kernel into buffer at %p and copying at address %p\n", MEMORY_KERNEL_BUFFER_ADDR, positionInTargetMemory);
	while( (read = FAT_read(&disk, kernel_file, MEMORY_KERNEL_BUFFER_SIZE, MEMORY_KERNEL_BUFFER_ADDR)) ){
		memcpy(positionInTargetMemory, MEMORY_KERNEL_BUFFER_ADDR, read);
		positionInTargetMemory += read;
	}
	FAT_close(kernel_file);
	puts("Kernel loaded !\n");

	// Call the kernel
	puts("Jumping to the kernel. Goodbye...");
	KernelMain kernel_main = (KernelMain) MEMORY_KERNEL_ADDR;
	kernel_main();

	puts("The kernel returned. WTF ? Halting.");
	end();

	// test_printf();
	// test_printDirectory(disk, "/", 5);
	// test_printDirectory(disk, "/dir", 4);
	// test_FAT_Open_and_Read(disk, "test.txt");
	// test_FAT_Open_and_Read(disk, "dir/test_sub.txt");
}
