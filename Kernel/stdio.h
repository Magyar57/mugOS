#ifndef __STDIO_H__
#define __STDIO_H__

#include "FILE.h"

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
#define stdin stdin
#define stdout stdout
#define stderr stderr

// Write the char c to the screen
void putc(char c);

// Write the null-terminated string str to the screen.
void puts(const char* str);

// Prints a formatted string to the screen.
void printf(const char* formatted_string, ...);

// int fileno(FILE* stream);
// FILE *fopen(const char *restrict pathname, const char *restrict mode);
// FILE *fdopen(int fd, const char *mode);
// FILE *freopen(const char *restrict pathname, const char *restrict mode, FILE *restrict stream);

#endif
