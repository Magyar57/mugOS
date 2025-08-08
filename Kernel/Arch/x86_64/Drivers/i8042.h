#ifndef __i8042_H__
#define __i8042_H__

#include <stdint.h>

// i8042.h: Intel 8042 PS/2 controller driver

// Initialize the i8042 PS/2 controller
void i8042_init();

void i8042_getStatus(bool* enabled, bool* port1Valid, bool* port2Valid, bool* translation);
void i8042_enableDevicesIRQs();
bool i8042_sendByteToDevice(int device, uint8_t byte);
bool i8042_receiveByte(uint8_t* byte_out);

#endif
