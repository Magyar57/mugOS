#ifndef __LOGGING_H__
#define __LOGGING_H__

#define DEBUG 0
#define SUCCESS 1
#define INFO 2
#define WARNING 3
#define ERROR 4

// Quick debug printing
#define debug(fmt, ...) Logging_logModule(DEBUG, __FILE__, fmt, ##__VA_ARGS__ )

void Logging_logModule(int logLevel, const char* moduleName, const char* logFmtStr, ...);
void Logging_log(int logLevel, const char* logFmtStr, ...);

#endif
