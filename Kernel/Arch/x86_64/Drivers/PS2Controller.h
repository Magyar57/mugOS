#ifndef __PS2_CONTROLLER_H__
#define __PS2_CONTROLLER_H__

#include "Drivers/i8042.h"

#define PS2Controller_initialize i8042_initialize

/// @returns Returns whether the controller/driver is initialized and enabled, and which ports are enabled
#define PS2Controller_getStatus i8042_getStatus

/// @brief Enable the interrupts sent from the devices
#define PS2Controller_enableDevicesInterrupts i8042_enableDevicesInterrupts

/// @brief Disable the interrupts sent from the devices
#define PS2Controller_disableDevicesInterrupts i8042_disableDevicesInterrupts

// Send a byte to the PS/2 device 1
// Returns: true on success, false on failure
#define PS2Controller_sendByteToDevice1 i8042_sendByteToDevice1

/// @brief Send a byte to the PS/2 device 2
/// @returns true on success, false on failure
#define PS2Controller_sendByteToDevice2 i8042_sendByteToDevice2

/// @brief Receive a byte from a previously sent command
/// @returns true on success, false on failure (no readable byte in output buffer at the end of timeout).
/// If success, write output to *byte_out
#define PS2Controller_receiveDeviceByte i8042_receiveDeviceByte

#endif
