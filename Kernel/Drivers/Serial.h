#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

void Serial_initialize();
bool Serial_isEnabled();

bool Serial_sendByte(int device, uint8_t byte);
bool Serial_sendString(int device, const char* str);
uint8_t Serial_receiveByte(int device);

bool Serial_sendByteDefault(uint8_t byte);
bool Serial_sendStringDefault(const char* str);
uint8_t Serial_receiveByteDefault();

#endif
