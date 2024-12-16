#include <stdbool.h>
#include "io.h"
#include "IRQ.h"
#include "Logging.h"

#include "Serial.h"

#define MODULE "Serial Port driver"

// Possible ports to poll (x86 specific !!)
const int PORTS[] = {
	0x3f8,	// SERIAL_PORT_COM1
	0x2f8,	// SERIAL_PORT_COM2
	0x3e8,	// SERIAL_PORT_COM3
	0x2e8,	// SERIAL_PORT_COM4
	0x5f8,	// SERIAL_PORT_COM5
	0x4f8,	// SERIAL_PORT_COM6
	0x5e8,	// SERIAL_PORT_COM7
	0x4e8	// SERIAL_PORT_COM8
};

// UART chip types
typedef enum {
	UART_NONE,
	UART_8250,
	UART_16450,
	UART_16550,
	UART_16550A,
} UART;
// Chip names
const char* UART_Chips[] = {
	"INVALID",
	"8250",
	"16450",
	"16550",
	"16550A"
};

int m_portNumber = -1; // From 1 to 8. -1: no serial device present
UART m_UART = 0;

// UART registers. Ports are given as an offset from the COM port
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
	int port = PORTS[m_portNumber];

	debug("COM/UART interrupt");

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
			debug("IIR: interrupt is pending => Transmitter Holding Register is Empty");
			// TODO serve "by reading IIR (if source of int only!) or writing to THR"
			break;
		case SERIAL_IIR_INT_DATA:
			debug("IIR: interrupt is pending => data available");
			// TODO serve "by reading RBR until under level"
			break;
		case SERIAL_IIR_INT_LINE:
			debug("IIR: interrupt is pending => line status changed");
			// TODO serve "by reading the LSR"
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

	// else {
	// 	debug("no interrupt pending, WTH ?");
	// }

	// Print line status for debug
	// uint8_t lsr = inb(port+SERIAL_OFFSET_LSR);
	// debug("LSR=%p", lsr);
	// if (lsr & SERIAL_LSR_DR) debug("LSR: data to be read ! :)");
	// if (lsr & SERIAL_LSR_OE) debug("LSR: data was lost");
	// if (lsr & SERIAL_LSR_PE) debug("LSR: parity detected an error");
	// if (lsr & SERIAL_LSR_FE) debug("LSR: framing error (stop bit was missing)");
	// if (lsr & SERIAL_LSR_BI) debug("LSR: got a break");
	// if (lsr & SERIAL_LSR_THRE) debug("LSR: transmission buffer empty (we can send data)");
	// if (lsr & SERIAL_LSR_TEMT) debug("LSR: transmitter is idle");
}

/// @brief Test if a UART chip is connected to a port
UART detectUART(int port){
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

static bool initalizePort(int port){
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
	// Test all ports
	for(int i=0 ; i<sizeof(PORTS)/sizeof(int) ; i++){
		UART uart = detectUART(PORTS[i]);
		if (uart == UART_NONE) continue;

		bool res = initalizePort(PORTS[i]);
		if (!res){
			log(ERROR, MODULE, "Found faulty UART Controller '%s' on COM%d, ignoring", UART_Chips[uart], i+1);
			continue;
		}

		m_portNumber = i;
		m_UART = uart;
		log(INFO, MODULE, "Found UART Controller '%s' on COM%d", UART_Chips[m_UART], m_portNumber+1);
		break; // stop at the first functionning one
	}

	// No valid device / UART found
	if (m_portNumber == -1){
		log(SUCCESS, MODULE, "Initialization found no Serial device");
		return;
	}

	// Serial_sendByte('h');
	// Serial_sendByte('e');
	// Serial_sendByte('l');
	// Serial_sendByte('l');
	// Serial_sendByte('o');
	// Serial_sendByte('\n');
	// Serial_sendByte('\r');

	IRQ_registerHandler(3, handleUARTInterrupt); // IRQ3: COM2 or COM4 port
	IRQ_registerHandler(4, handleUARTInterrupt); // IRQ4: COM1 or COM3 port
	enablePortInterrupts(m_portNumber);
	log(SUCCESS, MODULE, "Initialized, using device on COM%d", m_portNumber+1);
}

// Aka can send
static bool isTransmitterReady(){
	uint8_t lineStatus = inb(PORTS[m_portNumber] + SERIAL_OFFSET_LSR);
	return (lineStatus & SERIAL_LSR_THRE); // bit set: can send
}

bool isDataPresent(){
	uint8_t lineStatus = inb(PORTS[m_portNumber] + SERIAL_OFFSET_LSR);
	return (lineStatus & SERIAL_LSR_DR); // bit set: there's data to be read
}

void Serial_sendByte(uint8_t byte){
	while(!isTransmitterReady()); // wait
	outb(PORTS[m_portNumber]+SERIAL_OFFSET_BUFFER, byte);
}

uint8_t Serial_receiveByte(){
	while(!isDataPresent()); // wait
	return inb(PORTS[m_portNumber] + SERIAL_OFFSET_BUFFER);
}
