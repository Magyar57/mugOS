#include <stdbool.h>
#include <stddef.h>
#include "Panic.h"
#include "Drivers/VGA.h"
#include "Drivers/Graphics/GOP.h"

#include "Graphics.h"

bool m_initialized = false;
GOPDriver m_gopDriver;
void* m_driver = NULL; // pointer to m_gopDriver for GOP, NULL for VGA

void Graphics_initialize(void* gop){
	// VGA fallback
	if (gop == NULL){
		bool vga_ok = VGA_initialize();
		if (!vga_ok){
			panic();
		}
		m_driver = NULL;
	}

	// GOP default
	GOP_initialize(&m_gopDriver, gop);
	m_driver = &m_gopDriver;
}

void Graphics_clearScreen(){
	if (!m_initialized) return;

	if (m_driver)	GOP_clearScreen(m_driver);
	else			VGA_clearScreen();
}

void Graphics_putchar(char c){
	if (!m_initialized) return;

	if (m_driver)	GOP_putchar(m_driver, c);
	else			VGA_putchar(c);
}
