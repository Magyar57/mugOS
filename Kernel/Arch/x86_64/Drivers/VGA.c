#include <stdint.h>
#include "io.h"
#include "assert.h"

#include "VGA.h"

// VGA Driver
// For now, only text mode is supported
// Note: We use the BIOS to detect the video mode. With UEFI, we'll have to change that

#define TAB_SIZE 4

// Address 0x410 => Bios Data Area: packed bit flags for detected hardware
#define BDA_HARDWARE 0x410

enum VGATextMode{
	VGA_TEXTMODE_NONE,
	VGA_TEXTMODE_COLOUR,
	VGA_TEXTMODE_MONOCHROME
};

#define VGA_COLOUR_VIDEO_MEMORY		(uint8_t*) 0xb8000
#define VGA_MONOCHROME_VIDEO_MEMORY	(uint8_t*) 0xb0000
static uint8_t* m_videoMemory; // Either COLOUR or MONOCHROME_VIDEO_MEMORY

#define SCREEN_WIDTH  80 // Default text mode screen width
#define SCREEN_HEIGHT 25 // Default text mode screen height
#define DEFAULT_COLOR (0 << 4) | (7) // (background_color << 4) | (character_color)
static int m_screenX = 0; // Cursor's X position
static int m_screenY = 0; // Cursor's Y position

#define VGA_PORT_CRTC_ADDRESS_REGISTER	0x3d4
#define VGA_PORT_CRTC_DATA_REGISTER		0x3d5
#define VGA_CRTC_ADDR_CURSOR_HIGH		0x0e
#define VGA_CRTC_ADDR_CURSOR_LOW		0x0f

static enum VGATextMode detectVideoTextMode(){
	const uint16_t* bdaHardwarePtr = (const uint16_t*) BDA_HARDWARE;
	const uint16_t bdaHardware = *bdaHardwarePtr;

	uint16_t masked = bdaHardware & 0x30;
	if (masked == 0x20) return VGA_TEXTMODE_COLOUR;
	if (masked == 0x30) return VGA_TEXTMODE_MONOCHROME;

	// masked == 0x00
	return VGA_TEXTMODE_NONE;
}

static void setCursorPosition(int x, int y){
	int new_pos = y * SCREEN_WIDTH + x;

	// Lower byte
	outb(VGA_PORT_CRTC_ADDRESS_REGISTER, VGA_CRTC_ADDR_CURSOR_LOW);
	outb(VGA_PORT_CRTC_DATA_REGISTER, (uint8_t) 0x000000ff & new_pos);
	// Upper byte
	outb(VGA_PORT_CRTC_ADDRESS_REGISTER, VGA_CRTC_ADDR_CURSOR_HIGH);
	outb(VGA_PORT_CRTC_DATA_REGISTER, (uint8_t) ((0x0000ff00 & new_pos) >> 8));
}

static inline void putchar_xy(int x, int y, char c){
	assert(x<SCREEN_WIDTH && y<SCREEN_HEIGHT);

	m_videoMemory[2*(y*SCREEN_WIDTH + x)] = c;
}

static inline void putColor_xy(int x, int y, uint8_t color){
	assert(x<SCREEN_WIDTH && y<SCREEN_HEIGHT);

	m_videoMemory[2*(y*SCREEN_WIDTH + x) + 1] = color;
}

static inline char getc_xy(int x, int y){
	assert(x<SCREEN_WIDTH && y<SCREEN_HEIGHT);

	return m_videoMemory[2*(y*SCREEN_WIDTH + x)];
}

static inline char getColor_xy(int x, int y){
	assert(x<SCREEN_WIDTH && y<SCREEN_HEIGHT);

	return m_videoMemory[2*(y*SCREEN_WIDTH + x) + 1];
}

static void scrollDown(int n){
	for(int y=n ; y<SCREEN_HEIGHT ; y++){
		for(int x=0 ; x<SCREEN_WIDTH ; x++){
			char c = getc_xy(x, y);
			char color = getColor_xy(x, y);
			putchar_xy(x, y-n, c);
			putColor_xy(x, y-n, color);
		}
	}

	for(int y=SCREEN_HEIGHT-n ; y<SCREEN_HEIGHT ; y++){
		for (int x=0; x<SCREEN_WIDTH ; x++){
			putchar_xy(x, y, '\0');
			putColor_xy(x, y, DEFAULT_COLOR);
		}
	}

	m_screenY -= n;
	setCursorPosition(m_screenX, m_screenY);
}

bool VGA_initialize(){
	// Test for color or not
	enum VGATextMode videoMode = detectVideoTextMode();
	if (videoMode == VGA_TEXTMODE_NONE)
		return false;

	m_videoMemory = (videoMode == VGA_TEXTMODE_COLOUR) ?
		VGA_COLOUR_VIDEO_MEMORY : VGA_MONOCHROME_VIDEO_MEMORY;

	// Enable cursor
	m_screenX = 0;
	m_screenY = 0;
	setCursorPosition(m_screenX, m_screenY);

	return true;
}

// Note: ortograph is to conform to the C stdlib
void VGA_putchar(char c){
	if (c == '\n'){
		m_screenX = 0;
		m_screenY++;
		goto end_putc;
	}
	if (c == '\r'){
		m_screenX = 0;
		goto end_putc;
	}
	if (c == '\t'){
		do {
			putchar_xy(m_screenX, m_screenY, ' ');
			m_screenX++;
		} while(m_screenX % TAB_SIZE != 0 && m_screenX<SCREEN_WIDTH);
		goto end_putc;
	}

	putchar_xy(m_screenX, m_screenY, c);

	m_screenX = (m_screenX+1) % SCREEN_WIDTH;
	if (m_screenX == 0) m_screenY++;

	end_putc:
	if (m_screenY >= SCREEN_HEIGHT) scrollDown(1);
	setCursorPosition(m_screenX, m_screenY);
}

void VGA_clearScreen(){
	for(int y=0 ; y<SCREEN_HEIGHT ; y++){
		for(int x=0 ; x<SCREEN_WIDTH ; x++){
			putchar_xy(x, y, ' ');
			putColor_xy(x, y, DEFAULT_COLOR);
		}
	}

	m_screenX = 0;
	m_screenY = 0;
	setCursorPosition(m_screenX, m_screenY);
}
