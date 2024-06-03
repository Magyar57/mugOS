#ifndef __STDIO_H__
#define __STDIO_H__

#define NULL ((void *)0)

// Write the char c to the screen
void putc(char c);

// Write the string str to the screen.
// The string has to be null-terminated (which is in the C specifications, apparently)
void puts(const char* str);

// Prints a formatted string to the screen.
// Suppported formats are:
// - %d or %i
//
// Supported options are:
// - yes
void _cdecl printf(const char* formatted_string, ...);

#endif
