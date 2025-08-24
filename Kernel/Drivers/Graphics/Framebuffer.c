#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "assert.h"
#include "IO.h"
#include "IRQ.h"
#include "Logging.h"
#include "Drivers/Graphics/Font.h"

#include "Drivers/Graphics/Framebuffer.h"

#define MODULE "Framebuffer"

// At first, we were assuming that pitch != width * (bpp/8)
// To fix this, we compute the line offset (in number of pixels) instead:
// offset = pitch / (bpp/8)
// Note: we assume that pitch is a multiple of the pixel byte size (aka 4 bytes for 32 bpp)
#define getLineOffset() 8*(this->pitch/this->bpp)

void Framebuffer_clearTerminal(Framebuffer* this){
	assert(this);
	memset(this->text, '\0', TERMINAL_SIZE);
}

void Framebuffer_setZoom(Framebuffer* this, uint32_t zoom){
	assert(this);

	if (zoom == 0)
		zoom = 1;

	// Recompute what's needed

	this->zoom = zoom;
	this->charWidth = zoom * BITMAP_CHAR_WIDTH;
	this->charHeight = zoom * BITMAP_CHAR_HEIGHT;

	uint64_t maxHorizontalChar = (this->width - 2*this->drawOffsetX*this->zoom) / (BITMAP_CHAR_WIDTH*this->zoom);
	uint64_t maxVerticalChar = (this->height - 2*this->drawOffsetY*this->zoom) / (BITMAP_CHAR_HEIGHT*this->zoom);
	this->textWidth = min(maxHorizontalChar, MAX_TERMINAL_WIDTH);
	this->textHeight = min(maxVerticalChar, MAX_TERMINAL_HEIGHT);
}

void Framebuffer_setClearColor(Framebuffer* this, color_t clearColor){
	assert(this);
	this->clearColor = clearColor;
}

void Framebuffer_setFontColor(Framebuffer* this, color_t fontColor){
	assert(this);
	this->fontColor = fontColor;
}

void Framebuffer_clearScreen(Framebuffer* this){
	assert(this);
	const int one_line_offset = getLineOffset();

	for(uint64_t j=0 ; j<this->height ; j++){
		size_t lineOffset = j*one_line_offset;
		for (uint64_t i=0 ; i<this->width ; i++){
			writeRelaxed32(this->address + lineOffset + i, this->clearColor);
		}
	}
	writeMemoryBarrier();
}

void Framebuffer_drawLetter(Framebuffer* this, unsigned char letter, uint32_t fontColor, unsigned int offsetX, unsigned int offsetY){
	assert(this);
	assert((offsetX+this->charWidth) < this->width);
	assert((offsetY+this->charHeight) < this->height);

	const uint8_t* bitmap = DEFAULT_FONT[letter];
	int mask; // bit mask, will be shifted right every iteration
	color_t colorToDraw;
	uint32_t* pixel;

	// Cache as many things as possible, since this is a critical section (performance-wise)
	const uint64_t one_line_offet = getLineOffset();
	const uint64_t zoom = this->zoom;
	const uint64_t zoomedOffsetX = zoom * offsetX;
	uint64_t baseLineOffset;

	// i, j: indexes in the bitmap character
	for (int j=0 ; j<BITMAP_CHAR_HEIGHT ; j++){

		baseLineOffset = one_line_offet * zoom * (j+offsetY);
		mask = 0b10000000;
		for (int i=0 ; i<BITMAP_CHAR_WIDTH ; i++){
			// Draw pixel if the bit is set in the bitmap
			colorToDraw = (bitmap[j] & mask) ? fontColor : this->clearColor;
			// More cache
			uint64_t horizontalOffset = zoom*i + zoomedOffsetX;

			// ii, jj: pixel indexes in the framebuffer
			for (uint64_t jj=0 ; jj<zoom ; jj++){
				uint64_t iterationLineOffset = baseLineOffset + jj*one_line_offet;
				for (uint64_t ii=0 ; ii<zoom ; ii++){
					pixel = this->address + iterationLineOffset + horizontalOffset + ii;
					writeRelaxed32(pixel, colorToDraw);
				}
			}

			mask >>= 1; // next bitmap pixel
		}
	}
	writeMemoryBarrier();
}

// (Re)draws the entire screen. IRQ UNSAFE !
static inline void drawScreen(Framebuffer* this){
	assert(this);

	this->cursorX = 0;
	this->cursorY = 0;

	for (uint32_t j=0 ; j<this->textHeight ; j++){
		for (uint32_t i=0 ; i<this->textWidth ; i++){
			char c = this->text[j*MAX_TERMINAL_WIDTH + i];

			if (c == '\n'){
				this->cursorX = 0;
				this->cursorY++;
				break;
			}

			switch (c){
			case '\0':
				break;
			case '\n':
				assert(false); // handled already
				break;
			case '\r':
				assert(false); // should not happen, '\r' is never written in text
				break;
			case '\t':
				assert(false);
				break;
			default:
				const unsigned int x = this->cursorX*this->charWidth + this->drawOffsetX;
				const unsigned int y = this->cursorY*this->charHeight + this->drawOffsetY;
				Framebuffer_drawLetter(this, c, this->fontColor, x, y);
				this->cursorX = (this->cursorX+1) % this->textWidth;
				if (this->cursorX == 0) this->cursorY++;
				break;
			}
		}
	}
}

void Framebuffer_scrollDown(Framebuffer* this){
	assert(this);
	unsigned long flags;
	IRQ_disableSave(flags);

	// Move the terminal text one line up
	memmove(this->text, this->text+MAX_TERMINAL_WIDTH, TERMINAL_SIZE-MAX_TERMINAL_WIDTH);
	memset(this->text + TERMINAL_SIZE-MAX_TERMINAL_WIDTH, 0, MAX_TERMINAL_WIDTH); // reset last line

	// Redraw everything (cursor is updated by drawScreen)
	Framebuffer_clearScreen(this);
	drawScreen(this);

	IRQ_restore(flags);
}

void Framebuffer_putchar(Framebuffer* this, const char c){
	assert(this);
	static uint32_t endline_pos = 0; // track where the last character in the line is (for proper '\r' support)

	if (c == '\0')
		return;

	unsigned long flags;
	IRQ_disableSave(flags);

	switch (c){
	case '\t':
		do {
			Framebuffer_putchar(this, ' ');
		} while (this->cursorX % TAB_SIZE != 0);
		break;

	case '\n':
		this->text[this->cursorY*MAX_TERMINAL_WIDTH + endline_pos] = '\n';
		this->cursorX = 0;
		this->cursorY++;
		endline_pos = 0; // new line
		break;

	case '\r':
		this->cursorX = 0;
		break;

	default:
		this->text[this->cursorY*MAX_TERMINAL_WIDTH + this->cursorX] = c;
		unsigned int x = this->cursorX*this->charWidth + this->drawOffsetX;
		unsigned int y =this->cursorY*this->charHeight + this->drawOffsetY;
		Framebuffer_drawLetter(this, c, this->fontColor, x, y);

		this->cursorX = (this->cursorX+1) % this->textWidth;
		if (this->cursorX == 0) {
			this->cursorY++;
			endline_pos = 0;
		}
		else {
			endline_pos = max(endline_pos, this->cursorX);
		}
		break;
	}

	if (this->cursorY >= this->textHeight)
		Framebuffer_scrollDown(this);

	IRQ_restore(flags);
}

void Framebuffer_puts_noLF(Framebuffer* this, const char* str){
	assert(this);
	if (str==NULL) return;

	while(*str){
		Framebuffer_putchar(this, *str);
		str++;
	}
}

void Framebuffer_puts(Framebuffer* this, const char* str){
	Framebuffer_puts_noLF(this, str);
	Framebuffer_putchar(this, '\n');
}

void Framebuffer_putPixel(Framebuffer* this, unsigned int x, unsigned int y, color_t pixel){
	assert(this);
	assert(x < this->width);
	assert(y < this->height);

	const int offset = getLineOffset();
	write32(this->address + offset*y + x, pixel);
}

void Framebuffer_drawRectangle(Framebuffer* this, unsigned int x, unsigned int y, unsigned int width, unsigned int height, color_t c){
	assert(this);
	const unsigned int final_x = x+width;
	const unsigned int final_y = y+height;
	assert(final_x < this->width);
	assert(final_y < this->height);

	const unsigned int offset = getLineOffset();
	unsigned int line_cache;

	// Top line
	line_cache = y*offset;
	for (unsigned int i=line_cache+x ; i<line_cache+final_x ; i++)
		write32(this->address + i, c);

	// Left line
	// line_cache = y*offset; // but line_cache is the same, no need to recompute it
	for (unsigned int j=line_cache+x ; j<offset*final_y ; j+=offset)
		write32(this->address + j, c);

	// Right line
	// line_cache is still the same
	for (unsigned int j=line_cache+final_x-1 ; j<offset*final_y ; j+=offset)
		write32(this->address + j, c);

	// Bottom line
	line_cache = (final_y-1)*offset;
	for (unsigned int i=line_cache+x ; i<line_cache+final_x ; i++)
		write32(this->address + i, c);

	writeMemoryBarrier();
}

void Framebuffer_fillRectangle(Framebuffer* this, unsigned int x, unsigned int y, unsigned int width, unsigned int height, color_t c){
	assert(this);
	const unsigned int final_x = x+width;
	const unsigned int final_y = y+height;
	assert(final_x < this->width);
	assert(final_y < this->height);

	const int offset = getLineOffset();
	int line_cache;

	for (unsigned int j=y ; j<final_y ; j++) {
		line_cache = offset*j;
		for (unsigned int i=x ; i<final_x ; i++) {
			write32(this->address + line_cache + i, c);
		}
	}
}

bool Framebuffer_init(Framebuffer* this){
	// Sanitize check the needed variables were initialized correctly beforehand
	if (this->address == NULL || this->width==0 || this->height==0 || this->pitch==0 || this->bpp==0)
		return false;

	if (this->bpp != 32){
		log(ERROR, MODULE, "A value of %d for bpp (Bits per pixel) is unsupported (only 32 is supported)", this->bpp);
		return false;
	}

	this->drawOffsetX = 4;
	this->drawOffsetY = 4;
	Framebuffer_setClearColor(this, COLOR_32BPP(31, 31, 31));
	Framebuffer_setFontColor(this, COLOR_WHITE);
	Framebuffer_setZoom(this, 1); // setZoom recomputes some internal variables, so we don't need to set those

	Framebuffer_clearTerminal(this);
	Framebuffer_clearScreen(this);

	this->cursorX = 0;
	this->cursorY = 0;

	return true;
}
