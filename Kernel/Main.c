#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "stdio.h"
#include "string.h"
#include "Arch/HAL.h"
#include "Drivers/Graphics.h"
#include "Drivers/PS2.h"
#include "Drivers/Keyboard.h"

extern uint8_t __bss_start;
extern uint8_t __end;

// Kernel entry point
// Note: It is marked as ".entry" so that we can place it at the top of
// the binary when linking (see the ld map Linker.map)
void __attribute__((section(".entry"))) kmain(){
	// Clear uninitalized data
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	clear_screen();
	puts("Supposons que je sois dans votre kernel !");

	Graphics_initialize();
	HAL_initialize();
	PS2_initalize(); // Initialize PS/2 driver
	Keyboard_initalize(); // Initialize keyboard driver/subsystem

	while(true); // infinite active wait
}
