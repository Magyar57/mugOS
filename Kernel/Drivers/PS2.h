#ifndef __PS2_H__
#define __PS2_H__

#include <stdint.h>

// Initalize the PS/2 device driver
void PS2_initalize();

// Notify the PS/2 driver that the keyboard needs attention
void PS2_notifyKeyboard();

// Notify the PS/2 driver that the mouse needs attention
void PS2_notifyMouse();

// Makes the driver handle a PS/2 mouse packet
void PS2_handleMousePacket(uint8_t packet[4]);

#endif
