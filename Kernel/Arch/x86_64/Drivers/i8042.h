#ifndef __i8042_H__
#define __i8042_H__

#include <stdint.h>

// i8042.h: Intel 8042 PS/2 controller driver

// Initialize the i8042 PS/2 controller
void i8042_init();

void i8042_getStatus(bool* isEnabled_out, bool* port1Available_out, bool* port2Available_out, bool* translationOut);
void i8042_setDevicesIRQ(bool device1, bool device2);
bool i8042_sendByteToDevice(int device, uint8_t byte);
bool i8042_receiveByte(uint8_t* byte_out);

#endif
