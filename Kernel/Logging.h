#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stddef.h>

#define DEBUG 0
#define SUCCESS 1
#define INFO 2
#define WARNING 3
#define ERROR 4
#define PANIC 5

void Logging_log(int logLevel, const char* moduleName, const char* logFmtStr, ...);

/// @brief Dump the memory (in hexadecimal) in the console
void Logging_hexdump(int logLevel, const char* moduleName, void* addr , size_t n);

/// @brief Alias for Logging_log
#define log Logging_log

/// @brief Alias for `Logging_log`, with default arguments.
/// Handy for writing quick logs while debugging
#define debug(fmt, ...) Logging_log(DEBUG, __FILE__, fmt, ##__VA_ARGS__)

/// @brief Debug alias for Logging_hexdump (similar to the `debug` alias)
#define hexdump(addr, n) Logging_hexdump(DEBUG, __FILE__, addr, n)

#endif
