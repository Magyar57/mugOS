#include "stdio.h"
#include "string.h"
#include "CPU.h"
#include "EFI/Protocols/GraphicsOutputProtocol.h" // TODO replace with a boot protocol
#include "Arch/HAL.h"
#include "Drivers/Graphics/Graphics.h"
#include "Drivers/Serial.h"
#include "Drivers/PS2.h"
#include "Drivers/Keyboard.h"

extern uint8_t __bss_start;
extern uint8_t __end;

// Kernel entry point
// Note: It is marked as ".entry" so that we can place it at the top of
// the binary when linking (see the ld map Linker.map)
__attribute__((sysv_abi)) __attribute__((section(".entry")))
void kmain(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop){
	// Clear uninitialized data
	// memset(&__bss_start, 0, (&__end) - (&__bss_start)); // TODO put back when we have working relocations

	Graphics_initialize(gop);
	Graphics_clearScreen();
	puts("Supposons que je sois dans votre kernel !");
	// puts("TODO remove hardcoded fd=1 in fileno (stdio.c) when we have working relocations");

	// HAL_initialize();
	// Serial_initialize();
	// PS2_initialize(); // Initialize PS/2 driver
	// Keyboard_initialize(); // Initialize keyboard driver/subsystem

	// Infinite loop: whenever an interrupts fire, handle it ; then stop again.
	while(true) halt();
}
