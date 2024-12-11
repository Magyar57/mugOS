#include <stdbool.h>
#include "stdio.h"
#include "assert.h"
#include "io.h"

#include "i8242.h"

// Intel 8242 PS/2 Controller driver
// https://wiki.osdev.org/%228042%22_PS/2_Controller

// Ports
#define PS2C_PORT_DATA						0x60 // Read/Write
#define PS2C_PORT_STATUS_REGISTER			0x64 // Read
#define PS2C_PORT_COMMAND					0x64 // Write
// Status register
#define PS2C_STATUS_OUTPUT_BUFF				0b00000001
#define PS2C_STATUS_INPUT_BUFF				0b00000010
#define PS2C_STATUS_SYSTEM_FLAG				0b00000100
#define PS2C_STATUS_COMMAND_OR_DATA			0b00001000
#define PS2C_STATUS_TIMEOUT_ERR				0b01000000
#define PS2C_STATUS_PARITY_ERR				0b10000000
// Commands
#define PS2C_CMD_READ_CONFIG_BYTE			0x20
#define PS2C_CMD_WRITE_CONFIG_BYTE			0x60
#define PS2C_CMD_DISABLE_PORT2				0xa7
#define PS2C_CMD_ENABLE_PORT2				0xa8
#define PS2C_CMD_TEST_PORT2					0xa9
#define PS2C_CMD_SELF_TEST					0xaa
#define PS2C_CMD_TEST_PORT1					0xab
#define PS2C_CMD_DISABLE_PORT1				0xad
#define PS2C_CMD_ENABLE_PORT1				0xae
#define PS2C_CMD_READ_OUTPUT_PORT			0xd0
#define PS2C_CMD_WRITE_OUTPUT_PORT			0xd1
#define PS2C_CMD_WRITE_PORT1_OUTPUT_BUFF	0xd2
#define PS2C_CMD_WRITE_PORT2_OUTPUT_BUFF	0xd3
#define PS2C_CMD_WRITE_PORT2_INPUT_BUFF		0xd4
// Controller Configuration Byte
#define PS2C_CONFBYTE_PORT1_INTERRUPT		0b00000001
#define PS2C_CONFBYTE_PORT2_INTERRUPT		0b00000010
#define PS2C_CONFBYTE_SYSTEM_FLAG			0b00000100
#define PS2C_CONFBYTE_PORT1_CLOCK			0b00010000
#define PS2C_CONFBYTE_PORT2_CLOCK			0b00100000
#define PS2C_CONFBYTE_PORT1_TRANSLATION		0b01000000
// Controller Output Port
#define PS2C_OUTPUT_PORT_RESET				0b00000001
#define PS2C_OUTPUT_PORT_A20				0b00000010
#define PS2C_OUTPUT_PORT_PORT2_CLOCK		0b00000100
#define PS2C_OUTPUT_PORT_PORT2_DATA			0b00001000
#define PS2C_OUTPUT_PORT_PORT1_BUFF_FULL	0b00010000
#define PS2C_OUTPUT_PORT_PORT2_BUFF_FULL	0b00100000
#define PS2C_OUTPUT_PORT_PORT1_CLOCK		0b01000000
#define PS2C_OUTPUT_PORT_PORT1_DATA			0b10000000
// Controller responses
#define PS2C_RES_SELF_TEST_SUCCESS			0x55
#define PS2C_RES_PORT1_TEST_SUCCESS			0x00
#define PS2C_RES_PORT2_TEST_SUCCESS			0x00

// Devices (this is not the PS/2 devices driver though,
// these are needed for intialization)
#define PS2C_DEV_CMD_RESET					0xff
#define PS2C_DEV_RES_ACK					0xfa
#define PS2C_DEV_RES_SELF_TEST_PASSED		0xaa

static bool g_enabled = false;
static bool g_isPort1Valid = false; // works
static bool g_isPort2Valid = false; // is present AND works

#define TIMEOUT 1<<16 // TIMEOUT is a counter used when waiting for responses

static inline uint8_t readPort(uint8_t port){
	uint8_t res = inb(port);
	io_wait();
	return res;
}

static inline void sendToPort(uint8_t port, uint8_t data){
	outb(port, data);
	io_wait();
}

static inline uint8_t readControllerConfigurationByte(){
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_READ_CONFIG_BYTE);
	return readPort(PS2C_PORT_DATA);
}

static inline void writeControllerConfigurationByte(uint8_t byte){
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_WRITE_CONFIG_BYTE);
	sendToPort(PS2C_PORT_DATA, byte);
}

static bool i8242_resetDevice(int device){
	assert(device == 1 || device == 2);
	uint8_t buff;

	(device == 1) ?
		i8242_sendByteToDevice1(PS2C_DEV_CMD_RESET) : i8242_sendByteToDevice2(PS2C_DEV_CMD_RESET);

	// Byte 1: ACK or TEST_PASSED
	if (!i8242_receiveDeviceByte(&buff) && buff!=PS2C_DEV_RES_ACK && buff!=PS2C_DEV_RES_SELF_TEST_PASSED){
		return false;
	}
	// Byte 2: ACK or TEST_PASSED
	if (!i8242_receiveDeviceByte(&buff) && (buff!=PS2C_DEV_RES_ACK) && (buff!=PS2C_DEV_RES_SELF_TEST_PASSED)){
		return false;
	}
	// Byte 3: device ID (can be NOT present in the case of AT Keyboard)
	if (!i8242_receiveDeviceByte(&buff)){
		return true;
	}
	// Byte 4: present for some devices
	if (!i8242_receiveDeviceByte(&buff)){
		return true;
	}

	return true;
}

void i8242_initalize(){
	uint8_t buff;

	// 1. Disable Legacy USB (see USB driver, if present)

	// 2. Determine if the PS/2 Controller exists
	// When ACPI will be implemented... it is present on QEMU

	// 3. Disable devices
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_DISABLE_PORT1);
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_DISABLE_PORT2);

	// 4. Flush output buffer (discard data)
	readPort(PS2C_PORT_DATA);

	// 5. Set Controller Configuration Byte
	buff = readControllerConfigurationByte();
	buff &= ~(PS2C_CONFBYTE_PORT1_INTERRUPT|PS2C_CONFBYTE_PORT1_TRANSLATION|PS2C_CONFBYTE_PORT1_CLOCK);
	writeControllerConfigurationByte(buff);

	// 6. Perform self-test
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_SELF_TEST);
	buff = readPort(PS2C_PORT_DATA);
	if (buff != PS2C_RES_SELF_TEST_SUCCESS){
		g_enabled = false;
		puts("[ERROR!] PS/2 Controller driver: initalization failed, chip self test failed"); // TODO retry ?
		return;
	}

	// 7. Determine presence of port 2
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_ENABLE_PORT2);
	buff = readControllerConfigurationByte();
	if ( (buff & PS2C_CONFBYTE_PORT2_CLOCK) == 0 ){
		g_isPort2Valid = true; // temp, we'll test it before actually setting it to valid
		sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_DISABLE_PORT2);
		// Disable IRQ2 and enable port2 clock
		buff &= ~(PS2C_CONFBYTE_PORT2_INTERRUPT|PS2C_CONFBYTE_PORT2_CLOCK);
		writeControllerConfigurationByte(buff);
	}
	else {
		g_isPort2Valid = false;
	}

	// 8. Perform interface tests
	// Port 1
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_TEST_PORT1);
	buff = readPort(PS2C_PORT_DATA);
	g_isPort1Valid = (buff == PS2C_RES_PORT1_TEST_SUCCESS);
	// Port 2
	if (g_isPort2Valid) {
		sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_TEST_PORT2);
		buff = readPort(PS2C_PORT_DATA);
		g_isPort2Valid = (buff == PS2C_RES_PORT2_TEST_SUCCESS);
	}
	// Update driver state
	g_enabled = (g_isPort1Valid | g_isPort2Valid);
	if (!g_enabled){
		puts("[ERROR!] PS/2 Controller driver: initalization failed, no functionning PS/2 port found.");
		return;
	}

	// 9. ~~Re-enable devices~~ Actually, we DISABLE them again, because they
	// trigger interrupts even when they are disabled in the Controller Configuration Byte...
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_DISABLE_PORT1);
	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_DISABLE_PORT2);

	// 10. Reset devices
	bool res;
	res = i8242_resetDevice(1);
	if (!res) {
		g_isPort1Valid = false;
		puts("[  INFO  ] PS/2 Controller driver: Device 1 reset failed, deactivated");
	}
	res = i8242_resetDevice(2);
	if (!res) {
		g_isPort1Valid = false;
		puts("[  INFO  ] PS/2 Controller driver: Device 2 reset failed, deactivated");
	}

	// Re-enable devices and interrupts for available devices
	buff = readControllerConfigurationByte();
	if (g_isPort1Valid) {
		sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_ENABLE_PORT1);
		buff |= PS2C_CONFBYTE_PORT1_INTERRUPT;
	}
	if (g_isPort2Valid){
		buff |= PS2C_CONFBYTE_PORT2_INTERRUPT;
		sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_ENABLE_PORT2);
	}
	writeControllerConfigurationByte(buff);
	puts("[  OK  ] PS/2 Controller driver: Initalization success");
}

void i8242_getStatus(bool* isEnabled_out, bool* port1Available_out, bool* port2Available_out){
	*isEnabled_out = g_enabled;
	*port1Available_out = g_isPort1Valid;
	*port2Available_out = g_isPort2Valid;
}

void i8242_enableDevicesInterrupts(){
	uint8_t ccb = readControllerConfigurationByte();
	ccb |= (PS2C_CONFBYTE_PORT1_INTERRUPT|PS2C_CONFBYTE_PORT2_INTERRUPT);
	writeControllerConfigurationByte(ccb);
}

void i8242_disableDevicesInterrupts(){
	uint8_t ccb = readControllerConfigurationByte();
	ccb = ccb & ~(PS2C_CONFBYTE_PORT1_INTERRUPT|PS2C_CONFBYTE_PORT2_INTERRUPT);
	writeControllerConfigurationByte(ccb);
}

bool i8242_sendByteToDevice1(uint8_t byte){
	assert(g_enabled && g_isPort1Valid);

	int timer = 0;
	bool buffer_clear = false;
	while( !buffer_clear && (timer < TIMEOUT)){
		// Check if input buffer  is full
		uint8_t status_register = readPort(PS2C_PORT_STATUS_REGISTER);
		buffer_clear = ((status_register & PS2C_STATUS_INPUT_BUFF) == 0);

		timer++;
	}
	if (timer == TIMEOUT) return false;

	sendToPort(PS2C_PORT_DATA, byte);
	return true;
}

bool i8242_sendByteToDevice2(uint8_t byte){
	assert(g_enabled && g_isPort2Valid);

	sendToPort(PS2C_PORT_COMMAND, PS2C_CMD_WRITE_PORT2_INPUT_BUFF);
	// Nothing more needed for device 2, we can reuse our code for the first one
	return i8242_sendByteToDevice1(byte);
}

bool i8242_receiveDeviceByte(uint8_t* byte_out){
	assert(g_enabled);
	assert(g_isPort1Valid || g_isPort2Valid);
	bool buffer_full = false;
	uint8_t buff;

	int timer = 0;
	while( !buffer_full && (timer < TIMEOUT)){
		// Check if output buffer is full
		buff = readPort(PS2C_PORT_STATUS_REGISTER);
		buffer_full = ((buff & PS2C_STATUS_OUTPUT_BUFF) != 0);
		timer++;
	}
	if (timer >= TIMEOUT) return false;

	*byte_out = readPort(PS2C_PORT_DATA);
	return true;
}
