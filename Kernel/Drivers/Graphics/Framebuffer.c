#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "assert.h"
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
	this->textWidth = (maxHorizontalChar > MAX_TERMINAL_WIDTH) ? MAX_TERMINAL_WIDTH : maxHorizontalChar;
	this->textHeight = (maxVerticalChar > MAX_TERMINAL_HEIGHT) ? MAX_TERMINAL_HEIGHT : maxVerticalChar;
}

void Framebuffer_setClearColor(Framebuffer* this, color_t clearColor){
	assert(this);
	this->clearColor = clearColor;
}

void Framebuffer_clearScreen(Framebuffer* this){
	assert(this);
	const int one_line_offset = getLineOffset();

	for(uint64_t j=0 ; j<this->height ; j++){
		size_t lineOffset = j*one_line_offset;
		for (uint64_t i=0 ; i<this->width ; i++){
			this->address[lineOffset + i] = this->clearColor;
		}
	}
}

void Framebuffer_drawLetter(Framebuffer* this, unsigned char letter, uint32_t fontColor, unsigned int offsetX, unsigned int offsetY){
	assert(this);
	assert((offsetX+BITMAP_CHAR_WIDTH) < this->width);
	assert((offsetY+BITMAP_CHAR_HEIGHT) < this->height);

	uint8_t* bitmap = m_defaultFont[letter];
	int mask; // bit mask, will be shifted right every iteration
	color_t colorToDraw;

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
					this->address[iterationLineOffset + horizontalOffset+ii] = colorToDraw;
				}
			}

			mask >>= 1; // next bitmap pixel
		}
	}
}

// Draws a '\t' character to the screen. IRQ UNSAFE !
static inline void drawTab(Framebuffer* this){
	do {
		Framebuffer_drawLetter(this, ' ', WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);
		this->cursorX = (this->cursorX+1) % this->textWidth;
		if (this->cursorX == 0) {
			this->cursorY++;
			break;
		}
	} while (this->cursorX % TAB_SIZE != 0);
}

// (Re)draws the entire screen. IRQ UNSAFE !
static inline void drawScreen(Framebuffer* this){
	assert(this);

	this->cursorX = 0;
	this->cursorY = 0;
	for (int i=0 ; i<TERMINAL_SIZE ; i++){
		char c = this->text[i];

		switch (c){
			case '\0':
				break;
			case '\n':
				this->cursorX = 0;
				this->cursorY++;
				break;
			case '\r':
				this->cursorX = 0;
				break;;
			case '\t':
				drawTab(this);
				break;
			default:
				Framebuffer_drawLetter(this, c, WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);
				this->cursorX = (this->cursorX+1) % this->textWidth;
				if (this->cursorX == 0) this->cursorY++;
				break;
		}

	}
}

// Get the size of the terminal's first line, in character number
static inline size_t getFirstTextLineSize(Framebuffer* this){
	assert(this);

	// To get the terminal line size, we count the number of character up to
	// the next '\n', or we hit the end of the screen ("forced" line feed)
	// For this "forced" case, we need to track the number of printer characters on screen too !
	// Hence screen_size

	size_t line_size = 0;
	size_t screen_size = 0;
	char c = this->text[0];
	while( (c != '\n') && (screen_size < this->textWidth) ){
		if (c == '\t'){
			screen_size += (TAB_SIZE - (screen_size % TAB_SIZE));
		}
		else {
			screen_size++;
		}

		line_size++;
		c = this->text[line_size];
	}

	return line_size;
}

void Framebuffer_scrollDown(Framebuffer* this){
	assert(this);
	unsigned long flags;
	IRQ_disableSave(flags);

	// First line size
	size_t line_size = getFirstTextLineSize(this);
	if (line_size != this->textWidth) line_size++; // include the '\n'

	// Move the terminal text one line up
	memmove(this->text, this->text+line_size, TERMINAL_SIZE-line_size);
	this->textIndex -= line_size;

	// Redraw everything (cursor is reset by drawScreen)
	Framebuffer_clearScreen(this);
	drawScreen(this);

	IRQ_restore(flags);
}

void Framebuffer_putchar(Framebuffer* this, const char c){
	assert(this);
	unsigned long flags;

	switch (c){
		case '\0':
			return;

		case '\t':
			IRQ_disableSave(flags);
			this->text[this->textIndex] = '\t';
			this->textIndex++;
			drawTab(this);
			IRQ_restore(flags);
			break;

		case '\n':
			IRQ_disableSave(flags);
			this->text[this->textIndex] = c;
			this->textIndex++;
			this->cursorX = 0;
			this->cursorY++;
			IRQ_restore(flags);
			break;

		case '\r':
			IRQ_disableSave(flags);
			this->text[this->textIndex] = c;
			this->textIndex++;
			this->cursorX = 0;
			IRQ_restore(flags);
			break;

		default:
			IRQ_disableSave(flags);
			this->text[this->textIndex] = c;
			this->textIndex++;
			Framebuffer_drawLetter(this, c, WHITE, this->cursorX*BITMAP_CHAR_WIDTH + this->drawOffsetX, this->cursorY*BITMAP_CHAR_HEIGHT + this->drawOffsetY);
			this->cursorX = (this->cursorX+1) % this->textWidth;
			if (this->cursorX == 0) this->cursorY++;
			IRQ_restore(flags);
			break;
	}

	// Note: scrollDown is interrupt-safe
	if (this->cursorY >= this->textHeight)
		Framebuffer_scrollDown(this);
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
	assert( (x>0) && (x < this->width) );
	assert( (y>0) && (y < this->height) );

	const int offset = getLineOffset();
	this->address[offset*y + x] = pixel;
}

void Framebuffer_drawRectangle(Framebuffer* this, unsigned int x, unsigned int y, unsigned int sizeX, unsigned int sizeY, color_t c){
	assert(this);
	const unsigned int final_x = x+sizeX;
	const unsigned int final_y = y+sizeY;
	// Note: we tolerate the equality because the for loops draw STRICTLY under the final_x/y values
	assert(final_x <= this->width);
	assert(final_y <= this->height);

	const int offset = getLineOffset();
	int line_cache;

	for (int j=y ; j<final_y ; j++) {
		line_cache = offset*j;
		for (int i=x ; i<final_x ; i++) {
			this->address[line_cache + i] = c;
		}
	}
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
