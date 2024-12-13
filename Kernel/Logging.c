#include <stdarg.h>
#include "stdio.h"

#include "Logging.h"

const char* loglevelPrefix[] = {
	"[#DEBUG] ",
	"[  OK  ] ",
	"[ INFO ] ",
	"[ WARN ] ",
	"[ERROR!] ",
};

void Logging_logModule(int logLevel, const char* moduleName, const char* logFmtStr, ...){
	if (logLevel<0 || logLevel>ERROR) return;
	FILE* stream = (logLevel != ERROR) ? stdout : stderr;

	fputs(loglevelPrefix[logLevel], stream);
	fprintf(stream, "%s: ", moduleName);

	va_list args;
	va_start(args, logFmtStr);
	vfprintf(stream, logFmtStr, args);
	va_end(args);
}

void Logging_log(int logLevel, const char* logFmtStr, ...){
	if (logLevel<0 || logLevel>ERROR) return;
	FILE* stream = (logLevel != ERROR) ? stdout : stderr;

	fputs(loglevelPrefix[logLevel], stream);

	va_list args;
	va_start(args, logFmtStr);
	vfprintf(stream, logFmtStr, args);
	va_end(args);
}
