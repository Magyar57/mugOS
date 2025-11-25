#include <stdint.h>
#include <stdatomic.h>
#include "Logging.h"
#include "IO.h"
#include "IRQ/IRQ.h"
#include "Time/Time.h"
#include "HAL/Halt.h"

#include "PIT.h"
#define MODULE "PIT"

// Programmable Interrupt Timer driver
// Note: only channel 0 is supported

#define PORT_CHANNEL0		0x40 // Channel 0 counter register port (r/w)
#define PORT_CHANNEL1		0x41 // Channel 1 counter register port (r/w)
#define PORT_CHANNEL2		0x42 // Channel 2 counter register port (r/w)
#define PORT_COMMAND_REG	0x43 // Mode/command register (w)

#define BASE_FREQUENCY	1193181 // Hz (1.1931816666 MHz)

union CommandReg {
	uint8_t value;
	struct {
		// true: BCD mode, false: 16-bit binary mode
		uint8_t bcdMode : 1;
		// `0b000` Mode 0 - Interrupt on terminal count
		// `0b001` Mode 1 - Hardware re-triggerable one-shot
		// `0b010` Mode 2 - Rate generator
		// `0b011` Mode 3 - Square wave generator
		// `0b100` Mode 4 - Software triggered strobe
		// `0b101` Mode 5 - Hardware triggered strobe
		// `0b110` Mode 2
		// `0b111` Mode 3
		uint8_t operatingMode : 3;
		// `0b00` Latch count value command
		// `0b01` Access mode: low byte only
		// `0b10` Access mode: high byte only
		// `0b11` Access mode: low byte/high byte
		uint8_t accessMode : 2;
		// `0b00` Channel 0: PIC IRQ 0
		// `0b01` Channel 1: DRAM refresh (legacy)
		// `0b10` Channel 2: IBM-PC speaker
		// `0b11` Read-back command (8254 only)
		uint8_t channel : 2;
	} bits;
};

static void scheduleEvent(unsigned long ticks);

static struct EventTimer m_pit = {
	.name = "PIT",
	.score = 1,
	.frequency = BASE_FREQUENCY,
	.scheduleEvent = scheduleEvent,
	.minTick = 1,
	.maxTick = UINT16_MAX
};

static atomic_bool m_initIrqReceived = false;

static void writeCounterRegister(uint16_t channel, uint16_t value){
	uint8_t low_byte = value & 0xff;
	uint8_t high_byte = (value >> 8) & 0xff;

	unsigned long flags;
	IRQ_disableSave(flags);

	outb(channel, low_byte);
	outb(channel, high_byte);

	IRQ_restore(flags);
}

static void pitIrq(void*){
	m_pit.eventHandler();
}

static void pitInitIrq(void*){
	atomic_store(&m_initIrqReceived, true);
}

/// @brief Schedule an event (IRQ) to fire in `tick` PIT ticks
static void scheduleEvent(unsigned long ticks){
	writeCounterRegister(PORT_CHANNEL0, ticks);
}

void PIT_init(){
	IRQ_disable();

	// Setup channel 0 to one-shot mode
	// In this mode, writing the counter register will make the PIC start decreeasing it,
	// and trigger a single IRQ when it reaches 0. Hence scheduleEvent()'s implementation
	union CommandReg command;
	command.bits.channel = 0b00;
	command.bits.accessMode = 0b11;
	command.bits.operatingMode = 0b000;
	command.bits.bcdMode = false;
	outb(PORT_COMMAND_REG, command.value);

	// We need to set the counter so that the PIT actually changes mode,
	// which triggers a one-shot IRQ
	IRQ_installHandler(IRQ_PIT, pitInitIrq);
	IRQ_enableSpecific(IRQ_PIT);
	IRQ_enable();
	writeCounterRegister(PORT_CHANNEL0, 1);

	// Wait for the IRQ
	while (!atomic_load(&m_initIrqReceived))
		pause();

	// Now we can install our actual IRQ handler
	IRQ_installHandler(IRQ_PIT, pitIrq);
	IRQ_enableSpecific(IRQ_PIT);

	Time_registerEventTimer(&m_pit);

	log(SUCCESS, MODULE, "Initialized success, frequency is %lu.%06lu MHz",
		BASE_FREQUENCY / 1000000, BASE_FREQUENCY % 1000000);
}
