#ifndef __I8242_H__
#define __I8242_H__

#include <stdint.h>
#include <stdbool.h>

// Intel 8242 PS/2 controller driver

#define PS2Controller_initialize() i8242_initalize()

// Initalize the i8242 PS/2 controller
// (Note: if using USB, initalize and disable USB Legacy support first)
void i8242_initalize();

// Returns weather the controller/driver is initalized and enabled, and which ports are enabled
void PS2Controller_getStatus(bool* isEnabled_out, bool* port1Available_out, bool* port2Available_out);

// Enable the interrupts sent from the devices
void PS2Controller_enableDevicesInterrupts();

// Disable the interrupts sent from the devices
void PS2Controller_disableDevicesInterrupts();

// Send a byte to the PS/2 device 1
// Returns: true on success, false on failure
bool PS2Controller_sendByteToDevice1(uint8_t byte);

// Send a byte to the PS/2 device 2
// Returns: true on success, false on failure
bool PS2Controller_sendByteToDevice2(uint8_t byte);

// Receive a byte from a previously sent command
// Returns: true on success, false on failure (no readable byte in output buffer at the end of timeout)
// If success, write output to *byte_out
bool PS2Controller_receiveDeviceByte(uint8_t* byte_out);

#endif
