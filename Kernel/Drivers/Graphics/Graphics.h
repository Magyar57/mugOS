#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

enum GraphicsSource {
	GRAPHICS_NONE,					// No graphics are present
	GRAPHICS_BIOS_VGA,				// Legacy BIOS VGA structure (unsupported)
	GRAPHICS_UEFI_GOP,				// Raw UEFI GOP framebuffer (deprecated)
	GRAPHICS_LIMINE_FRAMEBUFFER,	// Limine framebuffer structure
} ;

/// @brief Initialize the Graphics subsystem
/// @param graphics The type of graphic structure to initialize
/// @param pointer The pointer to the graphics data structure (eg: limine framebuffer,
/// 				UEFI EFI_GRAPHICS_OUTPUT_PROTOCOL*). It musts correspond to the graphics parameter
void Graphics_init(enum GraphicsSource graphics, void* pointer);

void Graphics_clearScreen();
void Graphics_putchar(char c);
void Graphics_puts(const char* str);
void Graphics_puts_noLF(const char* str);
void Graphics_scrollDown();

#endif
