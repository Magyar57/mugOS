#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <stdint.h>
#include <stdbool.h>

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

typedef struct s_Framebuffer {
	// Framebuffer properties
	uint32_t* address;			// The memory address of the framebuffer (pixel array)
	uint64_t width;				// Width in pixel
	uint64_t height;			// Height in pixel
	uint64_t pitch;				// Number of bytes that makes a line
	uint16_t bpp;				// Bits per pixel

	// Drawing stuff
	gop_color_t clearColor;		// Framebufer's background/clear color
	uint32_t drawOffsetX;		// Character X offset to the border of the screen
	uint32_t drawOffsetY;		// Character Y offset to the border of the screen
	uint32_t zoom;				// Font zoom level (multiplier)
	uint32_t charWidth;			// The width of a character on screen, in pixels (accounts for zoom)
	uint32_t charHeight;		// The height of a character on screen, in pixels (accounts for zoom)

	// Character array (framebuffer as a terminal)
	uint32_t cursorX;			// Cursor X position on screen
	uint32_t cursorY;			// Cursor Y position on screen
	uint32_t textWidth;			// Width (number of characters in 'text')
	uint32_t textHeight;		// Height (number of characters in 'text')
	uint32_t textIndex;
	unsigned char text[TERMINAL_SIZE]; // Contains the printed letters
} Framebuffer;

void Framebuffer_clearTerminal(Framebuffer* this);
void Framebuffer_setClearColor(Framebuffer* this, uint32_t clearColor);
void Framebuffer_setZoom(Framebuffer* this, uint32_t zoom);
void Framebuffer_clearScreen(Framebuffer* this);
void Framebuffer_drawLetter(Framebuffer* this, unsigned char letter, uint32_t fontColor, int offsetX, int offsetY);
void Framebuffer_scrollDown(Framebuffer* this);
void Framebuffer_putchar(Framebuffer* this, const char c);
void Framebuffer_puts_noLF(Framebuffer* this, const char* str);
void Framebuffer_puts(Framebuffer* this, const char* str);

/// @brief Initialize the Framebuffer driver (from a GOP or Bios obtained memory-mapped framebuffer)
/// @param this Framebuffer object to initialize. Note: it HAS to be pre-initialized with
///        fields `framebuffer`, `width`, `height`, `pitch` and `bpp` set
/// @returns A boolean indicating initialization success
/// @note This method and its object are meant to be used and abstracted by the Graphics subsystem
bool Framebuffer_initialize(Framebuffer* this);

#endif
