#include <stdint.h>
#include <stddef.h>
#include "EFI/Protocols/GraphicsOutputProtocol.h"
#include "Drivers/Graphics/Font.h"

#include "Drivers/Graphics/GOP.h"

void GOP_clearTerminal(GOPDriver* this){
	for (int i=0 ; i<TERMINAL_SIZE ; i++) this->terminalText[i] = '\0';
}

void GOP_setZoom(GOPDriver* this, uint32_t zoom){
	if (zoom == 0)
		zoom = 1;

	// Recompute what's needed

	this->zoom = zoom;
	this->charWidth = zoom * BITMAP_CHAR_WIDTH;
	this->charHeight = zoom * BITMAP_CHAR_HEIGHT;

	uint64_t maxHorizontalChar = (((EFI_GRAPHICS_OUTPUT_PROTOCOL*) this->gop)->Mode->Info->HorizontalResolution - 2*this->drawOffsetX*this->zoom) / (BITMAP_CHAR_WIDTH*this->zoom);
	uint64_t maxVerticalChar = (((EFI_GRAPHICS_OUTPUT_PROTOCOL*) this->gop)->Mode->Info->VerticalResolution - 2*this->drawOffsetY*this->zoom) / (BITMAP_CHAR_HEIGHT*this->zoom);
	this->terminalWidth = (maxHorizontalChar > MAX_TERMINAL_WIDTH) ? MAX_TERMINAL_WIDTH : maxHorizontalChar;
	this->terminalHeight = (maxVerticalChar > MAX_TERMINAL_HEIGHT) ? MAX_TERMINAL_HEIGHT : maxVerticalChar;
}

void GOP_setClearColor(GOPDriver* this, uint32_t clearColor){
	this->clearColor = clearColor;
}

void GOP_clearScreen(GOPDriver* this){
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = (EFI_GRAPHICS_OUTPUT_PROTOCOL*) this->gop;

	for(int j=0 ; j<gop->Mode->Info->VerticalResolution ; j++){
		size_t lineOffset = gop->Mode->Info->PixelsPerScanLine*j;
		for (int i=0 ; i<gop->Mode->Info->HorizontalResolution ; i++){
			this->framebuffer[lineOffset + i] = this->clearColor;
		}
	}
}

void GOP_drawLetter(GOPDriver* this, unsigned char letter, uint32_t fontColor, int offsetX, int offsetY){
	uint8_t* bitmap = m_defaultFont[letter];
	int mask; // bit mask, will be shifted right every iteration
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = (EFI_GRAPHICS_OUTPUT_PROTOCOL*) this->gop;
	gop_color_t colorToDraw;

	// Cache as many things as possible, since this is a critical section (performance-wise)
	uint64_t pixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
	uint64_t zoom = this->zoom;
	uint64_t zoomedOffsetX = zoom * offsetX;

	// i, j: indexes in the bitmap character
	for (int j=0 ; j<BITMAP_CHAR_HEIGHT ; j++){
		
		uint64_t baseLineOffset = pixelsPerScanLine * zoom * (j+offsetY);

		mask = 0b10000000;
		for (int i=0 ; i<BITMAP_CHAR_WIDTH ; i++){
			// Draw pixel if the bit is set in the bitmap
			colorToDraw = (bitmap[j] & mask) ? fontColor : this->clearColor;
			// More cache
			uint64_t horizontalOffset = zoom*i + zoomedOffsetX;

			// ii, jj: pixel indexes in the framebuffer
			for (int jj=0 ; jj<zoom ; jj++){
				uint64_t iterationLineOffset = baseLineOffset + jj*pixelsPerScanLine;
				for (int ii=0 ; ii<zoom ; ii++){
					this->framebuffer[iterationLineOffset + horizontalOffset+ii] = colorToDraw;
				}
			}

			mask >>= 1; // next bitmap pixel
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
	GOP_clearScreen(this);
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

void GOP_initialize(GOPDriver* this, void* gop){
	this->gop = (void*) gop;
	this->framebuffer = (uint32_t*) ((EFI_GRAPHICS_OUTPUT_PROTOCOL*) gop)->Mode->FrameBufferBase;

	this->drawOffsetX = 4;
	this->drawOffsetY = 4;
	GOP_setClearColor(this, LIGHT_GREY);
	GOP_setZoom(this, 1); // setZoom recomputes some internal variables, so we don't need to set those

	GOP_clearTerminal(this);
	GOP_clearScreen(this);

	this->cursorX = 0;
	this->cursorY = 0;
}
