#ifndef __PS2_CONTROLLER_H__
#define __PS2_CONTROLLER_H__

#include "Preprocessor.h"
#include "Arch/x86_64/Drivers/i8042.h"

always_inline void PS2Controller_init(){
	i8042_init();
}

/// @returns Returns whether the controller/driver is initialized and enabled, and which ports are enabled
always_inline void PS2Controller_getStatus(bool *isEnabled_out, bool *port1Available_out, bool *port2Available_out, bool *translationOut){
	i8042_getStatus(isEnabled_out, port1Available_out, port2Available_out, translationOut);
}

/// @brief Enable/disable the IRQs sent from the devices
always_inline void PS2Controller_setDevicesIRQ(bool device1, bool device2){
	i8042_setDevicesIRQ(device1, device2);
}

// Send a byte to the PS/2 device 1
// Returns: true on success, false on failure
always_inline bool PS2Controller_sendByteToDevice(int device, uint8_t byte){
	return i8042_sendByteToDevice(device, byte);
}

/// @brief Receive a byte from a previously sent command
/// @returns true on success, false on failure (no readable byte in output buffer at the end of timeout).
/// If success, write output to *byte_out
always_inline bool PS2Controller_receiveByte(uint8_t* byte_out){
	return i8042_receiveByte(byte_out);
}

#endif
