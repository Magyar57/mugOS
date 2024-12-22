#ifndef __SERIAL_H__
#define __SERIAL_H__

void Serial_initalize();
void Serial_sendByte(uint8_t byte);
uint8_t Serial_receiveByte();

#endif
