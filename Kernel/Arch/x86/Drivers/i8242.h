#ifndef __I8242_H__
#define __I8242_H__

#include <stdint.h>
#include <stdbool.h>

// Intel 8242 PS/2 controller driver

// Initalize the i8242 PS/2 controller
// (Note: if using USB, initalize and disable USB Legacy support first)
void i8242_initalize();

void i8242_getStatus(bool* isEnabled_out, bool* port1Available_out, bool* port2Available_out);
void i8242_enableDevicesInterrupts();
void i8242_disableDevicesInterrupts();
bool i8242_sendByteToDevice1(uint8_t byte);
bool i8242_sendByteToDevice2(uint8_t byte);
bool i8242_receiveDeviceByte(uint8_t* byte_out);

#endif
