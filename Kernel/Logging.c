#include <stddef.h>
#include <stdarg.h>
#include "stdio.h"
#include "Drivers/Serial.h"

#include "Logging.h"

#define MODULE "Logging"

static const char* loglevelPrefix[] = {
	"[#DEBUG] ",
	"[  OK  ] ",
	"[ INFO ] ",
	"[ WARN ] ",
	"[ERROR!] ",
	">>PANIC!<< "
};

/// @brief Format str of size n to the string to log (specified by logLevel, moduleName, logFmtStr and args)
static inline bool formatLogString(char* str, size_t n, int logLevel, const char* moduleName, const char* logFmtStr, va_list args){
	int written1;
	if (moduleName == NULL)	written1 = snprintf(str, n, "%s-> ", loglevelPrefix[logLevel]);
	else 					written1 = snprintf(str, n, "%s%s -> ", loglevelPrefix[logLevel], moduleName);
	if (written1 < 0) return false; // buffer too small
	n -= written1;

	int written2 = vsnprintf(str+written1, n, logFmtStr, args);
	if (written2 < 0) return false;
	n -= written2;

	int written3 = snprintf(str+written1+written2, n, "\r\n");
	if (written3 < 0) return false;

	return true;
}

void Logging_log(int logLevel, const char* moduleName, const char* logFmtStr, ...){
	if (logLevel<0 || logLevel>PANIC) return;

	va_list args;
	va_start(args, logFmtStr);

	char buff[4096]; // 4 KB log string length limit
	bool success = formatLogString(buff, sizeof(buff), logLevel, moduleName, logFmtStr, args);
	if (!success){
		log(ERROR, MODULE, "Message is too long, cannot log !!");
		return;
	}

	va_end(args);

	// During booting: log to stdout (screen)
	fputs(buff, (logLevel >= ERROR) ? stdout : stderr);

	// Log to Serial
	// sendStringDefault returns false if the Serial is uninitialized AND on error
	// For simplicity, we ignore errors
	Serial_sendStringDefault(buff);
}
