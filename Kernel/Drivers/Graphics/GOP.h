#ifndef __GOP_H__
#define __GOP_H__

#include <stdint.h>

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
	void* gop; // EFI_GRAPHICS_OUTPUT_PROTOCOL*
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

void GOP_clearTerminal(GOPDriver* this);
void GOP_setClearColor(GOPDriver* this, uint32_t clearColor);
void GOP_clearScreen(GOPDriver* this);
void GOP_drawLetter(GOPDriver* this, unsigned char letter, uint32_t fontColor, int offsetX, int offsetY);
void GOP_drawScreen(GOPDriver* this);
void GOP_scrollDown(GOPDriver* this, unsigned int n);
void GOP_putchar(GOPDriver* this, const char c);
void GOP_puts_noLF(GOPDriver* this, const char* str);
void GOP_puts(GOPDriver* this, const char* str);

/// @brief Initialize the GOP driver
/// @param this GOP driver to initialize
/// @param gop a UEFI 'EFI_GRAPHICS_OUTPUT_PROTOCOL*' from an UEFI firmware
void GOP_initialize(GOPDriver* this, void* gop);

#endif
