#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stddef.h>

#define DEBUG 0
#define SUCCESS 1
#define INFO 2
#define WARNING 3
#define ERROR 4
#define PANIC 5

/// @brief Log a formatted string
/// @param logLevel SUCCESS, INFO, WARNING, ERROR... See `Logging.h` for values
/// @param moduleName Nullable string of the module from which log is emitted
/// @param logFmtStr Formatted string to log
/// @param ... formatting arguments (printf syntax)
void log(int logLevel, const char* moduleName, const char* logFmtStr, ...);

/// @brief Dump the memory (in hexadecimal) in the console
void hexdump(int logLevel, const char* moduleName, void* addr , size_t n);

/// @brief Alias for `log`, with default arguments.
/// Handy for writing quick logs while debugging
#define debug(fmt, ...) log(DEBUG, __FILE__, fmt, ##__VA_ARGS__)

#endif
