#ifndef __VGA_H__
#define __VGA_H__

#include <stdbool.h>

bool VGA_initialize();
void VGA_putchar(char c);
void VGA_clearScreen();

#endif
