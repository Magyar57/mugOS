#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "stdio.h"
#include "x86.h"

#define PRINTF_NUMBER_BUFFER_SIZE 32

#define TAB_SIZE 4

uint8_t* g_ScreenBuffer = (uint8_t*) 0x000b8000;
#define SCREEN_WIDTH  80 // Default text mode screen width
#define SCREEN_HEIGHT 25 // Default text mode screen height
#define DEFAULT_COLOR (0 << 4) | (7) // (background_color << 4) | (character_color)
int g_screenX = 0; // Cursor's X position
int g_screenY = 12; // Cursor's Y position

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

static inline void putc_xy(int x, int y, char c){
	// Note the multiplication by 2: this is because the first byte is the char on screen,
	// and the second byte is the color (4-bits foreground and 4-bits background)
	x = x % SCREEN_WIDTH;
	y = y % SCREEN_HEIGHT;

	g_ScreenBuffer[2*(y*SCREEN_WIDTH + x)] = c;
}

static inline void put_color_xy(int x, int y, uint8_t color){
	x = x % SCREEN_WIDTH;
	y = y % SCREEN_HEIGHT;

	g_ScreenBuffer[2*(y*SCREEN_WIDTH + x) + 1] = color;
}

static inline char getc_xy(int x, int y){
	x = x % SCREEN_WIDTH;
	y = y % SCREEN_HEIGHT;

	return g_ScreenBuffer[2*(y*SCREEN_WIDTH + x)];
}

static inline char get_color_xy(int x, int y){
	x = x % SCREEN_WIDTH;
	y = y % SCREEN_HEIGHT;

	return g_ScreenBuffer[2*(y*SCREEN_WIDTH + x) + 1];
}

void putc(char c){
	// Instead of using the BIOS interupt (x86_Video_WriteCharTeletype), in protected mode
	// we need to interact with the screen IO, which is memory-mapped
	// x86_Video_WriteCharTeletype(c, 0);

	if (c == '\n'){
		g_screenX = 0;
		g_screenY++;
		goto end_putc;
	}
	if (c == '\r'){
		g_screenX = 0;
		goto end_putc;
	}
	if (c == '\t'){
		do {
			putc_xy(g_screenX, g_screenY, ' ');
			g_screenX++;
		} while(g_screenX % TAB_SIZE != 0 && g_screenX<SCREEN_WIDTH);
		goto end_putc;
	}

	putc_xy(g_screenX, g_screenY, c);

	g_screenX = (g_screenX+1) % SCREEN_WIDTH;
	if (g_screenX == 0) g_screenY++;

	end_putc:
	if (g_screenY >= SCREEN_HEIGHT) scroll_down(1);
	set_cursor_position(g_screenX, g_screenY);
}

void puts_no_lf(const char* str){
	while(*str){
		putc(*str);
		str++;
	}
}

void puts(const char* str){
	puts_no_lf(str);
	putc('\n');
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
	while (--pos >= 0) putc(buffer[pos]);
}

static inline void printf_signed(long long number, int radix){
	if (number < 0){
		putc('-');
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
						// Note: we call va_args with int size, and not char, because in 32bits pmode
						// all arguments pushed onto the stack are rounded to 32 bits
						putc((char) va_arg(args, int));
						break;
					case 's':
						puts_no_lf(va_arg(args, const char*));
						break;
					case '%':
						putc('%');
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
						putc('0');
						putc('x');
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

void clear_screen(){
	for(int y=0 ; y<SCREEN_HEIGHT ; y++){
		for(int x=0 ; x<SCREEN_WIDTH ; x++){
			putc_xy(x, y, ' ');
			// put_color_xy(x, y, y*SCREEN_WIDTH+x);
			put_color_xy(x, y, DEFAULT_COLOR);
		}
	}

	g_screenX = 0;
	g_screenY = 0;
	set_cursor_position(g_screenX, g_screenY);
}

void set_cursor_position(int x, int y){
	int new_pos = y * SCREEN_WIDTH + x;

	// Lower byte
	x86_outb(0x3d4, 0x0f);
	x86_outb(0x3d5, (uint8_t) 0x000000ff & new_pos);
	// Upper byte
	x86_outb(0x3d4, 0x0e);
	x86_outb(0x3d5, (uint8_t) ((0x0000ff00 & new_pos) >> 8));
}

void scroll_down(int n){
	for(int y=n ; y<SCREEN_HEIGHT ; y++){
		for(int x=0 ; x<SCREEN_WIDTH ; x++){
			char c = getc_xy(x, y);
			char color = get_color_xy(x, y);
			putc_xy(x, y-n, c);
			put_color_xy(x, y-n, color);
		}
	}

	for(int y=SCREEN_HEIGHT-n ; y<SCREEN_HEIGHT ; y++){
		for (int x=0; x<SCREEN_WIDTH ; x++){
			putc_xy(x, y, '\0');
			put_color_xy(x, y, DEFAULT_COLOR);
		}
	}

	g_screenY -= n;
	set_cursor_position(g_screenX, g_screenY);
}
