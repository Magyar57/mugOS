#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "stdio.h"
#include "string.h"

extern uint8_t __bss_start;
extern uint8_t __end;

void end(){
	// Just an infinite loop
	while(true){}
}

// Entry point for the second-stage bootloader C code.
// It is called by the Main.asm code (which is the entry point of the 2nd stage bootloader)
void __attribute__((section(".entry"))) start(){
	// Clear uninitalized data
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	clear_screen();
	puts("Supposons que je sois dans votre kernel !");

	end();
}
