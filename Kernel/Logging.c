#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include "stdio.h"

#include "Logging.h"

static const char* loglevelPrefix[] = {
	"[#DEBUG] ",
	"[  OK  ] ",
	"[ INFO ] ",
	"[ WARN ] ",
	"[ERROR!] ",
	">>PANIC!<< "
};

void Logging_log(int logLevel, const char* moduleName, const char* logFmtStr, ...){
	if (logLevel<0 || logLevel>PANIC) return;
	FILE* stream = (logLevel != ERROR) ? stdout : stderr;

	fputs(loglevelPrefix[logLevel], stream);
	if (moduleName != NULL) fprintf(stream, "%s: ", moduleName);

	va_list args;
	va_start(args, logFmtStr);
	vfprintf(stream, logFmtStr, args);
	va_end(args);
	fputc('\n', stream);
}
