#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "io.h"
#include "stdio.h"

#define PRINTF_NUMBER_BUFFER_SIZE 32

#define TAB_SIZE 4

// temporary hardcode putchar to VGA_putchar
#include "Drivers/VGA.h"
#define putchar VGA_putchar

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

void puts_no_lf(const char* str){
	while(*str){
		putchar(*str);
		str++;
	}
}

void puts(const char* str){
	puts_no_lf(str);
	putchar('\n');
}

static inline void printf_unsigned(unsigned long long number, int radix){

	// All possible characters that we can encouter
	const char hexChars[] = "0123456789abcdef";

	char buffer[PRINTF_NUMBER_BUFFER_SIZE];
	int pos = 0; // position in the buffer

	// Convert the number to ASCII
	do {
		unsigned long long rem = number % radix;
		number /= radix;

		buffer[pos++] = hexChars[rem];
	} while(number > 0 && pos<PRINTF_NUMBER_BUFFER_SIZE);

	// Print the parsed number to the screen
	while (--pos >= 0) putchar(buffer[pos]);
}

static inline void printf_signed(long long number, int radix){
	if (number < 0){
		putchar('-');
		number = -number;
	}

	printf_unsigned(number, radix);
}

void printf(const char* formatted_string, ...){
	// Variadic argument initalizer
	va_list args;
	va_start(args, formatted_string);

	int state = PRINTF_STATE_NORMAL;
	int length = PRINTF_LENGTH_DEFAULT;
	bool number = false;
	bool sign = false;
	int radix = 10;

	while(*formatted_string){

		switch(state){

			case PRINTF_STATE_NORMAL:
				switch(*formatted_string){
					case '%':
						state = PRINTF_STATE_LENGTH;
						break;
					default:
						putchar(*formatted_string);
						break;
				}
				break;

			case PRINTF_STATE_LENGTH:
				switch(*formatted_string){
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
				if (*formatted_string == 'h'){
					length = PRINTF_LENGTH_SHORT_SHORT;
					state = PRINTF_STATE_SPEC;
				}
				else {
					goto PRINTF_STATE_SPEC_;
				}
				break;

			case PRINTF_STATE_LENGTH_LONG:
				if (*formatted_string == 'l'){
					length = PRINTF_LENGTH_LONG_LONG;
					state = PRINTF_STATE_SPEC;
				}
				else {
					goto PRINTF_STATE_SPEC_;
				}
				break;

			case PRINTF_STATE_SPEC:
				PRINTF_STATE_SPEC_:
				switch(*formatted_string){
					case 'c':
						// Note: we call va_args with int size, and not char, because in 32bits pmode
						// all arguments pushed onto the stack are rounded to 32 bits
						putchar((char) va_arg(args, int));
						break;
					case 's':
						puts_no_lf(va_arg(args, const char*));
						break;
					case '%':
						putchar('%');
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
						putchar('0');
						putchar('x');
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
					if (sign) printf_signed(va_arg(args, int), radix);
					else printf_unsigned(va_arg(args, unsigned int), radix);
					break;
				case PRINTF_LENGTH_LONG:
					if (sign) printf_signed(va_arg(args, long), radix);
					else printf_unsigned(va_arg(args, unsigned long), radix);
					break;
				case PRINTF_LENGTH_LONG_LONG:
					if (sign) printf_signed(va_arg(args, long long), radix);
					else printf_unsigned(va_arg(args, unsigned long long), radix);
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

		formatted_string++;
	}

	va_end(args);
}
