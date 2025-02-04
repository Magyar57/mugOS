#include <stdint.h>
#include <stddef.h>
#include "Logging.h"
#include "Drivers/Graphics/Font.h"

#include "Drivers/Graphics/Framebuffer.h"

#define MODULE "Framebuffer"

void Framebuffer_clearTerminal(Framebuffer* this){
	for (int i=0 ; i<TERMINAL_SIZE ; i++) this->terminalText[i] = '\0';
}

void Framebuffer_setZoom(Framebuffer* this, uint32_t zoom){
	if (zoom == 0)
		zoom = 1;

	// Recompute what's needed

	this->zoom = zoom;
	this->charWidth = zoom * BITMAP_CHAR_WIDTH;
	this->charHeight = zoom * BITMAP_CHAR_HEIGHT;

	uint64_t maxHorizontalChar = (this->width - 2*this->drawOffsetX*this->zoom) / (BITMAP_CHAR_WIDTH*this->zoom);
	uint64_t maxVerticalChar = (this->height - 2*this->drawOffsetY*this->zoom) / (BITMAP_CHAR_HEIGHT*this->zoom);
	this->terminalWidth = (maxHorizontalChar > MAX_TERMINAL_WIDTH) ? MAX_TERMINAL_WIDTH : maxHorizontalChar;
	this->terminalHeight = (maxVerticalChar > MAX_TERMINAL_HEIGHT) ? MAX_TERMINAL_HEIGHT : maxVerticalChar;
}

void Framebuffer_setClearColor(Framebuffer* this, uint32_t clearColor){
	this->clearColor = clearColor;
}

void Framebuffer_clearScreen(Framebuffer* this){
	for(int j=0 ; j<this->height ; j++){
		size_t lineOffset = this->width*j;
		for (int i=0 ; i<this->width ; i++){
			this->address[lineOffset + i] = this->clearColor;
		}
	}
}

void Framebuffer_drawLetter(Framebuffer* this, unsigned char letter, uint32_t fontColor, int offsetX, int offsetY){
	uint8_t* bitmap = m_defaultFont[letter];
	int mask; // bit mask, will be shifted right every iteration
	gop_color_t colorToDraw;

	// Cache as many things as possible, since this is a critical section (performance-wise)
	uint64_t pixelsPerScanLine = this->width; // TODO CHECK
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
					this->address[iterationLineOffset + horizontalOffset+ii] = colorToDraw;
				}
			}

			mask >>= 1; // next bitmap pixel
		}
	}
}

void Framebuffer_drawScreen(Framebuffer* this){
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
		Framebuffer_drawLetter(this, c, WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);
		this->cursorX = (this->cursorX+1) % this->terminalWidth;
		if (this->cursorX == 0) this->cursorY++;
	}
}

void Framebuffer_scrollDown(Framebuffer* this, unsigned int n){
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
	Framebuffer_clearScreen(this);
	Framebuffer_drawScreen(this);
}

void Framebuffer_putchar(Framebuffer* this, const char c){
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
			Framebuffer_putchar(this, ' '); // recursion is fine, cannot go deeper than one call
		} while(this->cursorX % TAB_SIZE != 0 && this->cursorX<this->terminalWidth);
		goto end_putc;
	}

	Framebuffer_drawLetter(this, c, WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);

	this->cursorX = (this->cursorX+1) % this->terminalWidth;
	if (this->cursorX == 0) this->cursorY++;

	end_putc:
	if (this->cursorY >= this->terminalHeight) Framebuffer_scrollDown(this, 1);
}

void Framebuffer_puts_noLF(Framebuffer* this, const char* str){
	while(*str){
		Framebuffer_putchar(this, *str);
		str++;
	}
}

void Framebuffer_puts(Framebuffer* this, const char* str){
	Framebuffer_puts_noLF(this, str);
	Framebuffer_putchar(this, '\n');
}

bool Framebuffer_initialize(Framebuffer* this){
	// Sanitize check the needed variables were intialized correctly beforehand
	if (this->address == NULL || this->width==0 || this->height==0 || this->pitch==0 || this->bpp==0)
		return false;

	if (this->bpp != 32){
		log(ERROR, MODULE, "A value of %d for bpp (Bits per pixel) is unsupported (only 32 is supported)", this->bpp);
		return false;
	}

	this->drawOffsetX = 4;
	this->drawOffsetY = 4;
	Framebuffer_setClearColor(this, LIGHT_GREY);
	Framebuffer_setZoom(this, 1); // setZoom recomputes some internal variables, so we don't need to set those

	Framebuffer_clearTerminal(this);
	Framebuffer_clearScreen(this);

	this->cursorX = 0;
	this->cursorY = 0;

	return true;
}
