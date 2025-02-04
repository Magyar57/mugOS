#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>
#include <stdbool.h>

void Serial_initialize();
bool Serial_isEnabled();

void Serial_sendByte(int device, uint8_t byte);
void Serial_sendString(int device, const char* str);
uint8_t Serial_receiveByte(int device);

void Serial_sendByteDefault(uint8_t byte);
void Serial_sendStringDefault(const char* str);
uint8_t Serial_receiveByteDefault();

#endif
