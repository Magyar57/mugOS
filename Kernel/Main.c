#include "stdio.h"
#include "string.h"
#include "LimineRequests.h"
#include "CPU.h"
#include "Arch/HAL.h"
#include "Drivers/Graphics/Graphics.h"
#include "Drivers/Serial.h"
#include "Drivers/PS2.h"
#include "Drivers/Keyboard.h"

extern uint8_t __bss_start;
extern uint8_t __end;

// Kernel entry point
void kmain(){
	// Clear uninitialized data
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	Graphics_initialize(GRAPHICS_LIMINE_FRAMEBUFFER, (void*) framebufferReq.response->framebuffers[0]);
	Graphics_clearScreen();
	puts("Supposons que je sois dans votre kernel !");

	HAL_initialize();
	Serial_initialize();
	PS2_initialize(); // Initialize PS/2 driver
	Keyboard_initialize(); // Initialize keyboard driver/subsystem

	// Infinite loop: whenever an interrupts fire, handle it ; then stop again.
	while(true) halt();
}
