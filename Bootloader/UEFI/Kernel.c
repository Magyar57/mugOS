#include <stdbool.h>
#include "EFI/EFI.h"
#include "EFI/Protocols/GraphicsOutputProtocol.h"

#include "Font.h"

typedef uint32_t gop_color_t;
#define COLOR_32BPP(r, g, b)	(r<<16 | g<<8 | b)			// r, g, b: 255 values (bounds not checked!)
#define WHITE					COLOR_32BPP(255,255,255)
#define LIGHT_GREY				COLOR_32BPP(31,31,31)
#define DARK_GREY				COLOR_32BPP(24,24,24)
#define BLACK					COLOR_32BPP(0,0,0)

#define MAX_TERMINAL_WIDTH		256
#define MAX_TERMINAL_HEIGHT		512
#define TERMINAL_SIZE			MAX_TERMINAL_WIDTH*MAX_TERMINAL_HEIGHT

#define TAB_SIZE 4

typedef struct s_GOPDriver {
	// Framebuffer and drawing stuff
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	uint32_t* framebuffer;		// gop->Mode->FrameBufferBase TODO handle smaller buffers (less than 32bpp)
	gop_color_t clearColor;		// Framebufer's background/clear color
	uint32_t drawOffsetX;		// Character X offset to the border of the screen
	uint32_t drawOffsetY;		// Character Y offset to the border of the screen	
	// Character array
	uint32_t cursorX;			// Cursor X position in terminalText
	uint32_t cursorY;			// Cursor Y position in terminalText
	uint32_t terminalWidth;
	uint32_t terminalHeight;
	unsigned char terminalText[TERMINAL_SIZE]; // Contains the printed letters
} GOPDriver;

GOPDriver g_gopDriver;

void GOP_puts_noLF(GOPDriver* this, const char* str);

void GOP_clearTerminal(GOPDriver* this){
	for (int i=0 ; i<TERMINAL_SIZE ; i++) this->terminalText[i] = '\0';
}

void GOP_clearScreen(GOPDriver* this, uint32_t clearColor){
	this->clearColor = clearColor;

	for(int j=0 ; j<this->gop->Mode->Info->VerticalResolution ; j++){
		size_t lineOffset = this->gop->Mode->Info->PixelsPerScanLine*j;
		for (int i=0 ; i<this->gop->Mode->Info->HorizontalResolution ; i++){
			this->framebuffer[lineOffset + i] = clearColor;
		}
	}
}

void GOP_drawLetter(GOPDriver* this, unsigned char letter, uint32_t fontColor, int offsetX, int offsetY){
	uint8_t* bitmap = m_defaultFont[letter];
	int mask; // bit mask, will be shifted right every iteration

	for (int j=0 ; j<BITMAP_CHAR_HEIGHT ; j++){
		size_t lineOffset = this->gop->Mode->Info->PixelsPerScanLine * (j+offsetY);
		mask = 0b10000000;
		for (int i=0 ; i<BITMAP_CHAR_WIDTH ; i++){
			if (bitmap[j] & mask)
				this->framebuffer[lineOffset + i+offsetX] = fontColor;
			else
				this->framebuffer[lineOffset + i+offsetX] = this->clearColor;
			mask >>= 1; // next pixel
		}
	}
}

void GOP_drawScreen(GOPDriver* this){
	this->cursorX = 0;
	this->cursorY = 0;
	for (int i=0 ; i<TERMINAL_SIZE ; i++){
		char c = this->terminalText[i];
		if (c == '\n'){
			this->cursorX = 0;
			this->cursorY++;
			continue;
		}
		if (c == '\r'){
			this->cursorX = 0;
			continue;
		}
		if (c == '\0')
			continue;
		GOP_drawLetter(this, c, WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);
		this->cursorX = (this->cursorX+1) % this->terminalWidth;
		if (this->cursorX == 0) this->cursorY++;
	}
}

void GOP_scrollDown(GOPDriver* this, unsigned int n){
	if (n < 0) return;

	// Move lines up
	for(int y=n ; y<this->terminalHeight ; y++){
		for(int x=0 ; x<this->terminalWidth ; x++){
			char c = this->terminalText[this->terminalWidth*y + x];
			this->terminalText[this->terminalWidth*(y-n) + x] = c;
		}
	}

	for(int y=this->terminalHeight-n ; y<this->terminalHeight ; y++){
		for (int x=0; x<this->terminalWidth ; x++){
			this->terminalText[this->terminalWidth*y + x] = '\0';
		}
	}

	this->cursorY -= n;

	// Redraw everything
	GOP_clearScreen(this, this->clearColor);
	GOP_drawScreen(this);
}

void GOP_putchar(GOPDriver* this, const char c){
	this->terminalText[this->terminalWidth*this->cursorY + this->cursorX] = c;

	if (c == '\n'){
		this->cursorX = 0;
		this->cursorY++;
		goto end_putc;
	}
	if (c == '\r'){
		this->cursorX = 0;
		goto end_putc;
	}
	if (c == '\t'){
		do {
			GOP_putchar(this, ' '); // recursion is fine, cannot go deeper than one call
		} while(this->cursorX % TAB_SIZE != 0 && this->cursorX<this->terminalWidth);
		goto end_putc;
	}

	GOP_drawLetter(this, c, WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);

	this->cursorX = (this->cursorX+1) % this->terminalWidth;
	if (this->cursorX == 0) this->cursorY++;

	end_putc:
	if (this->cursorY >= this->terminalHeight) GOP_scrollDown(this, 1);
}

void GOP_puts_noLF(GOPDriver* this, const char* str){
	while(*str){
		GOP_putchar(this, *str);
		str++;
	}
}

void GOP_puts(GOPDriver* this, const char* str){
	GOP_puts_noLF(this, str);
	GOP_putchar(this, '\n');
}

void GOP_initialize(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop){
	g_gopDriver.gop = gop;
	g_gopDriver.framebuffer = (uint32_t*) gop->Mode->FrameBufferBase;

	g_gopDriver.drawOffsetX = 4;
	g_gopDriver.drawOffsetY = 4;

	uint64_t maxHorizontalChar = (gop->Mode->Info->HorizontalResolution - 2*g_gopDriver.drawOffsetX) / BITMAP_CHAR_WIDTH;
	uint64_t maxVerticalChar = (gop->Mode->Info->VerticalResolution - 2*g_gopDriver.drawOffsetY) / BITMAP_CHAR_HEIGHT;
	g_gopDriver.terminalWidth = (maxHorizontalChar > MAX_TERMINAL_WIDTH) ? MAX_TERMINAL_WIDTH : maxHorizontalChar;
	g_gopDriver.terminalHeight = (maxVerticalChar > MAX_TERMINAL_HEIGHT) ? MAX_TERMINAL_HEIGHT : maxVerticalChar;

	g_gopDriver.cursorX = 0;
	g_gopDriver.cursorY = 0;
}

__attribute__((section(".entry")))
void kmain(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop){

	GOP_initialize(gop);
	GOP_clearTerminal(&g_gopDriver);
	GOP_clearScreen(&g_gopDriver, COLOR_32BPP(31, 31, 31));

	GOP_puts(&g_gopDriver, "Supposons que je sois dans votre kernel !\n");

	GOP_puts(&g_gopDriver, "Testing character set:");
	for (int letter=0 ; letter<256 ; letter++)
		GOP_putchar(&g_gopDriver, letter);

	while(true){
		__asm__ volatile("cli; hlt");
	}
}
