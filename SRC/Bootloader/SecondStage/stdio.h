#ifndef __STDIO_H__
#define __STDIO_H__

// Write the char c to the screen
void putc(char c);

// Write the null-terminated string str to the screen.
void puts(const char* str);

// Prints a formatted string to the screen.
void printf(const char* formatted_string, ...);

// Clear the screen
void clear_screen();

#endif
