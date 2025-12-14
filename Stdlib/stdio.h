#ifndef __STDIO_H__
#define __STDIO_H__

#include <stddef.h>
#include <stdarg.h>
#include "FILE.h"

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
#define stdin stdin
#define stdout stdout
#define stderr stderr

// The value returned by fgetc and similar functions to indicate the end of the file
#define EOF (-1)

int fileno(FILE* stream);
// FILE* fopen(const char *restrict pathname, const char *restrict mode);
// FILE* fdopen(int fd, const char *mode);
// FILE* freopen(const char *restrict pathname, const char *restrict mode, FILE *restrict stream);
// int fflush(FILE* _Nullable stream);

/// @brief Writes the char c, cast to an unsigned char, to stream
int fputc(int c, FILE* stream);

/// @brief Equivalent to fputc
int putc(int c, FILE* stream);

/// @brief Equivalent to `fputc(c, stdout)`
int putchar(int c);

/// @brief Writes s to the stream, without its terminating null byte '\0'
int fputs(const char* restrict s, FILE* restrict stream);

/// @brief Writes s to stdout, with a trailing newline
int puts(const char *s);

__attribute__((format(printf, 1, 2)))
int printf(const char* restrict format, ...);

__attribute__((format(printf, 2, 3)))
int fprintf(FILE* restrict stream, const char* restrict format, ...);

__attribute__((format(printf, 2, 3)))
int dprintf(int fd, const char* restrict format, ...);

__attribute__((format(printf, 2, 3)))
int sprintf(char* restrict str, const char* restrict format, ...);

__attribute__((format(printf, 3, 4)))
int snprintf(char* restrict str, size_t size, const char* restrict format, ...);

int vprintf(const char* restrict format, va_list args);
int vfprintf(FILE* restrict stream, const char* restrict format, va_list args);
int vdprintf(int fd, const char* restrict format, va_list args);
int vsprintf(char* restrict str, const char* restrict format, va_list args);
int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list args);

#endif
