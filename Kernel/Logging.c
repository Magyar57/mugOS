#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include "stdio.h"
#include "Drivers/Serial.h"

#include "Logging.h"

static const char* loglevelPrefix[] = {
	"[#DEBUG] ",
	"[  OK  ] ",
	"[ INFO ] ",
	"[ WARN ] ",
	"[ERROR!] ",
	">>PANIC!<< "
};

static void logToSerial(int logLevel, const char* moduleName, const char* logFmtStr, va_list args){
	if (!Serial_isEnabled())
		return;

	// really big buffer ; we could use kmalloc if snprintf returns -1
	char buff[4096];

	int written1;
	if (moduleName == NULL)	written1 = snprintf(buff, sizeof(buff), "%s-> ", loglevelPrefix[logLevel]);
	else 					written1 = snprintf(buff, sizeof(buff), "%s%s -> ", loglevelPrefix[logLevel], moduleName);
	if (written1 < 0) return; // buffer is not large enough

	int written2 = vsnprintf(buff+written1, sizeof(buff)-written1, logFmtStr, args);
	if (written2 < 0) return; // buffer is not large enough

	int written3 = snprintf(buff+written1+written2, sizeof(buff)-written1-written2, "\r\n");
	if (written3 < 0) return; // buffer is not large enough

	Serial_sendStringDefault(buff);
}

void Logging_log(int logLevel, const char* moduleName, const char* logFmtStr, ...){
	if (logLevel<0 || logLevel>PANIC) return;
	FILE* stream = (logLevel >= ERROR) ? stdout : stderr;

	va_list args, argsSerial;
	va_start(args, logFmtStr);
	va_copy(argsSerial, args);

	fputs(loglevelPrefix[logLevel], stream);
	if (moduleName != NULL) fprintf(stream, "%s -> ", moduleName);
	else fprintf(stream, "-> ");
	vfprintf(stream, logFmtStr, args);
	fputc('\n', stream);
	va_end(args);

	logToSerial(logLevel, moduleName, logFmtStr, argsSerial);
	va_end(argsSerial);
}
