#include "Logging.h"
#include "assert.h"
#include "IO.h"
#include "IRQ.h"

#include "i8042.h"

#define MODULE "i8042 PS/2 Controller driver"

// Intel 8042 PS/2 Controller driver

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

static bool m_enabled = false;
static bool m_isPort1Valid = false; // works
static bool m_isPort2Valid = false; // is present AND works
static bool m_translation; // Whether port 1 translation is on
static uint8_t m_configByte; // Bufferized configuration byte (used after initialization)

// TIMEOUT is a counter used when waiting for responses (such as sendCommand)
// TODO: replace by an actual timer (which does not vary on the CPU speed...)
#define TIMEOUT 1<<20

static bool sendCommand(uint8_t command);
static inline bool waitUntilBitValueOrTimeout(uint8_t mask, uint8_t value);

// Warning: this function can fail, but will still return a value
static inline uint8_t readControllerConfigurationByte(){
	uint8_t res = 0x00;

	sendCommand(PS2C_CMD_READ_CONFIG_BYTE);

	if (!i8042_receiveByte(&res))
		log(WARNING, MODULE, "Failed to read the Controller Configuration Byte !! Returned value is invalid");

	return res;
}

static inline bool writeControllerConfigurationByte(uint8_t byte){

	if (!sendCommand(PS2C_CMD_WRITE_CONFIG_BYTE))
		return false;

	if (!waitUntilBitValueOrTimeout(PS2C_STATUS_INPUT_BUFF, 0))
		return false;

	outb(PS2C_PORT_DATA, byte);
	return true;
}

void i8042_init(){
	uint8_t buff;
	IRQ_disableSpecific(IRQ_PS2_KEYBOARD);
	IRQ_disableSpecific(IRQ_PS2_MOUSE);

	// 1. Disable Legacy USB (see USB driver, if present)

	// 2. Determine if the PS/2 Controller exists
	// When ACPI will be implemented... it is present on QEMU

	// 3. Disable devices (note: these have no responses)
	sendCommand(PS2C_CMD_DISABLE_PORT1);
	sendCommand(PS2C_CMD_DISABLE_PORT2);

	// 4. Flush output buffer (discard data) if there is any
	inb(PS2C_PORT_DATA);

	// 5. Set Controller Configuration Byte
	buff = readControllerConfigurationByte();
	// Note: we keep translation as it is by default
	m_translation = (buff & PS2C_CONFBYTE_PORT1_TRANSLATION);
	buff &= ~(PS2C_CONFBYTE_PORT1_INTERRUPT|PS2C_CONFBYTE_PORT1_CLOCK);
	writeControllerConfigurationByte(buff);

	// 6. Perform self-test
	// However, according to: https://forum.osdev.org/viewtopic.php?t=57546
	// we shouldn't use the PS/2 controller's self-test command as it can have unrecoverable side
	// effects, and doesn't work correctly on hardware that is emulating a PS/2 controller in SMM.
	// So we skip this part

	// 7. Determine presence of port 2
	sendCommand(PS2C_CMD_ENABLE_PORT2);
	buff = readControllerConfigurationByte();
	// The port 2 is present if the clock was cleared (disabled) when we enabled the port
	if ( (buff & PS2C_CONFBYTE_PORT2_CLOCK) == 0 ){
		// If it is present, configure it: set controller config byte, and disable port 2
		m_isPort2Valid = true; // temp, we'll test it before actually setting it to valid
		sendCommand(PS2C_CMD_DISABLE_PORT2);
		// Disable port 2's IRQ and enable its clock
		buff &= ~(PS2C_CONFBYTE_PORT2_INTERRUPT|PS2C_CONFBYTE_PORT2_CLOCK);
		writeControllerConfigurationByte(buff);
	}
	else {
		m_isPort2Valid = false;
	}

	// 8. Perform interface tests
	// Port 1
	sendCommand(PS2C_CMD_TEST_PORT1);
	i8042_receiveByte(&buff);
	m_isPort1Valid = (buff == PS2C_RES_PORT1_TEST_SUCCESS);
	// Port 2
	if (m_isPort2Valid) {
		sendCommand(PS2C_CMD_TEST_PORT2);
		i8042_receiveByte(&buff);
		m_isPort2Valid = (buff == PS2C_RES_PORT2_TEST_SUCCESS);
	}
	// Update driver state
	m_enabled = (m_isPort1Valid || m_isPort2Valid);
	if (!m_enabled){
		log(ERROR, MODULE, "Initalization failed, no functionning PS/2 port found.");
		return;
	}

	// Re-enable devices
	if (m_isPort1Valid) sendCommand(PS2C_CMD_ENABLE_PORT1);
	if (m_isPort2Valid)	sendCommand(PS2C_CMD_ENABLE_PORT2);

	// For now on, we can bufferize the configuration byte, as no command
	// will be sent to the controller that could change it
	m_configByte = readControllerConfigurationByte();

	log(SUCCESS, MODULE, "Initalization success (port 1 %s, port 2 %s)", m_isPort1Valid ? "ON":"OFF", m_isPort2Valid ? "ON":"OFF");
	IRQ_enableSpecific(IRQ_PS2_KEYBOARD);
	IRQ_enableSpecific(IRQ_PS2_MOUSE);
}

void i8042_getStatus(bool* isEnabled_out, bool* port1Available_out, bool* port2Available_out, bool* translationOut){
	*isEnabled_out = m_enabled;
	*port1Available_out = m_isPort1Valid;
	*port2Available_out = m_isPort2Valid;
	*translationOut = m_translation;
}

void i8042_setDevicesIRQ(bool device1, bool device2){
	if (device1)
		m_configByte |=  PS2C_CONFBYTE_PORT1_INTERRUPT; // Set IRQ bit for device 1
	else
		m_configByte &= ~PS2C_CONFBYTE_PORT1_INTERRUPT; // Clear IRQ bit for device 1

	if (device2){
		m_configByte |=  PS2C_CONFBYTE_PORT2_INTERRUPT;
	}
	else{
		m_configByte &= ~PS2C_CONFBYTE_PORT2_INTERRUPT;
	}

	writeControllerConfigurationByte(m_configByte);
}

// Wait until a bit (given by the 'mask') in the status register evaluates to 'value', or we hit the timeout
static inline bool waitUntilBitValueOrTimeout(uint8_t mask, uint8_t value){
	int timer = 0;

	while (timer < TIMEOUT) {
		// Check if bit has the value requested
		uint8_t status_register = inb(PS2C_PORT_STATUS_REGISTER);
		if ( (status_register & mask) == value )
			break;

		timer++;
	}

	return (timer != TIMEOUT);
}

static inline bool sendCommand(uint8_t command){
	if (!waitUntilBitValueOrTimeout(PS2C_STATUS_INPUT_BUFF, 0))
		return false;

	outb(PS2C_PORT_COMMAND, command);
	return true;
}

bool i8042_sendByteToDevice(int device, uint8_t byte){
	assert(device == 1 || device == 2);
	bool res;

	if (device == 2){
		res = sendCommand(PS2C_CMD_WRITE_PORT2_INPUT_BUFF);
		if (!res) return false;
	}

	if (!waitUntilBitValueOrTimeout(PS2C_STATUS_INPUT_BUFF, 0))
		return false;

	outb(PS2C_PORT_DATA, byte);
	return true;
}

bool i8042_receiveByte(uint8_t* byte_out){
	if (!waitUntilBitValueOrTimeout(PS2C_STATUS_OUTPUT_BUFF, 1))
		return false;

	*byte_out = inb(PS2C_PORT_DATA);
	return true;
}
