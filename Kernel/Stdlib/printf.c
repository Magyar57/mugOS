#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "unistd.h"
#include "string.h"

#include "stdio.h"

// Note: when - and 0, 0 should be ignored

// Doc: https://cplusplus.com/reference/cstdio/printf/
// %[$][flags][width][.precision][length modifier]specifier
// Supported options:
// - flags: # + (space)
// - width: (number)
// - width: h hh l ll
// - specifier: c s % d i u o x X p
// Unsupported options: '-' flag, and all floating point
// Future implementation notes:
// - '-' overrides '0' (if - is there, always pad with spaces, and on the right)
// - If precision is present, always pad the missing precision numbers with 0

// Tests printf
// printf("%% %c %s ", 'a', "my_string");
// printf("%d %i %x %p %o \n", 1234, -5678, 0x7fff, 0xbeef, 012345);
// printf("%hd %hi %hhd %hhu \n", (short)57, (short)-42, (unsigned char) 20, (char)-10);
// printf("%ld %li %lld %llu \n", 100000057l, -100000042l, -1099511627776ll, 0xffffffffffffffffull);
// printf("%p %x %#x %X %#llX %#018llx '%# 18llx'\n", 0x123456789abcdef0, 0x1ffffffff, 0x1ffffffff, 0x1ffffffff, 0x80000000ffffffff, 0x7fffffffllu, 0x7fffffffllu);
// printf("'%4d' '% 4d' '%+4d' %04d '% 04d' %+04d \n", -10, 10, 10, 10, -10, 10, 10, 10);

// Tests snprintf
// void test_snprintf(int max_buff, const char* fmt, uint64_t value){
//     char str[1024];
//     memset(str, 0, 1024);
//     if (max_buff >= 1024) return; 
//     int res = res = snprintf(str, max_buff, fmt, value);
//     printf("(%2d/%2d) sprintf('%s', %#018hhx) => '%s' \n", res, max_buff, fmt, value, str);
// }
// test_snprintf(64, "bonjaj", 0); // 6 'bonjaj'
// test_snprintf(4, "bonjaj", 0); // -1 'bon'
// test_snprintf(1, "bonjaj", 0); // -1 ''
// test_snprintf(0, "bonjaj", 0); // -1 ''
// test_snprintf(7, "%s", (uint64_t) NULL); // 6 '(null)'
// test_snprintf(6, "%s", (uint64_t) NULL); // -1 ''
// test_snprintf(2, "%c", 'a'); // 1 'a'
// test_snprintf(1, "%c", 'a'); // -1 ''
// test_snprintf(21, "%llu", 18446744073709551615llu); // 20 '18446744073709551615'
// test_snprintf(20, "%llu", 18446744073709551615llu); // -1 ''
// test_snprintf(21, "%lld", -9223372036854775807ll); // 20 '-9223372036854775807'
// test_snprintf(20, "%lld", -9223372036854775807ll); // -1 ''
// test_snprintf(11, "%p", 0xffffffff); // 10 '0xffffffff'
// test_snprintf(10, "%p", 0xffffffff); // -1 ''
// test_snprintf(11, "%#010x", 0xffffffff); // 10 '0xffffffff'
// test_snprintf(10, "%#010x", 0xffffffff); // -1 ''
// test_snprintf(13, "%#012x", 0xffffffff); // 12 '0x00ffffffff'
// test_snprintf(12, "%#012x", 0xffffffff); // -1 ''
// test_snprintf(13, "%#012x", 0xffffffff); // 12 '  0xffffffff'
// test_snprintf(12, "%#012x", 0xffffffff); // -1 ''
// test_snprintf(4, "%+d", 0x00000040); // 3 '+64'
// test_snprintf(5, "%+04d", 0x00000040); // 4 '+064'
// test_snprintf(5, "% +4d", 0x00000040); // 4 ' +64'

// Enum for the printf state-machine state
enum PRINTF_STATE {
	PRINTF_STATE_NORMAL,
	PRINTF_STATE_FLAGS,
	PRINTF_STATE_WIDTH,
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

// ================ Helpers for *printf implementation ================

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

// Put the padding (width * padding_char) if the current padding_char_position (' ' or '0') is the right one
static int printf_uint_putPadding(int fd, int width, char padding_char_position, char padding_char){
	if (padding_char != padding_char_position)
		return 0;

	int printed = 0;
	for (int i=width ; i>0 ; i--){
		int res = dputc(padding_char, fd);
		if (res == EOF) return printed; // cannot write anymore
		printed++;
	}

	return printed;
}

static int printf_uint_putPrefix(int fd, int radix, bool uppercase){
	int res;

	switch (radix){
		case 8:
			res = dputc('0', fd);
			return (res == EOF) ? EOF : 1;
		case 16:
			res = dputs((uppercase) ? "0X" : "0x", fd);
			return (res == EOF) ? EOF : 2;
		default:
			return 0;
	}
}

// sign is the character to put as a sign in front of the number
static int printf_uint(int fd, unsigned long long number, int radix, char sign, bool uppercase, bool putPrefix, bool padWithZeros, int64_t width){
	int printed = 0;
	int res;

	// All possible characters that we can encouter
	const char* hexChars = (uppercase) ? "0123456789ABCDEF" : "0123456789abcdef";
	// Padding character
	const char padding_char = padWithZeros ? '0' : ' ';

	char buffer[128];
	int pos = 0; // position in the buffer

	// Convert the number to ASCII
	do {
		unsigned long long rem = number % radix;
		number /= radix;

		buffer[pos++] = hexChars[rem];
	} while(number > 0 && pos<128);

	// Update the padding width: remove character already printed & prefix size
	width -= pos;
	if (putPrefix) width -= (radix > 9) ? 2 : 1;
	if (sign) width--; // leave one space for the sign

	// Print spaces padding, if padding_char is a space
	res = printf_uint_putPadding(fd, width, ' ', padding_char);
	if (res == EOF) return printed;
	printed += res;

	// Put the sign (if needed)
	if (sign){
		res = dputc(sign, fd);
		if (res == EOF) return printed;
		printed++;
	}

	// Put the prefix
	if (putPrefix){
		res = printf_uint_putPrefix(fd, radix, uppercase);
		if (res == EOF) return printed;
		printed += res;
	}

	// Print zeros padding, if padding_char is a zero
	res = printf_uint_putPadding(fd, width, '0', padding_char);
	if (res == EOF) return printed;
	printed += res;

	// Print the parsed number to the screen
	while (--pos >= 0) {
		res = dputc(buffer[pos], fd);
		if (res == EOF) return printed;
		printed++;
	}

	return printed;
}

static int printf_int(int fd, long long number, int radix, char sign, bool uppercase, bool putPrefix, bool padWithZeros, int64_t width){
	if (number < 0){
		sign = '-';
		number = -number;
	}
	// else leave sign untouched (can be either '+' or ' ')

	return printf_uint(fd, number, radix, sign, uppercase, putPrefix, padWithZeros, width);
}

// ================ Helpers for s*printf implementation ================

// Fast compute of the number of digits of a number in base 10
static inline int getNumberOfDigits_base10(unsigned long long number){
	// https://stackoverflow.com/questions/25892665/performance-of-log10-function-returning-an-int
	// extended to 64 bits
	if (number == 0) return 1;

	static const uint8_t guess[65] = {
		0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
		3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
		6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
		9, 9, 9, 9, 10, 10, 10, 11, 11, 11,
		12, 12, 12, 12, 13, 13, 13, 14, 14, 14,
		15, 15, 15, 15, 16, 16, 16, 17, 17, 17,
		18, 18, 18, 18, 19
	};
	static const unsigned long long tenToThe[] = {
		1llu, 10llu, 100llu, 1000llu, 10000llu, 100000llu,
		1000000llu, 10000000llu, 100000000llu, 1000000000llu, 10000000000llu,
		100000000000llu, 1000000000000llu, 10000000000000llu, 100000000000000llu,
		1000000000000000llu, 10000000000000000llu, 100000000000000000llu,
		1000000000000000000llu, 10000000000000000000llu,
    };

	int log2_of_x = number ? 64 - __builtin_clzll(number) : 0;
    unsigned int digits = guess[log2_of_x];
    return digits + (number >= tenToThe[digits]);
}

// Fast compute of the nubmer of digits of a number in base 16
static inline int getNumberOfDigits_base16(unsigned long long number){
	if (number == 0) return 1;
	int res = 0;

	while (number > 0){
		number >>= 4;
		res++;
	}
	return res;
}

// Fast compute of the nubmer of digits of a number in base 8
static inline int getNumberOfDigits_base8(unsigned long long number){
	if (number == 0) return 1;
	int res = 0;

	while (number > 0){
		number >>= 3;
		res++;
	}
	return res;
}

static inline int getNumberOfDigits(unsigned long long number, int base){
	if (base == 10){
		return getNumberOfDigits_base10(number);
	}
	if (base == 16){
		return getNumberOfDigits_base16(number);
	}
	if (base == 8){
		return getNumberOfDigits_base8(number);
	}

	__builtin_unreachable();
	return getNumberOfDigits_base10(number);
}

static size_t getSizeOfNumberToPrint_unsigned(unsigned long long number, int base, char sign, bool putPrefix, int64_t paddingWidth){
	// [space padding][prefix][0 padding][sign]number

	size_t size_to_print = getNumberOfDigits(number, base);

	// Account for prefix size
	if (putPrefix)
		size_to_print += (base > 9) ? 2 : 1;

	// Any sign to be printed (+, - or space)
	if (sign)
		size_to_print++;

	// size_to_print = max(paddingWidth, size_to_print)
	if (paddingWidth > size_to_print)
		size_to_print = paddingWidth;

	return size_to_print;
}

static size_t getSizeOfNumberToPrint_signed(long long number, int base, char sign, bool putPrefix, int64_t paddingWidth){
	unsigned long long numberUnsigned;

	// Account for sign character
	if (number<0){
		numberUnsigned = (unsigned long long) -number;
		sign = '-'; // force getSizeOfNumberToPrint_unsigned to account for a sign
	}
	else {
		numberUnsigned = (unsigned long long) number;
	}

	return getSizeOfNumberToPrint_unsigned(numberUnsigned, base, sign, putPrefix, paddingWidth);
}

// Put the padding (width * padding_char) if the current padding_char_position (' ' or '0') is the right one
static int sprintf_uint_putPadding(char* str, int width, char padding_char_position, char padding_char){
	if (padding_char != padding_char_position)
		return 0;

	int printed;
	for (printed=0 ; printed<width ; printed++){
		str[printed] = padding_char;
	}

	return printed;
}

static int sprintf_uint_putPrefix(char* str, int radix, bool uppercase){
	switch (radix){
		case 8:
			str[0] = '0';
			return 1;
		case 16:
			str[0] = '0';
			str[1] = (uppercase) ? 'X' : 'x';
			return 2;
		default:
			return 0;
	}
}

static int sprintf_uint(char* str, unsigned long long number, int radix, char sign, bool uppercase, bool putPrefix, bool padWithZeros, int64_t width){
	int printed = 0; // index in str

	// All possible characters that we can encouter
	const char* hexChars = (uppercase) ? "0123456789ABCDEF" : "0123456789abcdef";
	// Padding character
	const char padding_char = padWithZeros ? '0' : ' ';

	char buffer[128];
	int pos = 0; // position in the buffer

	// Convert the number to ASCII
	do {
		unsigned long long rem = number % radix;
		number /= radix;

		buffer[pos++] = hexChars[rem];
	} while(number > 0 && pos<128);

	// Update the padding width: remove character already printed & prefix size
	width -= pos;
	if (putPrefix) width -= (radix > 9) ? 2 : 1;
	if (sign) width--; // leave one space for the sign

	// Print spaces padding, if padding_char is a space
	printed += sprintf_uint_putPadding(str, width, ' ', padding_char);

	// Put the sign (if needed)
	if (sign){
		str[printed++] = sign;
	}

	// Put the prefix
	if (putPrefix){
		printed += sprintf_uint_putPrefix(str+printed, radix, uppercase);
	}

	// Print zeros padding, if padding_char is a zero
	printed += sprintf_uint_putPadding(str+printed, width, '0', padding_char);

	// Print the parsed number into the string
	while (--pos >= 0) {
		str[printed++] = buffer[pos];
	}

	return printed;
}

static int sprintf_int(char* str, long long number, int radix, char sign, bool uppercase, bool putPrefix, bool padWithZeros, int64_t width){
	if (number < 0){
		sign = '-';
		number = -number;
	}

	return sprintf_uint(str, number, radix, sign, uppercase, putPrefix, padWithZeros, width);
}

// Returns whether writing a new character WILL write to the '\0' character's place
#define snprintf_isBoundaryExceeded(size, i, checkSize)	\
	( checkSize && (i > size-1) )

#define vsnprintf_internal_checkBoundaries(size, i, checkSize) \
	if (snprintf_isBoundaryExceeded(size, i, checkSize)) {printed=-1; goto end;}

// ================ printf state-machines functions ================

static int vdprintf_internal(int fd, const char* restrict format, va_list args){
	const char* s; // '%s' pointer
	int printed = 0; // number of characters printed (return value)

	int state = PRINTF_STATE_NORMAL;
	bool put_prefix = false;			// Flag '#' => put prefix: "0x"/"0X" for hex, "0" for octal
	bool pad_with_zeros = false;		// Flag '0' => pad with '0' instead of ' ' when width is specified
	char sign = '\0';					// Flag '+' or ' ' => how to print the sign of positives number (\0 means nothing to print)
	int64_t width = 0;					// Width (number) => width specifier (padding)
	int length = PRINTF_LENGTH_DEFAULT;	// Length (h, hh, l, ll) => size of the number to be poped
	bool print_number = true;			// Specifier is a number to print (if not, skip)
	int radix = 10;						// Specifier (d, x, o, ...) => print as decimal, hexadecimal, octal
	bool number_signed = false;			// Specifier (d, u) => print signed, unsigned
	bool uppercase = false;				// Specifier (x, X) => 0x7fff, 0X7FFF

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
			case ' ':
				sign = ' ';
				break;
			case '#':
				put_prefix = true;
				break;
			case '+':
				sign = '+';
				break;
			case '0':
				pad_with_zeros = true;
				break;
			default:
				state = PRINTF_STATE_WIDTH;
				goto PRINTF_STATE_WIDTH_;
			}
			break;

		case PRINTF_STATE_WIDTH:
			PRINTF_STATE_WIDTH_:
			switch (*format){
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					width *= 10;
					width += (*format - '0'); // convert '0' to '9' to their corresponding number 0-9
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
					print_number = false;
					break;
				case 'X':
					number_signed = false;
					radix = 16;
					uppercase = true;
					sign = '\0';
					break;
				case 'c':
					dputc((char) va_arg(args, int), fd);
					printed++;
					print_number = false;
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
					print_number = false;
					break;
				case 'd':
				case 'i':
					number_signed = true;
					radix = 10;
					break;
				case 'o':
					number_signed = false;
					radix = 8;
					break;
				case 'p':
					length = PRINTF_LENGTH_LONG_LONG; // pointers are 64 bits
					put_prefix = true; // always put 0x prefix for %p
					number_signed = false;
					radix = 16;
					sign = '\0';
					break;
				case 'u':
					number_signed = false;
					radix = 10;
					break;
				case 'x':
					number_signed = false;
					radix = 16;
					sign = '\0';
					uppercase = false;
					break;
				default:
					print_number = false;
					break;
			}

			// Specifier invalid (skip it), or already handled
			if (!print_number)
				goto reset_state;

			switch (length){
			case PRINTF_LENGTH_SHORT_SHORT:
			case PRINTF_LENGTH_SHORT:
			case PRINTF_LENGTH_DEFAULT:
				if (number_signed)	printed += printf_int(fd, va_arg(args, int), radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				else		printed += printf_uint(fd, va_arg(args, unsigned int), radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				break;
			case PRINTF_LENGTH_LONG:
				if (number_signed)	printed += printf_int(fd, va_arg(args, long), radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				else		printed += printf_uint(fd, va_arg(args, unsigned long), radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				break;
			case PRINTF_LENGTH_LONG_LONG:
				if (number_signed)	printed += printf_int(fd, va_arg(args, long long), radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				else		{
					unsigned long long val = va_arg(args, unsigned long long);
					printed += printf_uint(fd, val, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				break;
			default:
				break;
			}

			reset_state:
			state = PRINTF_STATE_NORMAL;
			put_prefix = false;
			pad_with_zeros = false;
			sign = '\0';
			width = 0;
			length = PRINTF_LENGTH_DEFAULT;
			print_number = true;
			radix = 10;
			number_signed = false;
			uppercase = false;
			break; // case PRINTF_STATE_SPEC
		}

		format++;
	}

	return printed;
}

static int vsnprintf_internal(char* restrict fmtStr, size_t size, bool checkSize, const char* restrict format, va_list args){
	const char* s; // '%s' pointer
	int printed = 0; // number of printed characters & index in fmtStr

	int state = PRINTF_STATE_NORMAL;
	bool put_prefix = false;			// Flag '#' => put prefix: "0x"/"0X" for hex, "0" for octal
	bool pad_with_zeros = false;		// Flag '0' => pad with '0' instead of ' ' when width is specified
	char sign = '\0';					// Flag '+' or ' ' => how to print the sign of positives number (\0 means nothing to print)
	int64_t width = 0;					// Width (number) => width specifier (padding)
	int length = PRINTF_LENGTH_DEFAULT;	// Length (h, hh, l, ll) => size of the number to be poped
	bool print_number = true;			// Specifier is a number to print (if not, skip)
	int radix = 10;						// Specifier (d, x, o, ...) => print as decimal, hexadecimal, octal
	bool number_signed = false;			// Specifier (d, u) => print signed, unsigned
	bool uppercase = false;				// Specifier (x, X) => 0x7fff, 0X7FFF

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
				vsnprintf_internal_checkBoundaries(size, printed+1, checkSize);
				fmtStr[printed++] = *format;
				break;
			}
			break;

		case PRINTF_STATE_FLAGS:
			switch (*format){
			case ' ':
				sign = ' ';
				break;
			case '#':
				put_prefix = true;
				break;
			case '+':
				sign = '+';
				break;
			case '0':
				pad_with_zeros = true;
				break;
			default:
				state = PRINTF_STATE_WIDTH;
				goto PRINTF_STATE_WIDTH_;
			}
			break;

		case PRINTF_STATE_WIDTH:
		PRINTF_STATE_WIDTH_:
			switch (*format){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				width *= 10;
				width += (*format - '0'); // convert '0' to '9' to their corresponding number 0-9
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
				vsnprintf_internal_checkBoundaries(size, printed+1, checkSize);
				fmtStr[printed++] = '%';
				print_number = false;
				break;
			case 'X':
				number_signed = false;
				radix = 16;
				uppercase = true;
				sign = '\0';
				break;
			case 'c':
				vsnprintf_internal_checkBoundaries(size, printed+1, checkSize);
				fmtStr[printed++] = (char) va_arg(args, int);
				print_number = false;
				break;
			case 's':
				s = va_arg(args, const char*);
				if (s == NULL) {
					vsnprintf_internal_checkBoundaries(size, printed+6, checkSize);
					fmtStr[printed++] = '(';
					fmtStr[printed++] = 'n';
					fmtStr[printed++] = 'u';
					fmtStr[printed++] = 'l';
					fmtStr[printed++] = 'l';
					fmtStr[printed++] = ')';
				}
				else {
					while(*s){
						vsnprintf_internal_checkBoundaries(size, printed+1, checkSize);
						fmtStr[printed++] = *s;
						s++;
					}
				}
				print_number = false;
				break;
			case 'd':
			case 'i':
				number_signed = true;
				radix = 10;
				break;
			case 'o':
				number_signed = false;
				radix = 8;
				break;
			case 'p':
				length = PRINTF_LENGTH_LONG_LONG; // pointers are 64 bits
				put_prefix = true; // always put 0x prefix for %p
				number_signed = false;
				radix = 16;
				sign = '\0';
				break;
			case 'u':
				number_signed = false;
				radix = 10;
				break;
			case 'x':
				number_signed = false;
				radix = 16;
				sign = '\0';
				uppercase = false;
				break;
			default:
				print_number = false;
				break;
			}

			// Specifier invalid (skip it), or already handled
			if (!print_number)
				goto reset_state;

			switch (length){
			case PRINTF_LENGTH_DEFAULT:
			case PRINTF_LENGTH_SHORT_SHORT:
			case PRINTF_LENGTH_SHORT:
				if (number_signed) {
					int numberToPrint = va_arg(args, int);
					vsnprintf_internal_checkBoundaries(size, printed+getSizeOfNumberToPrint_signed(numberToPrint, radix, sign, put_prefix, width), checkSize);
					printed += sprintf_int(fmtStr+printed, numberToPrint, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				else {
					unsigned int numberToPrint = va_arg(args, unsigned int);
					vsnprintf_internal_checkBoundaries(size, printed+getSizeOfNumberToPrint_unsigned(numberToPrint, radix, sign, put_prefix, width), checkSize);
					printed += sprintf_uint(fmtStr+printed, numberToPrint, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				break;
			case PRINTF_LENGTH_LONG:
				if (number_signed) {
					long numberToPrint = va_arg(args, long);
					vsnprintf_internal_checkBoundaries(size, printed+getSizeOfNumberToPrint_signed(numberToPrint, radix, sign, put_prefix, width), checkSize);
					printed += sprintf_int(fmtStr+printed, numberToPrint, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				else {
					unsigned long numberToPrint = va_arg(args, unsigned long);
					vsnprintf_internal_checkBoundaries(size, printed+getSizeOfNumberToPrint_unsigned(numberToPrint, radix, sign, put_prefix, width), checkSize);
					printed += sprintf_uint(fmtStr+printed, numberToPrint, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				break;
			case PRINTF_LENGTH_LONG_LONG:
				if (number_signed) {
					long long numberToPrint = va_arg(args, long long);
					vsnprintf_internal_checkBoundaries(size, printed+getSizeOfNumberToPrint_signed(numberToPrint, radix, sign, put_prefix, width), checkSize);
					printed += sprintf_int(fmtStr+printed, numberToPrint, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				else {
					unsigned long long numberToPrint = va_arg(args, unsigned long long);
					vsnprintf_internal_checkBoundaries(size, printed+getSizeOfNumberToPrint_unsigned(numberToPrint, radix, sign, put_prefix, width), checkSize);
					printed += sprintf_uint(fmtStr+printed, numberToPrint, radix, sign, uppercase, put_prefix, pad_with_zeros, width);
				}
				break;
			default:
				break;
			}

			reset_state:
			state = PRINTF_STATE_NORMAL;
			put_prefix = false;
			pad_with_zeros = false;
			sign = '\0';
			width = 0;
			length = PRINTF_LENGTH_DEFAULT;
			print_number = true;
			radix = 10;
			number_signed = false;
			uppercase = false;
			break;
		}

		format++;
	}

	end:
	fmtStr[printed] = '\0';
	return printed; // return number of character included, EXCLUDING null-terminating character
}

// ================ printf functions ================

int printf(const char* restrict format, ...){
	va_list args;

	va_start(args, format);
	int res = vdprintf_internal(stdout->fd, format, args);
	va_end(args);

	return res;
}

int fprintf(FILE* restrict stream, const char* restrict format, ...){
	if ((stream == NULL) || (stream->fd < 0))
		return -1;

	va_list args;

	va_start(args, format);
	int res = vdprintf_internal(stream->fd, format, args);
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
	return vdprintf_internal(stdout->fd, format, args);
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
