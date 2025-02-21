#include <stdbool.h>
#include <stdarg.h>
#include "unistd.h"
#include "string.h"

#include "stdio.h"

#define TAB_SIZE 4

// ================ Helper for printf implementation ================

// Doc: https://cplusplus.com/reference/cstdio/printf/
// %[$][flags][width][.precision][length modifier]specifier
// supported options:
// - flags: #
// - specifier: c s % d i o x X u p
// - width: h hh l ll

// Tests printf
// printf("%% %c %s ", 'a', "my_string");
// printf("%d %i %x %p %o \n", 1234, -5678, 0x7fff, 0xbeef, 012345);
// printf("%hd %hi %hhd %hhu \n", (short)57, (short)-42, (unsigned char) 20, (char)-10);
// printf("%ld %li %lld %llu \n", 100000057l, -100000042l, -1099511627776ll, 0xffffffffffffffffull);
// printf("%p %x %#x %X %#llX \n", 0x123456789abcdef0, 0x80000000ffffffff, 0x80000000ffffffff, 0x80000000ffffffff, 0x80000000ffffffff);

// Enum for the printf state-machine state
enum PRINTF_STATE {
	PRINTF_STATE_NORMAL,
	PRINTF_STATE_FLAGS,
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

static int dputc(int c, int fd){
	if (fd<0) return EOF;

	char to_write = (unsigned char) c;
	ssize_t res = write(fd, &to_write, 1);

	return (res == 1) ? to_write : EOF;
}

static int dputs(const char* restrict s, int fd){
	int res;
	if (s == NULL) return EOF;

	while(*s){
		res = dputc(*s, fd);
		if (res == EOF) return EOF;
		s++;
	}

	return 1;
}

static int printf_unsigned(int fd, unsigned long long number, int radix, bool uppercase){
	int printed = 0;

	// All possible characters that we can encouter
	const char* hexChars = (uppercase) ? "0123456789ABCDEF" : "0123456789abcdef";

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
		int res = dputc(buffer[pos], fd);
		if (res == EOF) return printed; // cannot write anymore
		printed++;
	}

	return printed;
}

static int printf_signed(int fd, long long number, int radix, bool uppercase){
	int offset = 0;

	if (number < 0){
		dputc('-', fd);
		number = -number;
		offset = 1;
	}

	return offset + printf_unsigned(fd, number, radix, uppercase);
}

static size_t sizeOfNumberToBeWritten(long long number, int radix, bool sign){
	size_t res = 0;
	unsigned long long numberUnsigned = (sign && number<0) ?
		(unsigned long long) -number : (unsigned long long) number;

	if (sign && number<0) res++; // '-'

	do {
		// unsigned long long rem = number % radix;
		numberUnsigned /= radix;
		res++;
	} while(numberUnsigned > 0);

	return res;
}

static int sprintf_unsigned(char* str, unsigned long long number, int radix, bool uppercase){
	int printed = 0;
	int i = 0; // index in str

	// All possible characters that we can encouter
	const char* hexChars = (uppercase) ? "0123456789ABCDEF" : "0123456789abcdef";

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
		str[i] = buffer[pos];
		printed++;
		i++;
	}

	return printed;
}

static int sprintf_signed(char* str, long long number, int radix, bool uppercase){
	int offset = 0;

	if (number < 0){
		str[0] = '-';
		number = -number;
		offset = 1;
	}

	return offset + sprintf_unsigned(str+offset, number, radix, uppercase);
}

// ================ internal printf functions (actual logic) ================

static int vdprintf_internal(int fd, const char* restrict format, va_list args){
	int state = PRINTF_STATE_NORMAL;
	int length = PRINTF_LENGTH_DEFAULT;
	bool number = false;
	bool sign = false;
	int radix = 10;
	int printed = 0;
	bool uppercase = false; // false: 0x7fff true: 0X7FFF
	bool putHexPrefix = false; // 0x prefix
	const char* s; // '%s' pointer

	if (format == NULL) return -1;
	if (fd < 0) return -2;

	while(*format){
		switch(state){

		case PRINTF_STATE_NORMAL:
			switch(*format){
			case '%':
				state = PRINTF_STATE_FLAGS;
				break;
			default:
				dputc(*format, fd);
				printed++;
				break;
			}
			break;

		case PRINTF_STATE_FLAGS:
			switch (*format){
			case '#':
				putHexPrefix = true;
				break;
			default:
				state = PRINTF_STATE_LENGTH;
				goto PRINTF_STATE_LENGTH_;
			}
			break;

		case PRINTF_STATE_LENGTH:
			PRINTF_STATE_LENGTH_:
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
			if (*format != 'h')
				goto PRINTF_STATE_SPEC_;
			length = PRINTF_LENGTH_SHORT_SHORT;
			state = PRINTF_STATE_SPEC;
			break;

		case PRINTF_STATE_LENGTH_LONG:
			if (*format != 'l')
				goto PRINTF_STATE_SPEC_;
			length = PRINTF_LENGTH_LONG_LONG;
			state = PRINTF_STATE_SPEC;
			break;

		case PRINTF_STATE_SPEC:
			PRINTF_STATE_SPEC_:
			switch(*format){
				case '%':
					dputc('%', fd);
					printed++;
					break;
				case 'X':
					if (putHexPrefix){ dputs("0X", fd); printed += 2; }
					number = true;
					sign = false;
					radix = 16;
					uppercase = true;
					break;
				case 'c':
					// Note: we call va_args with int size, and not char, because in 32bits pmode
					// all arguments pushed onto the stack are rounded to 32 bits
					dputc((char) va_arg(args, int), fd);
					printed++;
					break;
				case 's':
					s = va_arg(args, const char*);
					if (s == NULL) {
						dputs("(null)", fd);
						printed += 6;
					}
					else {
						dputs(s, fd);
						printed += strlen(s);
					}
					break;
				case 'd':
				case 'i':
					number = true;
					sign = true;
					radix = 10;
					break;
				case 'o':
					number = true;
					sign = false;
					radix = 8;
					break;
				case 'p':
					length = PRINTF_LENGTH_LONG_LONG; // pointers are 64 bits
					dputs("0x", fd);
					printed += 2;
					number = true;
					sign = false;
					radix = 16;
					break;
				case 'u':
					number = true;
					sign = false;
					radix = 10;
					break;
				case 'x':
					if (putHexPrefix){ dputs("0x", fd); printed += 2; }
					number = true;
					sign = false;
					radix = 16;
					uppercase = false;
					break;
				default:
					break; // ignore invalid specifiers
			}

			if (!number)
				goto reset_state;

			switch (length){
			case PRINTF_LENGTH_SHORT_SHORT:
			case PRINTF_LENGTH_SHORT:
			case PRINTF_LENGTH_DEFAULT:
				if (sign)	printed += printf_signed(fd, va_arg(args, int), radix, uppercase);
				else		printed += printf_unsigned(fd, va_arg(args, unsigned int), radix, uppercase);
				break;
			case PRINTF_LENGTH_LONG:
				if (sign)	printed += printf_signed(fd, va_arg(args, long), radix, uppercase);
				else		printed += printf_unsigned(fd, va_arg(args, unsigned long), radix, uppercase);
				break;
			case PRINTF_LENGTH_LONG_LONG:
				if (sign)	printed += printf_signed(fd, va_arg(args, long long), radix, uppercase);
				else		{
					unsigned long long val = va_arg(args, unsigned long long);
					printed += printf_unsigned(fd, val, radix, uppercase);
				}
				break;
			default:
				break;
			}

			reset_state:
			state = PRINTF_STATE_NORMAL;
			length = PRINTF_LENGTH_DEFAULT;
			radix = 10;
			sign = false;
			number = false;
			uppercase = false;
			putHexPrefix = false;
			break; // case PRINTF_STATE_SPEC
		}

		format++;
	}

	return printed;
}

// Returns whether writing a new character WILL write to the '\0' character's place
static inline bool snprintf_isBoundaryExceeded(size_t size, int i, bool checkSize){
	if (!checkSize) return false;

	// We are sure that this function will be called with size>0
	return (i > size-1);
}

#define vsnprintf_internal_checkBoundaries(size, i, checkSize) \
	if (snprintf_isBoundaryExceeded(size, i, checkSize)) {printed=-1; goto end;}

static int vsnprintf_internal(char* restrict fmtStr, size_t size, bool checkSize, const char* restrict format, va_list args){
	int state = PRINTF_STATE_NORMAL;
	int length = PRINTF_LENGTH_DEFAULT;
	bool number = false;
	bool sign = false;
	int radix = 10;
	int printed = 0;
	bool uppercase = false; // false: 0x7fff true: 0X7FFF
	bool putHexPrefix = false; // 0x prefix
	int i = 0; // index in fmtStr
	const char* s; // '%s' pointer

	if (fmtStr == NULL) return -2;
	if (format == NULL) return -3;

	if (checkSize && size==0) return -1; // cannot write final '\0'
	if (checkSize && size==1) {
		if (*format != '\0') printed = -1; // return error if there was stuff to print
		goto end; // write '\0' and return
	}

	while(*format){
		switch(state){

		case PRINTF_STATE_NORMAL:
			switch(*format){
			case '%':
				state = PRINTF_STATE_FLAGS;
				break;
			default:
				vsnprintf_internal_checkBoundaries(size, i+1, checkSize);
				fmtStr[i] = *format;
				printed++;
				i++;
				break;
			}
			break;

		case PRINTF_STATE_FLAGS:
			switch (*format){
			case '#':
				putHexPrefix = true;
				break;
			default:
				state = PRINTF_STATE_LENGTH;
				goto PRINTF_STATE_LENGTH_;
			}
			break;

		case PRINTF_STATE_LENGTH:
			PRINTF_STATE_LENGTH_:
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
			if (*format != 'h')
				goto PRINTF_STATE_SPEC_;
			length = PRINTF_LENGTH_SHORT_SHORT;
			state = PRINTF_STATE_SPEC;
			break;

		case PRINTF_STATE_LENGTH_LONG:
			if (*format != 'l')
				goto PRINTF_STATE_SPEC_;
			length = PRINTF_LENGTH_LONG_LONG;
			state = PRINTF_STATE_SPEC;
			break;

		case PRINTF_STATE_SPEC:
			PRINTF_STATE_SPEC_:
			switch(*format){
			case '%':
				vsnprintf_internal_checkBoundaries(size, i+1, checkSize);
				fmtStr[i] = '%';
				printed++;
				i++;
				break;
			case 'X':
				if (putHexPrefix){
					vsnprintf_internal_checkBoundaries(size, i+2, checkSize);
					fmtStr[i++] = '0';
					fmtStr[i++] = 'X';
					printed += 2;
				}
				number = true;
				sign = false;
				radix = 16;
				uppercase = true;
				break;
			case 'c':
				// Note: we call va_args with int size, and not char, because in 32bits pmode
				// all arguments pushed onto the stack are rounded to 32 bits
				vsnprintf_internal_checkBoundaries(size, i+1, checkSize);
				fmtStr[i] = (char) va_arg(args, int);
				printed++;
				i++;
				break;
			case 's':
				s = va_arg(args, const char*);
				if (s == NULL) {
					vsnprintf_internal_checkBoundaries(size, i+6, checkSize);
					fmtStr[i++] = '(';
					fmtStr[i++] = 'n';
					fmtStr[i++] = 'u';
					fmtStr[i++] = 'l';
					fmtStr[i++] = 'l';
					fmtStr[i++] = ')';
					printed += 6;
				}
				else {
					while(*s){
						vsnprintf_internal_checkBoundaries(size, i+1, checkSize);
						fmtStr[i] = *s;
						printed++;
						i++;
						s++;
					}
				}
				break;
			case 'd':
			case 'i':
				number = true;
				sign = true;
				radix = 10;
				break;
			case 'o':
				number = true;
				sign = false;
				radix = 8;
				break;
			case 'p':
				length = PRINTF_LENGTH_LONG_LONG; // pointers are 64 bits
				vsnprintf_internal_checkBoundaries(size, i+2, checkSize);
				fmtStr[i++] = '0';
				fmtStr[i++] = 'x';
				printed += 2;
				number = true;
				sign = false;
				radix = 16;
				break;
			case 'u':
				number = true;
				sign = false;
				radix = 10;
				break;
			case 'x':
				if (putHexPrefix){
					vsnprintf_internal_checkBoundaries(size, i+2, checkSize);
					fmtStr[i++] = '0';
					fmtStr[i++] = 'x';
					printed += 2;
				}
				number = true;
				sign = false;
				radix = 16;
				uppercase = false;
				break;
			default:
				break; // ignore invalid specifiers
			}

			if (!number)
				goto reset_state;

			switch (length){
			case PRINTF_LENGTH_DEFAULT:
			case PRINTF_LENGTH_SHORT_SHORT:
			case PRINTF_LENGTH_SHORT:
				if (sign) {
					int numberToPrint = va_arg(args, int);
					vsnprintf_internal_checkBoundaries(size, i+sizeOfNumberToBeWritten(numberToPrint, radix, sign), checkSize);
					int written = sprintf_signed(fmtStr+i, numberToPrint, radix, uppercase);
					printed += written;
					i+= written;
				}
				else {
					unsigned int numberToPrint = va_arg(args, unsigned int);
					vsnprintf_internal_checkBoundaries(size, i+sizeOfNumberToBeWritten(numberToPrint, radix, sign), checkSize);
					int written = sprintf_unsigned(fmtStr+i, numberToPrint, radix, uppercase);
					printed += written;
					i+= written;
				}
				break;
			case PRINTF_LENGTH_LONG:
				if (sign) {
					long numberToPrint = va_arg(args, long);
					vsnprintf_internal_checkBoundaries(size, i+sizeOfNumberToBeWritten(numberToPrint, radix, sign), checkSize);
					int written = sprintf_signed(fmtStr+i, numberToPrint, radix, uppercase);
					printed += written;
					i+= written;
				}
				else {
					unsigned long numberToPrint = va_arg(args, unsigned long);
					vsnprintf_internal_checkBoundaries(size, i+sizeOfNumberToBeWritten(numberToPrint, radix, sign), checkSize);
					int written = sprintf_unsigned(fmtStr+i, numberToPrint, radix, uppercase);
					printed += written;
					i+= written;
				}
				break;
			case PRINTF_LENGTH_LONG_LONG:
				if (sign) {
					long long numberToPrint = va_arg(args, long long);
					vsnprintf_internal_checkBoundaries(size, i+sizeOfNumberToBeWritten(numberToPrint, radix, sign), checkSize);
					int written = sprintf_signed(fmtStr+i, numberToPrint, radix, uppercase);
					printed += written;
					i+= written;
				}
				else {
					unsigned long long numberToPrint = va_arg(args, unsigned long long);
					vsnprintf_internal_checkBoundaries(size, i+sizeOfNumberToBeWritten(numberToPrint, radix, sign), checkSize);
					int written = sprintf_unsigned(fmtStr+i, numberToPrint, radix, uppercase);
					printed += written;
					i+= written;
				}
				break;
			default:
				break;
			}

			reset_state:
			state = PRINTF_STATE_NORMAL;
			length = PRINTF_LENGTH_DEFAULT;
			radix = 10;
			sign = false;
			number = false;
			uppercase = false;
			putHexPrefix = false;
			break;
		}

		format++;
	}

	end:
	fmtStr[i] = '\0';
	return printed;
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

int dprintf(int fd, const char* restrict format, ...){
	va_list args;

	va_start(args, format);
	int res = vdprintf_internal(fd, format, args);
	va_end(args);

	return res;
}

int sprintf(char* restrict str, const char* restrict format, ...){
	va_list args;

	va_start(args, format);
	int res = vsnprintf_internal(str, 0, false, format, args);
	va_end(args);

	return res;
}

int snprintf(char* restrict str, size_t size, const char* restrict format, ...){
	va_list args;

	va_start(args, format);
	int res = vsnprintf_internal(str, size, true, format, args);
	va_end(args);

	return res;
}

int vprintf(const char* restrict format, va_list args){
	return vfprintf(stdout, format, args);
}

int vfprintf(FILE* restrict stream, const char* restrict format, va_list args){
	if ((stream == NULL) || (stream->fd < 0))
		return -1;

	return vdprintf_internal(stream->fd, format, args);
}

int vdprintf(int fd, const char* restrict format, va_list args){
	return vdprintf_internal(fd, format, args);
}

int vsprintf(char* restrict str, const char* restrict format, va_list args){
	return vsnprintf_internal(str, 0, false, format, args);
}

int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list args){
	return vsnprintf_internal(str, size, true, format, args);
}
