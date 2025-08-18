#include <stdint.h>
#include "Logging.h"
#include "IO.h"

#include "PIT.h"

// Programmable Interrupt Timer driver
// Note: only channel 0 is supported

#define PORT_CHANNEL0		0x40 // Channel 0 data port (r/w)
#define PORT_CHANNEL1		0x41 // Channel 1 data port (r/w)
#define PORT_CHANNEL2		0x42 // Channel 2 data port (r/w)
#define PORT_COMMAND_REG	0x43 // Mode/command register (w)

#define BASE_FREQUENCY 1193181 // Hz (1.1931816666 MHz)

union CommandReg {
	uint8_t value;
	struct {
		// true: BCD mode, false: 16-bit binary mode
		uint8_t bcdMode : 1;
		// 0b000: Mode 0 - Interrupt on terminal count
		// 0b001: Mode 1 - Hardware re-triggerable one-shot
		// 0b010: Mode 2 - Rate generator
		// 0b011: Mode 3 - Square wave generator
		// 0b100: Mode 4 - Software triggered strobe
		// 0b101: Mode 5 - Hardware triggered strobe
		// 0b110: Mode 2
		// 0b111: Mode 3
		uint8_t operatingMode : 3;
		// 0b00: Latch count value command
		// 0b01: Access mode: low byte only
		// 0b10: Access mode: high byte only
		// 0b11: Access mode: low byte/high byte
		uint8_t accessMode : 2;
		// 0b00: Channel 0
		// 0b01: Channel 1
		// 0b10: Channel 2
		// 0b11: Read-back command (8254 only)
		uint8_t channel : 2;
	} bits;
};

// static void setDivisor(uint16_t div){
// 	debug("todo set divisor to %d", div);
// }

void PIT_init(){

	union CommandReg command;
	// command.value = inb(PORT_COMMAND_REG);
	command.bits.channel = 0b00;
	command.bits.accessMode = 0b00;
	command.bits.operatingMode = 0b010;
	command.bits.bcdMode = false;
	outb(PORT_COMMAND_REG, command.value);

	debug("set command register with value %#hhx", inb(PORT_COMMAND_REG));
}
