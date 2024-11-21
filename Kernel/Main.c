#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "stdio.h"
#include "string.h"
#include "Arch/HAL.h"
#include "Drivers/PS2.h"

extern uint8_t __bss_start;
extern uint8_t __end;

// Entry point for the second-stage bootloader C code.
// It is called by the Main.asm code (which is the entry point of the 2nd stage bootloader)
void __attribute__((section(".entry"))) start(){
	// Clear uninitalized data
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	clear_screen();
	puts("Supposons que je sois dans votre kernel !");

	HAL_initialize();
	PS2_initalize(); // Initialize PS/2 driver

	while(true); // infinite active wait
}
