#ifndef __PS2_CONTROLLER_H__
#define __PS2_CONTROLLER_H__

#include "Drivers/i8242.h"

#define PS2Controller_initialize i8242_initalize

// Returns weather the controller/driver is initalized and enabled, and which ports are enabled
#define PS2Controller_getStatus i8242_getStatus

// Enable the interrupts sent from the devices
#define PS2Controller_enableDevicesInterrupts i8242_enableDevicesInterrupts

// Disable the interrupts sent from the devices
#define PS2Controller_disableDevicesInterrupts i8242_disableDevicesInterrupts

// Send a byte to the PS/2 device 1
// Returns: true on success, false on failure
#define PS2Controller_sendByteToDevice1 i8242_sendByteToDevice1

// Send a byte to the PS/2 device 2
// Returns: true on success, false on failure
#define PS2Controller_sendByteToDevice2 i8242_sendByteToDevice2

// Receive a byte from a previously sent command
// Returns: true on success, false on failure (no readable byte in output buffer at the end of timeout)
// If success, write output to *byte_out
#define PS2Controller_receiveDeviceByte i8242_receiveDeviceByte

#endif
