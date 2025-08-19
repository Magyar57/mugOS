#ifndef __PS2_CONTROLLER_H__
#define __PS2_CONTROLLER_H__

#include "Preprocessor.h"
#include "Arch/x86_64/Drivers/i8042.h"

static always_inline void PS2Controller_init(){
	i8042_init();
}

/// @returns Returns whether the controller/driver is initialized and enabled,
/// and which ports are enabled
static always_inline void PS2Controller_getStatus(bool *enabled, bool *port1Valid,
												  bool *port2Valid, bool *translation){
	i8042_getStatus(enabled, port1Valid, port2Valid, translation);
}

/// @brief Send a byte to the PS/2 device 1
/// @returns `true` on success, `false` on failure
static always_inline bool PS2Controller_sendByteToDevice(int device, uint8_t byte){
	return i8042_sendByteToDevice(device, byte);
}

/// @brief Receive a byte from a previously sent command
/// @param byte_out Received byte is written here
/// @returns `true` on success, `false` if no response was received (timeout).
static always_inline uint8_t PS2Controller_receiveByte(){
	return i8042_receiveByte();
}

#endif
