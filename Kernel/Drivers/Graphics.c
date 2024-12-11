#include <stdbool.h>
#include "Panic.h"
#include "Drivers/VGA.h"

#include "Graphics.h"

void Graphics_initialize(){
	bool vga_ok = VGA_initialize();
	if (!vga_ok){
		PANIC();
	}
}

void Graphics_clearScreen(){
	VGA_clearScreen();
}
