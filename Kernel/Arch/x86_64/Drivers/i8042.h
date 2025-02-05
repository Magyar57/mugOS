#ifndef __i8042_H__
#define __i8042_H__

#include <stdint.h>
#include <stdbool.h>

// Intel 8042 PS/2 controller driver

// Initialize the i8042 PS/2 controller
// (Note: if using USB, initialize and disable USB Legacy support first)
void i8042_initialize();

void i8042_getStatus(bool* isEnabled_out, bool* port1Available_out, bool* port2Available_out);
void i8042_enableDevicesInterrupts();
void i8042_disableDevicesInterrupts();
bool i8042_sendByteToDevice1(uint8_t byte);
bool i8042_sendByteToDevice2(uint8_t byte);
bool i8042_receiveDeviceByte(uint8_t* byte_out);

#endif
