#include <stdbool.h>
#include "string.h"
#include "io.h"
#include "assert.h"
#include "CPU.h"
#include "IRQ.h"
#include "Logging.h"

#include "Serial.h"

// This is both:
// - A x86 UART Controller driver (8250, 16450, 16550 and 16550A)
// - A UART Device interface
// Note: only the 16550A controller has been tested. Other might work
// with a few bugs, but modern CPU onboards 16550A and no other
// so this driver should work with all modern systems

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

#define UARTDEVICE_EXT_BUFF_SIZE 1024
typedef struct s_UARTDevice {
	int identifier; // COM1 on DOS, /dev/ttyS0 on linux ; we simply use the number
	int present;
	int port;
	UARTController controller;
	const char* controllerName;
	int internalBufferSize; // Internal FIFO buffer size: 14 on 16550A, 1 otherwise
	uint8_t externalWriteBuffer[UARTDEVICE_EXT_BUFF_SIZE];
	unsigned int writeBeginIndex, inWriteBuffer; // externalWriteBuffer FIFO variables
	uint8_t externalReadBuffer[UARTDEVICE_EXT_BUFF_SIZE];
	unsigned int readBeginIndex, inReadBuffer; // externalWriteBuffer FIFO variables
} UARTDevice;

UARTDevice m_devices[N_PORTS];
int m_defaultDevice = -1;
bool m_enabled = false;

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

static void processTHRE(UARTDevice* dev);

// ================ External buffers handling ================

// Add (push back) the null-terminated string str to be written the device buffer
// If not enough place in buffer, send
static bool pushBackWriteBuffer(UARTDevice* dev, const uint8_t* str){
	assert(str);
	if (dev==NULL) return false;
	if (str==NULL) return true;

	size_t n = strlen((const char*) str);
	if (n==0) return true;

	// If we were not already writing (buffer empty), after filling the buffer,
	// we need to trigger the THRE again so that we actually send what we put in the buffer
	bool shouldTriggerTHRE = (dev->inWriteBuffer == 0);

	size_t availableSpace = UARTDEVICE_EXT_BUFF_SIZE - dev->inWriteBuffer;
	if (availableSpace < n) return false;

	// Copy the string to the external buffer
	for(int i=0 ; i<n ; i++){
		dev->externalWriteBuffer[dev->writeBeginIndex + dev->inWriteBuffer] = str[i];
		dev->inWriteBuffer++;
	}

	// Trigger THRE if we were not writing
	if (shouldTriggerTHRE){
		uint8_t ier = inb(dev->port+SERIAL_OFFSET_IER);
		ier |= SERIAL_IER_THRE;
		outb(dev->port+SERIAL_OFFSET_IER, ier);
	}

	// Test if THRE was not triggered, do it manually
	// Note: we test whether writeBuffer is full because activating THRE
	// might have triggered the interrupt, possibly emptying the buffer
	disableInterrupts();
	uint8_t lsr = inb(dev->port + SERIAL_OFFSET_LSR);
	if ((lsr & SERIAL_LSR_TEMT) && dev->inWriteBuffer){
		processTHRE(dev);
	}
	enableInterrupts();

	return true;
}

// Remove (pop front) a byte from the buffer
static uint8_t popFrontWriteBuffer(UARTDevice* dev){
	// n can be passed as an argument,
	// if we're able to return several char
	int n = 1;
	uint8_t res;

	// Clamp
	if (dev->inWriteBuffer < n) n = dev->inWriteBuffer;

	res = dev->externalWriteBuffer[dev->writeBeginIndex];
	dev->writeBeginIndex = (dev->writeBeginIndex + n) % UARTDEVICE_EXT_BUFF_SIZE;
	dev->inWriteBuffer -= n;

	// If nothing more to be written, disable THRE
	if (dev->inWriteBuffer == 0){
		uint8_t ier = inb(dev->port+SERIAL_OFFSET_IER);
		ier &= ~SERIAL_IER_THRE;
		outb(dev->port+SERIAL_OFFSET_IER, ier);
	}

	return res;
}

static bool pushBackReadBuffer(UARTDevice* dev, const uint8_t* str){
	assert(str);
	if (dev==NULL) return false;
	if (str==NULL) return true;

	size_t n = strlen((const char*) str);
	if (n==0) return true;

	size_t availableSpace = UARTDEVICE_EXT_BUFF_SIZE - dev->inReadBuffer;
	if (availableSpace < n) return false;

	// Copy the string to the external buffer
	for(int i=0 ; i<n ; i++){
		dev->externalReadBuffer[dev->readBeginIndex + dev->inReadBuffer] = str[i];
		dev->inReadBuffer++;
	}

	return true;
}

static uint8_t popFrontReadBuffer(UARTDevice* dev){
	// n can be passed as an argument,
	// if we're able to return several char
	int n = 1;
	uint8_t res;

	// Clamp
	if (dev->inReadBuffer < n) n = dev->inReadBuffer;

	res = dev->externalReadBuffer[dev->readBeginIndex];
	dev->readBeginIndex = (dev->readBeginIndex + n) % UARTDEVICE_EXT_BUFF_SIZE;
	dev->inReadBuffer -= n;

	// If nothing more to be written, disable THRE
	if (dev->inReadBuffer == 0){
		uint8_t ier = inb(dev->port+SERIAL_OFFSET_IER);
		ier &= ~SERIAL_IER_THRE;
		outb(dev->port+SERIAL_OFFSET_IER, ier);
	}

	return res;
}

// ================ Interrupt handling ================

// Process available data interrupts:
// - DR or trigger level reached
// - No receiver FIFO action since 4 words' time but data in RX-FIFO
static void processAvailableData(UARTDevice* dev){
	// Served "by reading RBR (until under level)"
	//
	// Read available data from the internal FIFO buffer, and send it back

	uint8_t temp[15];
	memset(temp, 0, sizeof(temp));

	// Read the available data
	for(int i=0 ; i<dev->internalBufferSize ; i++){
		// ONLY read while DR (Data Ready) set
		uint8_t lsr = inb(dev->port+SERIAL_OFFSET_LSR);
		if (!(lsr & SERIAL_LSR_DR))
			break;

		uint8_t buff = inb(dev->port+SERIAL_OFFSET_BUFFER);
		temp[i] = buff;
	}

	// Put it in the buffer for public access
	if (!pushBackReadBuffer(dev, temp)){
		log(WARNING, MODULE, "Received data but read buffer is full, data will be discarded. Please read more often !");
	}

	// Send it back
	pushBackWriteBuffer(dev, temp);
}

// Process THRE: Transmitter Holding Register Empty interrupt
static void processTHRE(UARTDevice* dev){
	// Served "by reading IIR or writing to THR"
	//
	// If we have data in the external buffer to be sent,
	// write it to the controller's internal buffer

	int i = 0;
	while(i<dev->internalBufferSize && dev->inWriteBuffer>0){
		char toSend = popFrontWriteBuffer(dev);
		outb(dev->port+SERIAL_OFFSET_BUFFER, toSend);
		i++;
	}

	// Emptied buffer => Disable THRE interrupt
	if (dev->inWriteBuffer == 0){
		uint8_t ier = inb(dev->port+SERIAL_OFFSET_IER);
		if (ier & SERIAL_IER_THRE){
			ier &= ~SERIAL_IER_THRE; // clear THRE bit
			outb(dev->port+SERIAL_OFFSET_IER, ier);
		}
	}
}

// Process MSR: Modem Service Register bits changed interrupt
static void processUpdatedMSR(UARTDevice* dev){
	// Served "by reading MSR"
	// We need to read it in order to serve the interrupt,
	// but we don't do anything with it

	// uint8_t msr = inb(dev->port+SERIAL_OFFSET_MSR);
	inb(dev->port+SERIAL_OFFSET_MSR);
}

// Process an updated LSR interrupt
static void processUpdatedLSR(UARTDevice* dev){
	// Served "by reading the LSR"

	uint8_t lsr = inb(dev->port+SERIAL_OFFSET_LSR);

	// if (lsr & SERIAL_LSR_DR); // Data to be read, ignore (handled by interrupts)
	if (lsr & SERIAL_LSR_OE) log(ERROR, MODULE, "Device %d: Data was lost", dev->identifier);
	if (lsr & SERIAL_LSR_PE) log(ERROR, MODULE, "Device %d: Parity error detected", dev->identifier);
	if (lsr & SERIAL_LSR_FE) log(ERROR, MODULE, "Device %d: Framing error (missing stop bit) detected", dev->identifier);
	if (lsr & SERIAL_LSR_BI) log(WARNING, MODULE, "Device %d: got a break (this is unsupported)");
	// if (lsr & SERIAL_LSR_THRE); // Transmission buffer empty, ignore (handled by interrupts)
	// if (lsr & SERIAL_LSR_TEMT); // Transmitter is not doing anything, ignore (handled by interrupts)
	if (lsr & SERIAL_LSR_IE) log(ERROR, MODULE, "Device %d: Error with a word in the input buffer", dev->identifier);
}

static void handleDeviceInterrupt(UARTDevice* dev){
	if (!dev->present) return; // interrupt might get triggered by software

	uint8_t iir;

	// Service interrupts while bit 0 is clear
	do {
		// Read IRR, only keep interrupt bits (lower 3 bits)
		iir = inb(dev->port+SERIAL_OFFSET_IIR) & 0b00001111;

		switch (iir){
		case SERIAL_IIR_INT_PENDING:
			// No more interrupt to handle
			break;
		case SERIAL_IIR_INT_MODEM:
			// MSR bits changed interrupt (priority: lowest)
			processUpdatedMSR(dev);
			break;
		case SERIAL_IIR_INT_TRANSMITTER:
			// THRE: Transmitter Holding Register Empty (priority: third)
			processTHRE(dev);
			break;
		case SERIAL_IIR_INT_DATA:
			// DR or trigger level reached (priority: second)
			processAvailableData(dev);
			break;

		case SERIAL_IIR_INT_LINE:
			// OE, PE, FE or BI of the LSR set (priority: highest)
			processUpdatedLSR(dev);
			break;

		case SERIAL_IIR_INT_FIFO:
			// No receiver FIFO action since 4 words' time but data in RX-FIFO (priority: second)
			processAvailableData(dev);
			break;

		default:
			log(WARNING, MODULE, "IRQ: Invalid or unsupported IRR value");
			break;
		}
	} while (iir != SERIAL_IIR_INT_PENDING);
}

static void handleInterrupt(ISR_Params* params){
	// Determine which device sent the interrupt

	// For all present devices, trigger IRQ handling
	// If no IRQ to be reported, the handler just returns
	for(int i=0 ; i<N_PORTS ; i++){
		if (m_devices[i].present){
			handleDeviceInterrupt(&m_devices[i]);
		}
	}
}

// ================ Public API ================

/// @brief Test if a UART controller is present, and return which controller it is
static UARTController detectUARTController(int port){
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

static bool initalizeUARTController(int port){
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

static void enableDeviceInterrupts(int port){
	// Enable everything but SERIAL_IER_THRE
	// This way, the THRE interrupt will be triggered every time we
	// set its bit in IER ; we use it to trigger writing to the internal
	// buffer when we filled the external buffer

	uint8_t val = SERIAL_IER_DR|SERIAL_IER_LINE|SERIAL_IER_MODEM;
	outb(port+SERIAL_OFFSET_IER, val);
}

void Serial_initalize(){
	int nDevices = 0;

	// Initialize UART Controllers, Serial Devices (if present), and set m_devices accordingly
	for(int i=0 ; i<N_PORTS ; i++){
		UARTDevice* curDev = m_devices+i;

		curDev->identifier = i+1;
		curDev->port = PORTS[i];
		curDev->controller = detectUARTController(curDev->port);
		curDev->controllerName = UARTControllersNames[curDev->controller];
		if (curDev->controller == UART_NONE) continue;
		curDev->internalBufferSize = (curDev->controller == UART_16550A) ? 14 : 1;
		curDev->writeBeginIndex = 0;
		curDev->inWriteBuffer = 0;

		curDev->present = initalizeUARTController(curDev->port);
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
		m_enabled = false;
		return;
	}

	IRQ_registerHandler(3, handleInterrupt); // IRQ3: COM2 or COM4 port
	IRQ_registerHandler(4, handleInterrupt); // IRQ4: COM1 or COM3 port
	for(int i=0 ; i<N_PORTS ; i++){
		if (m_devices[i].present){
			enableDeviceInterrupts(m_devices[i].port);
		}
	}

	m_enabled = true;
	log(SUCCESS, MODULE, "Initialized %d Serial device(s)", nDevices);
	log(INFO, MODULE, "Driver defaults to Serial device %d", m_defaultDevice+1);
}

bool Serial_isEnabled(){
	return m_enabled;
}

void Serial_sendByte(int device, uint8_t byte){
	if (!m_enabled || device<=0 || device>N_PORTS) return;

	unsigned char toSend[] = { byte, '\0' };
	bool res = pushBackWriteBuffer(&m_devices[device-1], toSend);

	if (!res)
		log(WARNING, MODULE, "Serial_sendByte: Couldn't send, buffer is full");
}

void Serial_sendString(int device, const char* str){
	if (!m_enabled || device<=0 || device>N_PORTS) return;

	bool res = pushBackWriteBuffer(&m_devices[device-1], (uint8_t*) str);

	if (!res)
		log(WARNING, MODULE, "Serial_sendString: Couldn't send, not enough room in buffer or invalid string");
}

uint8_t Serial_receiveByte(int device){
	if (!m_enabled || device<=0 || device>N_PORTS) return 0;

	return popFrontReadBuffer(&m_devices[device-1]);
}

void Serial_sendByteDefault(uint8_t byte){
	Serial_sendByte(m_devices[m_defaultDevice].identifier, byte);
}

void Serial_sendStringDefault(const char* str){
	Serial_sendString(m_devices[m_defaultDevice].identifier, str);
}

uint8_t Serial_receiveByteDefault(){
	return Serial_receiveByte(m_devices[m_defaultDevice].identifier);
}
