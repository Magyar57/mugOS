#include <stdbool.h>
#include "io.h"
#include "IRQ.h"
#include "Logging.h"

#include "Serial.h"

// This is both:
// - A x86 UART Controller driver (8250, 16450, 16550 and 16550A)
// - A UART Device interface

#define MODULE "Serial Port driver"

// Possible ports to poll (x86 specific !)
#define N_PORTS 8
const int PORTS[N_PORTS] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8, 0x5f8, 0x4f8, 0x5e8, 0x4e8 };

// UART controllers types
typedef enum {
	UART_NONE,
	UART_8250,
	UART_16450,
	UART_16550,
	UART_16550A,
} UARTController;
const char* UARTControllersNames[] = { "None", "8250", "16450", "16550", "16550A" };

typedef struct s_UARTDevice {
	int identifier; // COM1 on DOS, /dev/ttyS0 on linux ; we simply use the number
	int present;
	int port;
	UARTController controller;
	const char* controllerName;
} UARTDevice;

UARTDevice m_devices[N_PORTS];
int m_defaultDevice = -1;

// UART registers. Ports are given as an offset from the device port
#define SERIAL_OFFSET_BUFFER				0 // - Receive/transmit buffer (read/write)
#define SERIAL_OFFSET_IER					1 // - Interrupt enable register (IER)			=> (READ/WRITE)	make the chip trigger (or not) IRQs
#define SERIAL_OFFSET_IIR					2 // - Interrupt Identification Register (IIR)	=> (READ)       info about currently pending IRQs
#define SERIAL_OFFSET_FCR					2 // - FIFO Control Register (FCR)				=> (WRITE)      control FIFO buffer settings
#define SERIAL_OFFSET_LCR					3 // - Line Control Register (LCR)				=> (READ/WRITE) sets connection (line) parameters
#define SERIAL_OFFSET_MCR					4 // - Modem Control Register (MCR)				=> (READ/WRITE) Set/clear active lines (such as IRQs)
#define SERIAL_OFFSET_LSR					5 // - Line Status Register (LSR)				=> (READ)       gives info on current line (errors and more)
#define SERIAL_OFFSET_MSR					6 // - Modem Status Register (MSR)				=> (READ)       gives current state of the control lines from a device
#define SERIAL_OFFSET_SSR					7 // - Scratch Status Register (SSR)			=> (READ/WRITE) ???
#define SERIAL_OFFSET_BAUD_LOW				0
#define SERIAL_OFFSET_BAUD_HIGH				1

#pragma region Registers mappings
// IER: Interrupt Enable Register
#define SERIAL_IER_DR							0b00000001 // Data ready (DR) interrupt
#define SERIAL_IER_THRE							0b00000010 // Transmitter Holding Register Empty (THRE) interrupt
#define SERIAL_IER_LINE							0b00000100 // Line Status interrupt
#define SERIAL_IER_MODEM						0b00001000 // Modem Status interrupt
// FCR: First In First Out Control Register
#define SERIAL_FCR_ENABLE_FIF0					0b00000001
#define SERIAL_FCR_CLEAR_RECEIVE				0b00000010
#define SERIAL_FCR_CLEAR_TRANSMIT				0b00000100
#define SERIAL_FCR_DMA_MODE_SELECT				0b00001000
#define SERIAL_FCR_INTERRUPT_TRIGGER_LEVEL_1B	0b00000000
#define SERIAL_FCR_INTERRUPT_TRIGGER_LEVEL_4B	0b01000000
#define SERIAL_FCR_INTERRUPT_TRIGGER_LEVEL_8B	0b10000000
#define SERIAL_FCR_INTERRUPT_TRIGGER_LEVEL_14B	0b11000000
// Masks and values for the IIR: Interrupt Identification Register
#define SERIAL_IIR_INT_PENDING					0b00000001 // xxx 1 Interruption pending (if clear!!! if set, no int pending)
#define SERIAL_IIR_INT_MODEM					0b00000000 // 000 0 Modem Status
#define SERIAL_IIR_INT_TRANSMITTER				0b00000010 // 001 0 Transmitter Holding Register Empty (THRE)
#define SERIAL_IIR_INT_DATA						0b00000100 // 010 0 Received Data Available
#define SERIAL_IIR_INT_LINE						0b00000110 // 011 0 Receiver Line Status
#define SERIAL_IIR_INT_FIFO						0b00001100 // 110 0 No receiver FIFO action since 4 words' time
#define SERIAL_IIR_FIFO_BUFF_STATE				0b11000000 // Values below are for this mask
#define SERIAL_IIR_FIFO_BUFF_STATE_NONE			0b00000000 // 00 No FIFO
#define SERIAL_IIR_FIFO_BUFF_STATE_UNUSABLE		0b01000000 // 01 FIFO enabled but unusable
#define SERIAL_IIR_FIFO_BUFF_STATE_ENABLED		0b10000000 // 11 FIFO enabled
// MCR: Modem Control Register
#define SERIAL_MCR_DTR							0b00000001 // Data Terminal Ready pin
#define SERIAL_MCR_RTS							0b00000010 // Request To Send pin
#define SERIAL_MCR_OUT1							0b00000100 // On PCs, unused
#define SERIAL_MCR_OUT2							0b00001000 // On PCs, OUT2 is used to enable IRQs
#define SERIAL_MCR_LOOP							0b00010000 // Local loopback feature to test the UART
// LSR: Line Status Register
#define SERIAL_LSR_DR							0b00000001 // Data ready (can be read)
#define SERIAL_LSR_OE							0b00000010 // Overrun error (data has been lost)
#define SERIAL_LSR_PE							0b00000100 // Parity error (error was detected with parity bit)
#define SERIAL_LSR_FE							0b00001000 // Framing error (stop bit was missing)
#define SERIAL_LSR_BI							0b00010000 // Break indicator (there is a break in data input)
#define SERIAL_LSR_THRE							0b00100000 // Transmission buffer is empty (data can be sent)
#define SERIAL_LSR_TEMT							0b01000000 // Transmitter empty (set if transmitter is idle)
#define SERIAL_LSR_IE							0b10000000 // Impending error (error with a word in the input buffer)
#pragma endregion

// Divisor value. Note: the UART clock is the serial controller.
// BAUD_RATE = UART_CLOCK_SPEED / DIVISOR
// 38400 = 115200 / DIVISOR => DIVISOR=3
// 9600 = 115200 / DIVISOR => DIVISOR=12
#define BAUD_RATE_DIVISOR_LOW 12 // low byte
#define BAUD_RATE_DIVISOR_HIGH 0 // high byte

void handleUARTInterrupt(ISR_Params* params){
	uint8_t iir;

	// Determine port
	int port = PORTS[0]; // TODO how ?

	// Service interrupts while bit 0 is clear
	do {
		// Read IRR, only keep interrupt bits (lower 3 bits)
		iir = inb(port+SERIAL_OFFSET_IIR) & 0b00001111;
		debug("iir=%d", iir);

		switch (iir){
		case SERIAL_IIR_INT_PENDING:
			debug("IIR: no interrupt pending, WTF ?");
			break;
		case SERIAL_IIR_INT_MODEM:
			debug("IIR: interrupt is pending => modem status interrupt");
			uint8_t msr = inb(port+SERIAL_OFFSET_MSR);
			debug("read MSR: %p", msr);
			break;
		case SERIAL_IIR_INT_TRANSMITTER:
			debug("IIR: interrupt is pending => THRE out buff empty");
			// TODO serve "by reading IIR (if source of int only!) or writing to THR"
			break;
		case SERIAL_IIR_INT_DATA:
			debug("IIR: interrupt is pending => data available");
			// TODO serve "by reading RBR until under level"
			break;
		case SERIAL_IIR_INT_LINE:
			debug("IIR: interrupt is pending => line status changed");
			// serve "by reading the LSR"
			uint8_t lsr = inb(port+SERIAL_OFFSET_LSR);
			if (lsr & SERIAL_LSR_DR) debug("LSR (%p): data to be read ! :)", lsr);
			if (lsr & SERIAL_LSR_OE) debug("LSR(%p): data was lost", lsr);
			if (lsr & SERIAL_LSR_PE) debug("LSR(%p): parity detected an error", lsr);
			if (lsr & SERIAL_LSR_FE) debug("LSR(%p): framing error (stop bit was missing)", lsr);
			if (lsr & SERIAL_LSR_BI) debug("LSR(%p): got a break", lsr);
			if (lsr & SERIAL_LSR_THRE) debug("LSR(%p): transmission buffer empty (we can send data)", lsr);
			if (lsr & SERIAL_LSR_TEMT) debug("LSR(%p): transmitter is idle", lsr);
			break;
		case SERIAL_IIR_INT_FIFO:
			debug("IIR: interrupt is pending => no receiver FIFO action since 4 words' time but data in RX-FIFO");
			// TODO serve "by reading RBR"
			break;
		default:
			debug("Invalid IRR value, WTF ??");
			break;
		}

		// Update for next iteration
		iir = inb(port+SERIAL_OFFSET_IIR) & 0x07;
		static bool firstIter = true;
		if (!firstIter)
			for(;;);
		else
			firstIter = false;
	} while (iir != SERIAL_IIR_INT_PENDING);
}

/// @brief Test if a UART chip is connected to a port
UARTController detectUART(int port){
   	int mcr_save;

   	// Check if a UART is present anyway
   	mcr_save = inb(port+SERIAL_OFFSET_MCR);
   	outb(port+SERIAL_OFFSET_MCR, 0x10);
   	if ((inb(port+SERIAL_OFFSET_MSR) & 0xf0)) return 0;
   	outb(port+SERIAL_OFFSET_MCR, 0x1f);
   	if ((inb(port+SERIAL_OFFSET_MSR) & 0xf0) != 0xf0) return UART_NONE;
   	outb(port+SERIAL_OFFSET_MCR, mcr_save);

   	// Next thing to do is look for the scratch register
   	mcr_save = inb(port+SERIAL_OFFSET_SSR);
   	outb(port+SERIAL_OFFSET_SSR, 0x55);
   	if (inb(port+SERIAL_OFFSET_SSR) != 0x55) return UART_8250;
   	outb(port+SERIAL_OFFSET_SSR, 0xaa);
   	if (inb(port+SERIAL_OFFSET_SSR) != 0xaa) return UART_8250;
   	outb(port+SERIAL_OFFSET_SSR, mcr_save); // we don't need to restore it if it's not there

   	// Then check if there's a FIFO
   	outb(port + SERIAL_OFFSET_FCR, SERIAL_FCR_ENABLE_FIF0);
   	int iir = inb(port + SERIAL_OFFSET_IIR);
   	outb(port + SERIAL_OFFSET_FCR, 0x0); // some old-fashioned software relies on this! (apparently)
   	if ((iir & SERIAL_IIR_FIFO_BUFF_STATE_ENABLED) == 0) return UART_16450;
   	if ((iir & SERIAL_IIR_FIFO_BUFF_STATE_UNUSABLE) == 0) return UART_16550;
   	return UART_16550A;
}

static bool initalizeDevice(int port){
	outb(port+SERIAL_OFFSET_IER, 0x00); // Disable all interrupts

	// Set BAUD rate
	outb(port+SERIAL_OFFSET_LCR, 0x80); // set DLAB bit
	outb(port+SERIAL_OFFSET_BAUD_LOW, BAUD_RATE_DIVISOR_LOW);
	outb(port+SERIAL_OFFSET_BAUD_HIGH, BAUD_RATE_DIVISOR_HIGH);

	// LCR: 8 bits, no parity, one stop bit (& DLAB clear)
	outb(port+SERIAL_OFFSET_LCR, 0x03);

	// FCR: Enable FIFO, clear them, with 14-byte threshold
	outb(port+SERIAL_OFFSET_FCR,
			SERIAL_FCR_INTERRUPT_TRIGGER_LEVEL_14B
			|SERIAL_FCR_ENABLE_FIF0
			|SERIAL_FCR_CLEAR_RECEIVE
			|SERIAL_FCR_CLEAR_TRANSMIT);

	// MCR (configure pins): IRQs enabled, RTS/DSR set
	// outb(port+SERIAL_OFFSET_MCR, SERIAL_MCR_DTR|SERIAL_MCR_RTS|SERIAL_MCR_OUT2);

	// Test if chip is faulty (i.e: not same byte as sent)
	outb(port+SERIAL_OFFSET_MCR, SERIAL_MCR_LOOP); // Set in loopback mode, test the serial chip
	outb(port+SERIAL_OFFSET_BUFFER, 0xae);	// Test serial chip (send byte 0xAE and check if serial returns same byte)
 	if(inb(port+SERIAL_OFFSET_BUFFER) != 0xae)
 		return false;

	// MCR (configure pins)
	// If serial is not faulty set it in normal operation mode:
	// IRQs enabled, RTS/DSR set, OUT1 and OUT2 enabled, no loopback
	outb(port+SERIAL_OFFSET_MCR, SERIAL_MCR_DTR|SERIAL_MCR_RTS|SERIAL_MCR_OUT1|SERIAL_MCR_OUT2);

	return true;
}

static void enablePortInterrupts(int port){
	// Enable everything
	uint8_t val = SERIAL_IER_DR|SERIAL_IER_THRE|SERIAL_IER_LINE|SERIAL_IER_MODEM;
	outb(port+SERIAL_OFFSET_IER, val);
}

void Serial_initalize(){
	int nDevices = 0;

	// Test all ports
	for(int i=0 ; i<N_PORTS ; i++){
		UARTDevice* curDev = m_devices+i;

		curDev->identifier = i+1;
		curDev->port = PORTS[i];
		curDev->controller = detectUART(curDev->port);
		curDev->controllerName = UARTControllersNames[curDev->controller];
		if (curDev->controller == UART_NONE) continue;

		curDev->present = initalizeDevice(curDev->port);
		if (!curDev->present){
			log(ERROR, MODULE, "Found faulty UART Controller '%s' on port %d, ignoring", curDev->controllerName, curDev->identifier);
			continue;
		}

		log(INFO, MODULE, "Found UART Controller '%s' on port %d", curDev->controllerName, curDev->identifier);
		nDevices++;
		if (m_defaultDevice < 0) m_defaultDevice = i; // Default device is the first encountered
	}

	// No valid device / UART found
	if (nDevices == 0){
		log(SUCCESS, MODULE, "Initialization found no Serial device");
		return;
	}

	IRQ_registerHandler(3, handleUARTInterrupt); // IRQ3: COM2 or COM4 port
	IRQ_registerHandler(4, handleUARTInterrupt); // IRQ4: COM1 or COM3 port
	for(int i=0 ; i<N_PORTS ; i++){
		if (m_devices[i].present){
			enablePortInterrupts(m_devices[i].port);
		}
	}
	log(SUCCESS, MODULE, "Initialized %d Serial device(s)", nDevices);
	log(INFO, MODULE, "Driver defaults to Serial device %d", m_defaultDevice+1);

	// Serial_sendByte('h');
	// Serial_sendByte('e');
	// Serial_sendByte('l');
	// Serial_sendByte('l');
	// Serial_sendByte('o');
	// Serial_sendByte('\n');
	// Serial_sendByte('\r');
}

// Aka can send
static bool isTransmitterReady(){
	if (m_defaultDevice<0) return false;

	uint8_t lineStatus = inb(m_devices[m_defaultDevice].port + SERIAL_OFFSET_LSR);
	return (lineStatus & SERIAL_LSR_THRE); // bit set: can send
}

bool isDataPresent(){
	if (m_defaultDevice<0) return false;

	uint8_t lineStatus = inb(m_devices[m_defaultDevice].port + SERIAL_OFFSET_LSR);
	return (lineStatus & SERIAL_LSR_DR); // bit set: there's data to be read
}

void Serial_sendByte(uint8_t byte){
	if (m_defaultDevice<0) return;

	while(!isTransmitterReady()); // wait
	outb(m_devices[m_defaultDevice].port+SERIAL_OFFSET_BUFFER, byte);
}

uint8_t Serial_receiveByte(){
	if (m_defaultDevice<0) return 0;

	while(!isDataPresent()); // wait
	return inb(m_devices[m_defaultDevice].port + SERIAL_OFFSET_BUFFER);
}
