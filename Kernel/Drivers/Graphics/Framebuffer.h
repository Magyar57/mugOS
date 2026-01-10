#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <stdint.h>

typedef uint32_t color_t;

/// @brief Convert three 8 bits values to a color_t
/// @warning Bounts are not checked ! Values must be in [0, 255]
#define COLOR_32BPP(r, g, b)	(r<<16 | g<<8 | b)

#define COLOR_BLACK				COLOR_32BPP(  0,   0,   0)
#define COLOR_DARK_GREY			COLOR_32BPP( 24,  24,  24)
#define COLOR_GREY				COLOR_32BPP(128, 128, 128)
#define COLOR_LIGHT_GREY		COLOR_32BPP(192, 192, 192)
#define COLOR_WHITE				COLOR_32BPP(255, 255, 255)
#define COLOR_GREEN				COLOR_32BPP(  0, 255,   0)
#define COLOR_RED				COLOR_32BPP(255,   0,   0)
#define COLOR_BLUE				COLOR_32BPP(  0,   0, 255)
#define COLOR_YELLOW			COLOR_32BPP(255, 255,   0)
#define COLOR_CYAN				COLOR_32BPP(  0, 255, 255)
#define COLOR_MAGENTA			COLOR_32BPP(255,   0, 255)

#define MAX_TERMINAL_WIDTH		256	// Aka in number of characters in a line
#define MAX_TERMINAL_HEIGHT		512	// Aka number of lines
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
	color_t clearColor;			// Framebuffer's background/clear color
	color_t fontColor;			// Framebuffer's font (write) color
	uint32_t drawOffsetX;		// Character X offset to the border of the screen
	uint32_t drawOffsetY;		// Character Y offset to the border of the screen
	uint32_t zoom;				// Font zoom level (multiplier)
	// Cached width/height of a character on screen, in pixels, accounting for zoom
	uint32_t charWidth, charHeight;

	// Character array (framebuffer as a terminal)
	uint32_t cursorX;			// Cursor X position on screen
	uint32_t cursorY;			// Cursor Y position on screen
	uint32_t textWidth;			// Terminal width (number of characters in a line)
	uint32_t textHeight;		// Terminal height (number of lines)
	unsigned char text[TERMINAL_SIZE]; // Contains the printed letters
} Framebuffer;

void Framebuffer_clearTerminal(Framebuffer* this);
void Framebuffer_setClearColor(Framebuffer* this, color_t color);
void Framebuffer_setFontColor(Framebuffer* this, color_t color);
void Framebuffer_setZoom(Framebuffer* this, uint32_t zoom);
void Framebuffer_clearScreen(Framebuffer* this);
void Framebuffer_drawChar(Framebuffer* this, char c, uint32_t color, int x, int y);
void Framebuffer_scrollDown(Framebuffer* this);
void Framebuffer_putchar(Framebuffer* this, const char c);
void Framebuffer_puts_noLF(Framebuffer* this, const char* str);
void Framebuffer_puts(Framebuffer* this, const char* str);

/// @brief Put a pixel in the framebuffer at the given coordinates
/// @note This method should be avoided at all cost if drawing lots of pixels in a row !!!
/// It is very costly, as it will recompute a lof of checks and flags before putting the pixel
void Framebuffer_putPixel(Framebuffer* this, int x, int y, color_t pixel);

/// @brief Draw a rectangle if size `(sizeX, sizeY)` starting at offset `(x, y)`
void Framebuffer_drawRectangle(Framebuffer* this, int x, int y, int width, int height, color_t c);

/// @brief Draw a filled rectangle if size `(sizeX, sizeY)` starting at offset `(x, y)`
void Framebuffer_fillRectangle(Framebuffer* this, int x, int y, int width, int height, color_t c);

/// @brief Initialize the Framebuffer driver (from a GOP or Bios obtained memory-mapped framebuffer)
/// @param this Framebuffer object to initialize. Note: it HAS to be pre-initialized with
///        fields `framebuffer`, `width`, `height`, `pitch` and `bpp` set
/// @return A boolean indicating initialization success
/// @note This method and its object are meant to be used and abstracted by the Graphics subsystem
bool Framebuffer_init(Framebuffer* this);

#endif
