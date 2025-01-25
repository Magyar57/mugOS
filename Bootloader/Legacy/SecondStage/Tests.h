#ifndef __TESTS_H__
#define __TESTS_H__

#include "stdio.h"
#include "disk.h"

void test_printf();

// Print entries in the directory at dirpath
void test_printDirectory(DISK disk, const char* dirpath, int max_entries);

// Print the content of the file at 'filepath'
void test_FAT_Open_and_Read(DISK disk, const char* filepath);

#endif
