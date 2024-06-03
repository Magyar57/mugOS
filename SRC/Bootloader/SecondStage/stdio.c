#include "x86.h"
#include "stdint.h"

#include "stdio.h"

#define PRINTF_NUMBER_BUFFER_SIZE 32

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

void putc(char c){
	x86_Video_WriteCharTeletype(c, 0);
}

void puts(const char* str){
	while(*str){
		putc(*str);
		str++;
	}
}

static inline int* printf_number(int* argp, int length, bool isSigned, int radix){

	// All possible characters that we can encouter
	const char hexChars[] = "0123456789abcdef";
	
	char buffer[PRINTF_NUMBER_BUFFER_SIZE];
	int pos = 0; // position in the buffer
	unsigned long long number;
	int sign = 1; // 1: positive ; -1: negative

	// Convert the number to print to a positive unsigned long long, and put the sign information into "sign"
	switch(length){
		case PRINTF_LENGTH_SHORT_SHORT:
		case PRINTF_LENGTH_SHORT:
		case PRINTF_LENGTH_DEFAULT:
			if(isSigned){
				int n = *argp;
				if (n<0){
					n = -n; // we parse the positive number
					sign = -1;
				}
				number = (unsigned long long) n;
			}
			else {
				number = *(unsigned int*)argp;
			}
			argp++;
			break;
		
		case PRINTF_LENGTH_LONG:
			if(isSigned){
				long int n = *(long int*)argp;
				if (n<0){
					n = -n; // we parse the positive number
					sign = -1;
				}
				number = (unsigned long long) n;
			}
			else {
				number = *(unsigned long int*)argp;
			}
			argp += 2;
			break;
		
		case PRINTF_LENGTH_LONG_LONG:
			if(isSigned){
				long long int n = *(long long int*)argp;
				if (n<0){
					n = -n; // we parse the positive number
					sign = -1;
				}
				number = (unsigned long long) n;
			}
			else {
				number = *(unsigned long long int*)argp;
			}
			argp += 4;
			break;
	}
	
	// Convert the number to ASCII
	do {
		uint32_t rem;
		x86_div64_32(number, radix, &number, &rem); // rem = number % radix ; number = number / radix;

		buffer[pos++] = hexChars[rem];
	} while(number > 0 && pos<PRINTF_NUMBER_BUFFER_SIZE);

	// Print the parsed number to the screen
	if (isSigned && sign == -1) putc('-');
	while (--pos >= 0) putc(buffer[pos]);

	return argp;
}

void _cdecl printf(const char* formatted_string, ...){
	// pointer to the stack, initalized at the address of first argument of the function
	// we know that the stack must be aligned to the size of the "int" datatype. So we can use an int pointer
	int* argp = (int*) &formatted_string;
	argp++;

	int state = PRINTF_STATE_NORMAL;
	int length = PRINTF_LENGTH_DEFAULT;

	while(*formatted_string){
		
		switch(state){
			
			case PRINTF_STATE_NORMAL:
				switch(*formatted_string){
					case '%':
						state = PRINTF_STATE_LENGTH;
						break;
					default:
						putc(*formatted_string);
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
						putc((char) *argp);
						argp++;
						break;
					case 's':
						puts(*(char**)argp);
						argp++;
						break;
					case '%':
						putc('%');
						break;
					case 'd':
					case 'i':
						argp = printf_number(argp, length, true, 10);
						break;
					case 'u':
						argp = printf_number(argp, length, false, 10);
						break;
					case 'X':
					case 'x':
					case 'p':
						argp = printf_number(argp, length, false, 16);
						break;
					case 'o':
						argp = printf_number(argp, length, false, 8);
						break;
					default:
						break; // ignore invalid specifiers
				}

			// reset state
			state = PRINTF_STATE_NORMAL;
			length = PRINTF_LENGTH_DEFAULT;
			break;
		}
		
		formatted_string += sizeof(formatted_string) / sizeof(int);
		// or formatted_string++; ?
	}
}
