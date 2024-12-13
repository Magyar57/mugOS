#ifndef __LOGGING_H__
#define __LOGGING_H__

#define DEBUG 0
#define SUCCESS 1
#define INFO 2
#define WARNING 3
#define ERROR 4
#define PANIC 5

#define log Logging_log
#define debug(fmt, ...) Logging_log(DEBUG, __FILE__, fmt, ##__VA_ARGS__)

void Logging_log(int logLevel, const char* moduleName, const char* logFmtStr, ...);

#endif
