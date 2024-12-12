#include <stdbool.h>
#include <stdarg.h>
#include "string.h"

#include "stdio.h"

#define TAB_SIZE 4

// ================ Helper for printf implementation ================

// Enum for the printf state-machine state
enum PRINTF_STATE {
	PRINTF_STATE_NORMAL,
	PRINTF_STATE_LENGTH,
	PRINTF_STATE_LENGTH_SHORT,
	PRINTF_STATE_LENGTH_LONG,
	PRINTF_STATE_SPEC
};

// Enum for the printf state-machine length
enum PRINTF_LENGTH {
	PRINTF_LENGTH_DEFAULT,
	PRINTF_LENGTH_SHORT_SHORT,
	PRINTF_LENGTH_SHORT,
	PRINTF_LENGTH_LONG,
	PRINTF_LENGTH_LONG_LONG
};

static int printf_unsigned(FILE* restrict stream, unsigned long long number, int radix){
	int printed = 0;

	// All possible characters that we can encouter
	const char hexChars[] = "0123456789abcdef";

	char buffer[128];
	int pos = 0; // position in the buffer

	// Convert the number to ASCII
	do {
		unsigned long long rem = number % radix;
		number /= radix;

		buffer[pos++] = hexChars[rem];
	} while(number > 0 && pos<128);

	// Print the parsed number to the screen
	while (--pos >= 0) {
		int res = fputc(buffer[pos], stream);
		if (res == EOF) return printed; // cannot write anymore
		printed++;
	}

	return printed;
}

static int printf_signed(FILE* restrict stream, long long number, int radix){
	int offset = 0;

	if (number < 0){
		fputc('-', stream);
		number = -number;
		offset = 1;
	}

	return offset + printf_unsigned(stream, number, radix);
}

// ================ printf functions ================

int printf(const char* restrict format, ...){
	va_list args;

	va_start(args, format);
	int res = vprintf(format, args);
	va_end(args);

	return res;
}

int fprintf(FILE* restrict stream, const char* restrict format, ...){
	va_list args;

	va_start(args, format);
	int res = vfprintf(stream, format, args);
	va_end(args);

	return res;
}

// int dprintf(int fd, const char* restrict format, ...);

// int sprintf(char* restrict str, const char* restrict format, ...);

// int snprintf(char* restrict str, size_t size, const char* restrict format, ...);

int vprintf(const char* restrict format, va_list args){
	return vfprintf(stdout, format, args);
}

int vfprintf(FILE* restrict stream, const char* restrict format, va_list args){
	int state = PRINTF_STATE_NORMAL;
	int length = PRINTF_LENGTH_DEFAULT;
	bool number = false;
	bool sign = false;
	int radix = 10;
	int printed = 0;

	while(*format){

		switch(state){

			case PRINTF_STATE_NORMAL:
				switch(*format){
					case '%':
						state = PRINTF_STATE_LENGTH;
						break;
					default:
						fputc(*format, stream);
						printed++;
						break;
				}
				break;

			case PRINTF_STATE_LENGTH:
				switch(*format){
					case 'h':
						length = PRINTF_LENGTH_SHORT;
						state = PRINTF_STATE_LENGTH_SHORT;
						break;
					case 'l':
						length = PRINTF_LENGTH_LONG;
						state = PRINTF_STATE_LENGTH_LONG;
						break;
					default:
						goto PRINTF_STATE_SPEC_;
				}
				break;

			case PRINTF_STATE_LENGTH_SHORT:
				if (*format == 'h'){
					length = PRINTF_LENGTH_SHORT_SHORT;
					state = PRINTF_STATE_SPEC;
				}
				else {
					goto PRINTF_STATE_SPEC_;
				}
				break;

			case PRINTF_STATE_LENGTH_LONG:
				if (*format == 'l'){
					length = PRINTF_LENGTH_LONG_LONG;
					state = PRINTF_STATE_SPEC;
				}
				else {
					goto PRINTF_STATE_SPEC_;
				}
				break;

			case PRINTF_STATE_SPEC:
				PRINTF_STATE_SPEC_:
				switch(*format){
					case 'c':
						// Note: we call va_args with int size, and not char, because in 32bits pmode
						// all arguments pushed onto the stack are rounded to 32 bits
						fputc((char) va_arg(args, int), stream);
						printed++;
						break;
					case 's':
						const char* s = va_arg(args, const char*);
						fputs(s, stream);
						printed += strlen(s);
						break;
					case '%':
						fputc('%', stream);
						printed++;
						break;
					case 'd':
					case 'i':
						number = true;
						sign = true;
						radix = 10;
						break;
					case 'u':
						number = true;
						sign = false;
						radix = 10;
						break;
					case 'X':
					case 'x':
					case 'p':
						fputc('0', stream);
						fputc('x', stream);
						printed += 2;
						number = true;
						sign = false;
						radix = 16;
						break;
					case 'o':
						number = true;
						sign = false;
						radix = 8;
						break;
					default:
						break; // ignore invalid specifiers
				}

			if (number){
				switch (length){
				case PRINTF_LENGTH_SHORT_SHORT:
				case PRINTF_LENGTH_SHORT:
				case PRINTF_LENGTH_DEFAULT:
					if (sign)	printed += printf_signed(stream, va_arg(args, int), radix);
					else		printed += printf_unsigned(stream, va_arg(args, unsigned int), radix);
					break;
				case PRINTF_LENGTH_LONG:
					if (sign)	printed += printf_signed(stream, va_arg(args, long), radix);
					else		printed += printf_unsigned(stream, va_arg(args, unsigned long), radix);
					break;
				case PRINTF_LENGTH_LONG_LONG:
					if (sign)	printed += printf_signed(stream, va_arg(args, long long), radix);
					else		printed += printf_unsigned(stream, va_arg(args, unsigned long long), radix);
					break;
				default:
					break;
				}
			}

			// reset state
			state = PRINTF_STATE_NORMAL;
			length = PRINTF_LENGTH_DEFAULT;
			radix = 10;
			sign = false;
			number = false;
			break;
		}

		format++;
	}

	return printed;
}

// int vdprintf(int fd, const char* restrict format, va_list args);

// int vsprintf(char* restrict str, const char* restrict format, va_list args);

// int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list args);
