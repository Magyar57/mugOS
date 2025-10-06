#include <stdint.h>
#include <stdatomic.h>
#include "Logging.h"
#include "IO.h"
#include "HAL/Halt.h"
#include "IRQ.h"

#include "PIT.h"
#define MODULE "PIT"

// Programmable Interrupt Timer driver
// Note: only channel 0 is supported

#define PORT_CHANNEL0		0x40 // Channel 0 data port (r/w)
#define PORT_CHANNEL1		0x41 // Channel 1 data port (r/w)
#define PORT_CHANNEL2		0x42 // Channel 2 data port (r/w)
#define PORT_COMMAND_REG	0x43 // Mode/command register (w)

// Given a PIT base frequency f0 = 1.193181 MHz
// To choose a divisor d = f0/f
// T = 1/f = d/f0
// To have a precision p, we need T <= p <=> d <= f0*p

#define BASE_FREQUENCY	1193181 // Hz (1.1931816666 MHz)
#define DIVISOR			1193
#define PRECISION		DIVISOR*1000000 / BASE_FREQUENCY // µs

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

// Count of the number of ticks since intialization.
// Has atomicity for beeing thread and IRQ-safe
atomic_uint_fast64_t m_count = 0;

static void setDivisor(uint16_t div){
	uint8_t low_byte = div & 0xff;
	uint8_t high_byte = (div >> 8) & 0xff;

	IRQ_disable();
	outb(PORT_CHANNEL0, low_byte);
	outb(PORT_CHANNEL0, high_byte);
	IRQ_enable();
}

void pitIrq(void*){
	// IRQs are disabled, no need for atomicity here
	m_count++;
}

static void sleepMiliseconds(unsigned long ms){
	uint64_t curCount, initialCount = m_count;
	unsigned long waited = 0; // µs (same as PRECISION)

	do {
		halt();
		curCount = atomic_load(&m_count);
		waited = (curCount - initialCount) * PRECISION;
	} while(waited/1000 <= ms);
}

void PIT_init(struct EventTimer* pit){
	union CommandReg command;
	command.bits.channel = 0b00;
	command.bits.accessMode = 0b11;
	command.bits.operatingMode = 0b010;
	command.bits.bcdMode = false;
	outb(PORT_COMMAND_REG, command.value);

	setDivisor(DIVISOR);

	IRQ_installHandler(IRQ_PIT, pitIrq);
	IRQ_enableSpecific(IRQ_PIT);

	pit->name = "PIT";
	pit->sleep = PIT_sleep;
	pit->msleep = PIT_msleep;
	pit->usleep = PIT_usleep;
	pit->nsleep = PIT_nsleep;

	log(SUCCESS, MODULE, "Initialized success. Timer period/precision of T=%dus", PRECISION);
}

void PIT_sleep(unsigned long sec){
	sleepMiliseconds(1000*sec);
}

void PIT_msleep(unsigned long ms){
	sleepMiliseconds(ms);
}

void PIT_usleep(unsigned long us){
	unsigned long ms = max(us / 1000, 1);
	sleepMiliseconds(ms);
}

void PIT_nsleep(unsigned long ns){
	unsigned long ms = max(ns / 1000000, 1);
	sleepMiliseconds(ms);
}
