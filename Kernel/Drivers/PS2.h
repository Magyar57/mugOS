#ifndef __PS2_H__
#define __PS2_H__

#include <stdint.h>

// Initialize the PS/2 device driver
void PS2_initialize();

// Notify the PS/2 driver that the keyboard needs attention
void PS2_notifyKeyboard();

// Makes the driver handle a PS/2 mouse packet
void PS2_handleMousePacket(uint8_t flags, uint8_t dx, uint8_t dy, uint8_t wheelAndThumbBtn);

// Notify the PS/2 driver that the mouse needs attention
void PS2_notifyMouse();

#endif
