#include <stdbool.h>
#include "EFI/EFI.h"
#include "EFI/Protocols/GraphicsOutputProtocol.h"

#include "Font.h"

#define SYSV_ABI __attribute__((sysv_abi))

EFI_GRAPHICS_OUTPUT_PROTOCOL* m_gop = NULL;
uint32_t* m_framebuffer; // gop->Mode->FrameBufferBase

typedef uint32_t color_t;
#define COLOR_32BPP(r, g, b)	(r<<16 | g<<8 | b)			// r, g, b: 255 values (bounds not checked!)
#define WHITE					COLOR_32BPP(255,255,255)
#define LIGHT_GREY				COLOR_32BPP(31,31,31)
#define DARK_GREY				COLOR_32BPP(24,24,24)
#define BLACK					COLOR_32BPP(0,0,0)
color_t m_clearColor = LIGHT_GREY;

#define DRAW_OFFSET_X			4 // We don't want characters to cling to the border of the screen
#define DRAW_OFFSET_Y			4

#define MAX_TERMINAL_WIDTH		256
#define MAX_TERMINAL_HEIGHT		512
#define TERMINAL_SIZE			MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT
unsigned char m_terminalText[TERMINAL_SIZE]; // will contain the printed letters
uint32_t m_cursorX;
uint32_t m_cursorY;
uint32_t m_terminalWidth;
uint32_t m_terminalHeight;

#define TAB_SIZE 4

void GOP_puts_noLF(const char* str);

void GOP_clearTerminal(){
	for (int i=0 ; i<TERMINAL_SIZE ; i++) m_terminalText[i] = '\0';
}

void GOP_clearScreen(uint32_t clearColor){
	m_clearColor = clearColor;

	for(int j=0 ; j<m_gop->Mode->Info->VerticalResolution ; j++){
		size_t lineOffset = m_gop->Mode->Info->PixelsPerScanLine*j;
		for (int i=0 ; i<m_gop->Mode->Info->HorizontalResolution ; i++){
			m_framebuffer[lineOffset + i] = clearColor;
		}
	}
}

void GOP_drawLetter(unsigned char letter, uint32_t fontColor, int offsetX, int offsetY){
	uint8_t* bitmap = m_defaultFont[letter];
	int mask; // bit mask, will be shifted right every iteration

	for (int j=0 ; j<BITMAP_CHAR_HEIGHT ; j++){
		size_t lineOffset = m_gop->Mode->Info->PixelsPerScanLine * (j+offsetY);
		mask = 0b10000000;
		for (int i=0 ; i<BITMAP_CHAR_WIDTH ; i++){
			if (bitmap[j] & mask)
				m_framebuffer[lineOffset + i+offsetX] = fontColor;
			else
				m_framebuffer[lineOffset + i+offsetX] = m_clearColor;
			mask >>= 1; // next pixel
		}
	}
}

void GOP_drawScreen(){
	m_cursorX = 0;
	m_cursorY = 0;
	for (int i=0 ; i<TERMINAL_SIZE ; i++){
		char c = m_terminalText[i];
		if (c == '\n'){
			m_cursorX = 0;
			m_cursorY++;
			continue;
		}
		if (c == '\r'){
			m_cursorX = 0;
			continue;
		}
		if (c == '\0')
			continue;
		GOP_drawLetter(c, WHITE, m_cursorX*BITMAP_CHAR_WIDTH + DRAW_OFFSET_X, m_cursorY*BITMAP_CHAR_HEIGHT + DRAW_OFFSET_Y);
		m_cursorX = (m_cursorX+1) % m_terminalWidth;
		if (m_cursorX == 0) m_cursorY++;
	}
}

void GOP_scrollDown(unsigned int n){
	if (n < 0) return;

	// Move lines up
	for(int y=n ; y<m_terminalHeight ; y++){
		for(int x=0 ; x<m_terminalWidth ; x++){
			char c = m_terminalText[m_terminalWidth*y + x];
			m_terminalText[m_terminalWidth*(y-n) + x] = c;
		}
	}

	for(int y=m_terminalHeight-n ; y<m_terminalHeight ; y++){
		for (int x=0; x<m_terminalWidth ; x++){
			m_terminalText[m_terminalWidth*y + x] = '\0';
		}
	}

	m_cursorY -= n;

	// Redraw everything
	GOP_clearScreen(m_clearColor);
	GOP_drawScreen();
}

void GOP_putchar(const char c){
	m_terminalText[m_terminalWidth*m_cursorY + m_cursorX] = c;

	if (c == '\n'){
		m_cursorX = 0;
		m_cursorY++;
		goto end_putc;
	}
	if (c == '\r'){
		m_cursorX = 0;
		goto end_putc;
	}
	if (c == '\t'){
		do {
			GOP_putchar(' '); // recursion is fine, cannot go deeper than one call
		} while(m_cursorX % TAB_SIZE != 0 && m_cursorX<m_terminalWidth);
		goto end_putc;
	}

	GOP_drawLetter(c, WHITE, m_cursorX*BITMAP_CHAR_WIDTH + DRAW_OFFSET_X, m_cursorY*BITMAP_CHAR_HEIGHT + DRAW_OFFSET_Y);

	m_cursorX = (m_cursorX+1) % m_terminalWidth;
	if (m_cursorX == 0) m_cursorY++;

	end_putc:
	if (m_cursorY >= m_terminalHeight) GOP_scrollDown(1);
}

void GOP_puts_noLF(const char* str){
	while(*str){
		GOP_putchar(*str);
		str++;
	}
}

void GOP_puts(const char* str){
	GOP_puts_noLF(str);
	GOP_putchar('\n');
}

SYSV_ABI __attribute__((section(".entry")))
void kmain(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop){
	m_gop = gop;
	m_framebuffer = (uint32_t*) gop->Mode->FrameBufferBase;

	m_cursorX = 0;
	m_cursorY = 0;
	uint64_t maxHorizontalChar = (gop->Mode->Info->HorizontalResolution - 2*DRAW_OFFSET_X) / BITMAP_CHAR_WIDTH;
	uint64_t maxVerticalChar = (gop->Mode->Info->VerticalResolution - 2*DRAW_OFFSET_Y) / BITMAP_CHAR_HEIGHT;
	m_terminalWidth = (maxHorizontalChar > MAX_TERMINAL_WIDTH) ? MAX_TERMINAL_WIDTH : maxHorizontalChar;
	m_terminalHeight = (maxVerticalChar > MAX_TERMINAL_HEIGHT) ? MAX_TERMINAL_HEIGHT : maxVerticalChar;
	GOP_clearTerminal();
	GOP_clearScreen(COLOR_32BPP(31, 31, 31));

	GOP_puts("Supposons que je sois dans votre kernel !\n");

	GOP_puts("Testing character set:");
	for (int letter=0 ; letter<256 ; letter++)
		GOP_putchar(letter);

	while(true){
		__asm__ volatile("cli; hlt");
	}
}
