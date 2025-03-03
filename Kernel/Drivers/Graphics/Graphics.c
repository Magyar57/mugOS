#include <stdbool.h>
#include <stddef.h>
#include "Logging.h"
#include "Panic.h"
#include "limine.h"
#include "Drivers/Graphics/Framebuffer.h"

#include "Graphics.h"

#define MODULE "Graphics"

bool m_initialized = false;
void* m_driver; // current driver
enum GraphicsSource m_driverType; // Which drive we're using

// Supported drivers
Framebuffer m_framebuffer;

void Graphics_initialize(enum GraphicsSource graphics, void* pointer){
	m_driverType = graphics;

	switch (graphics){
	case GRAPHICS_NONE:
		m_driver = NULL;
		m_initialized = false; // request for no graphics, we just un-initialize
		break;
	case GRAPHICS_LIMINE_FRAMEBUFFER:
		m_driver = &m_framebuffer;
		struct limine_framebuffer* fb = (struct limine_framebuffer*) pointer;
		m_framebuffer.address = fb->address;
		m_framebuffer.width = fb->width;
		m_framebuffer.height = fb->height;
		m_framebuffer.pitch = fb->pitch;
		m_framebuffer.bpp = fb->bpp;
		m_initialized = Framebuffer_initialize(&m_framebuffer);
		if (!m_initialized)
			log(WARNING, MODULE, "Failed to initialize, framebuffer error");
		break;
	// All of those are unsupported/deprecated
	case GRAPHICS_BIOS_VGA:
	case GRAPHICS_UEFI_GOP:
	default:
		m_driver = NULL;
		m_initialized = false;
		log(PANIC, MODULE, "Requested video mode is unsupported or deprecated");
		panic();
	}
}

void Graphics_clearScreen(){
	if (!m_initialized) return;

	switch (m_driverType){
		case GRAPHICS_NONE:
			break;
		case GRAPHICS_BIOS_VGA:
			break;
		case GRAPHICS_UEFI_GOP:
			break;
		case GRAPHICS_LIMINE_FRAMEBUFFER:
			Framebuffer_clearScreen(m_driver);
			break;
	default:
		break;
	}

	Framebuffer_clearScreen(&m_framebuffer);
}

void Graphics_putchar(char c){
	if (!m_initialized) return;

	switch (m_driverType){
		case GRAPHICS_NONE:
			break;
		case GRAPHICS_BIOS_VGA:
			break;
		case GRAPHICS_UEFI_GOP:
			break;
		case GRAPHICS_LIMINE_FRAMEBUFFER:
			Framebuffer_putchar(m_driver, c);
			break;
	default:
		break;
	}
}

void Graphics_puts(const char* str){
	if (!m_initialized) return;

	switch (m_driverType){
		case GRAPHICS_NONE:
			break;
		case GRAPHICS_BIOS_VGA:
			break;
		case GRAPHICS_UEFI_GOP:
			break;
		case GRAPHICS_LIMINE_FRAMEBUFFER:
			Framebuffer_puts(m_driver, str);
			break;
	default:
		break;
	}
}

void Graphics_puts_noLF(const char* str){
	if (!m_initialized) return;

	switch (m_driverType){
		case GRAPHICS_NONE:
			break;
		case GRAPHICS_BIOS_VGA:
			break;
		case GRAPHICS_UEFI_GOP:
			break;
		case GRAPHICS_LIMINE_FRAMEBUFFER:
			Framebuffer_puts_noLF(m_driver, str);
			break;
	default:
		break;
	}
}
