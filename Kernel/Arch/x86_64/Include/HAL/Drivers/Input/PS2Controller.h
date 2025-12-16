#ifndef __PS2_CONTROLLER_H__
#define __PS2_CONTROLLER_H__

#include "mugOS/Preprocessor.h"
#include "Arch/x86_64/Drivers/Input/i8042.h"

static always_inline void PS2Controller_init(){
	i8042_init();
}

/// @return Returns whether the controller/driver is initialized and enabled,
/// and which ports are enabled
static always_inline void PS2Controller_getStatus(bool *enabled, bool *port1Valid,
												  bool *port2Valid, bool *translation){
	i8042_getStatus(enabled, port1Valid, port2Valid, translation);
}

/// @brief Enable IRQs for each functioning port
static always_inline void PS2Controller_enableDevicesIRQs(){
	return i8042_enableDevicesIRQs();
}

/// @brief Send a byte to the PS/2 device 1
/// @return `true` on success, `false` on failure
static always_inline bool PS2Controller_sendByteToDevice(int device, uint8_t byte){
	return i8042_sendByteToDevice(device, byte);
}

/// @brief Receive a byte from a previously sent command
/// @param byte_out Received byte is written here
/// @return `true` on success, `false` if no response was received (timeout).
static always_inline uint8_t PS2Controller_receiveByte(){
	return i8042_receiveByte();
}

#endif
