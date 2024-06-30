#include <stddef.h>
#include <stdbool.h>
#include "stdio.h"
#include "disk.h"
#include "fat.h"

#include "Tests.h"

void test_printf(){
	printf("Testing printf:\r\n");
	printf("%% %c %s ", 'a', "my_string");
	printf("%d %i %x %p %o ", 1234, -5678, 0x7fff, 0xbeef, 012345);
	printf("%hd %hi %hhu %hhd\r\n", (short)57, (short)-42, (unsigned char) 20, (char)-10);
	printf("%ld %lx %lld %llx\r\n\n", -100000000l, 0x7ffffffful, 10200300400ll, 0xeeeeaaaa7777ffffull);
}

void test_FAT_Open_and_Read(DISK disk, const char* filepath){
	char buffer[256];
	uint32_t read;

	printf(">> Reading '%s' file:\n", filepath);

	FAT_File* file_descriptor = FAT_open(&disk, filepath);
	if (file_descriptor != NULL){
		do {
			read = FAT_read(&disk, file_descriptor, sizeof(buffer), buffer);
			for(uint32_t i=0 ; i<read ; i++) putc(buffer[i]);
		} while(read);
	}
	FAT_close(file_descriptor);
	puts("");
}

void test_printDirectory(DISK disk, const char* dirpath, int max_entries){
	printf(">> Printing directory '%s':\n", dirpath);

	FAT_File* file_descriptor = FAT_open(&disk, dirpath);
	FAT_DirectoryEntry entry;

	int i = 0;
	while(FAT_readEntry(&disk, file_descriptor, &entry) && i<max_entries){
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
	FAT_close(file_descriptor);
	puts("");
}
