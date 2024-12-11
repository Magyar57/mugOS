#include <stdint.h>
#include <stdbool.h>
#include "io.h"

#include "VGA.h"

// VGA Driver
// For now, only text mode is supported
// Note: We use the BIOS to detect the video mode. With UEFI, we'll have to change that

// Address 0x410 => Bios Data Area: packed bit flags for detected hardware
#define BDA_HARDWARE 0x410

typedef enum {
	VGA_TEXTMODE_NONE,
	VGA_TEXTMODE_COLOUR,
	VGA_TEXTMODE_MONOCHROME
} VGATextMode;

#define VGA_COLOUR_VIDEO_MEMORY		(uint8_t*) 0xb8000
#define VGA_MONOCHROME_VIDEO_MEMORY	(uint8_t*) 0xb0000
uint8_t* m_videoMemory; // Either COLOUR or MONOCHROME_VIDEO_MEMORY

#define SCREEN_WIDTH  80 // Default text mode screen width
#define SCREEN_HEIGHT 25 // Default text mode screen height
#define DEFAULT_COLOR (0 << 4) | (7) // (background_color << 4) | (character_color)
int m_screenX = 0; // Cursor's X position
int m_screenY = 0; // Cursor's Y position

#define VGA_PORT_CRTC_ADDRESS_REGISTER	0x3d4
#define VGA_PORT_CRTC_DATA_REGISTER		0x3d5
#define VGA_CRTC_ADDR_CURSOR_HIGH		0x0e
#define VGA_CRTC_ADDR_CURSOR_LOW		0x0f

static VGATextMode detectVideoTextMode(){
	const uint16_t* bdaHardwarePtr = (const uint16_t*) BDA_HARDWARE;
	const uint16_t bdaHardware = *bdaHardwarePtr;

	uint16_t masked = bdaHardware & 0x30;
	if (masked == 0x20) return VGA_TEXTMODE_COLOUR;
	if (masked == 0x30) return VGA_TEXTMODE_MONOCHROME;

	// masked == 0x00
	return VGA_TEXTMODE_NONE;
}

#include "stdio.h"

static void setCursorPosition(int x, int y){
	int new_pos = y * SCREEN_WIDTH + x;

	// Lower byte
	outb(VGA_PORT_CRTC_ADDRESS_REGISTER, VGA_CRTC_ADDR_CURSOR_LOW);
	outb(VGA_PORT_CRTC_DATA_REGISTER, (uint8_t) 0x000000ff & new_pos);
	// Upper byte
	outb(VGA_PORT_CRTC_ADDRESS_REGISTER, VGA_CRTC_ADDR_CURSOR_HIGH);
	outb(VGA_PORT_CRTC_DATA_REGISTER, (uint8_t) ((0x0000ff00 & new_pos) >> 8));
}

bool VGA_initialize(){
	// Test for color or not
	VGATextMode videoMode = detectVideoTextMode();
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
