#ifndef __I8242_H__
#define __I8242_H__

#include <stdint.h>
#include <stdbool.h>

// Intel 8242 PS/2 controller driver

#define PS2Controller_initialize() i8242_initalize()

// Initalize the i8242 PS/2 controller
// (Note: if using USB, initalize and disable USB Legacy support first)
void i8242_initalize();

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
