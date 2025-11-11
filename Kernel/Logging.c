#include <stddef.h>
#include <stdarg.h>
#include "stdio.h"
#include "Drivers/Output/Serial.h"

#include "Logging.h"
#define MODULE "Logging"

static const char* LOG_LEVEL_PREFIX[] = {
	"[#DEBUG] ",
	"[  OK  ] ",
	"[ INFO ] ",
	"[ WARN ] ",
	"[ERROR!] ",
	">>PANIC!<< "
};

/// @brief Format str of size n to the string to log (specified by logLevel, moduleName, logFmtStr and args)
static inline bool formatLogString(char* str, size_t n, int logLevel, const char* moduleName, const char* logFmtStr, va_list args){
	int written1, written2, written3;

	// Insert log level and module into 'str'

	if (moduleName == NULL)
		written1 = snprintf(str, n, "%s-> ", LOG_LEVEL_PREFIX[logLevel]);
	else
		written1 = snprintf(str, n, "%s%s -> ", LOG_LEVEL_PREFIX[logLevel], moduleName);

	if (written1 < 0)
		return false; // buffer too small

	n -= written1;

	// Insert formatted string into 'str'

	written2 = vsnprintf(str+written1, n, logFmtStr, args);
	if (written2 < 0)
		return false;

	n -= written2;

	// Add end of line

	written3 = snprintf(str+written1+written2, n, "\r\n");
	if (written3 < 0)
		return false;

	return true;
}

void log(int logLevel, const char* moduleName, const char* logFmtStr, ...){
	if (logLevel<0 || logLevel>PANIC) return;
	if (logFmtStr == NULL){
		log(logLevel, moduleName, "(null)");
		return;
	}

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

void hexdump(int logLevel, const char* moduleName, void* addr , int n){
	if (logLevel<0 || logLevel>PANIC) return;

	if (addr == NULL){
		log(logLevel, moduleName, "Hexdump: (null)");
		return;
	}

	if (n <= 0){
		log(logLevel, moduleName, "Hexdump: Cannot dump size n=%d", n);
		return;
	}

	char buff[512];
	uint8_t* data = addr;
	int i;
	int printed;

	// Dump 16 bytes per line
	for (i=0 ; i < n-15 ; i+=16){
		printed = snprintf(buff, sizeof(buff),
			"Hexdump: %#.8llx - "
			"%.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx  "
			"%.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx",
			i,
			data[i], data[i+1], data[i+2], data[i+3], data[i+4], data[i+5], data[i+6], data[i+7],
			data[i+8], data[i+9], data[i+10], data[i+11], data[i+12], data[i+13], data[i+14], data[i+15]
		);
		if (printed < 0) {
			log(ERROR, MODULE, "snprintf failure, couldn't dump memory");
			return;
		}
		log(logLevel, moduleName, buff);
	}

	// Final, partial line

	if (n % 16 == 0)
		return;

	printed = snprintf(buff, sizeof(buff), "Hexdump: %#.8llx - ", i);
	if (printed < 0) {
		log(ERROR, MODULE, "snprintf failure, couldn't dump memory");
		return;
	}

	int offset = printed;

	for ( ; i<n ; i++){
		printed = snprintf(buff+offset, sizeof(buff)-i-offset, "%.2hhx ", data[i]);
		if (printed < 0){
			log(ERROR, MODULE, "snprintf failure, couldn't dump memory");
			return;
		}
		offset += printed;

		// Add space separator in the middle
		if (i % 8 == 7){
			printed = snprintf(buff+offset, sizeof(buff)-i-offset, " ");
			if (printed < 0){
				log(ERROR, MODULE, "snprintf failure, couldn't dump memory");
				return;
			}
			offset += printed;
		}
	}
	log(logLevel, moduleName, buff);
}
