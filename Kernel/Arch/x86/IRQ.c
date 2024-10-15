#include <stddef.h>
#include "stdio.h"
#include "Panic.h"
#include "IO.h"
#include "ISR.h"
#include "PIC.h"

#include "IRQ.h"

// IRQ 0-7 are mapped to interrupts 32 to 39 ; IRQ 8-15 to interrupts 40 to 47
#define IRQ_MASTER_PIC_REMAP_OFFSET 0x20

// Global array of [un]registered IRQ handlers
IRQHandler g_IRQHandlers[16];

#define IRQ_Timer		0x00
#define IRQ_Keyboard	0x01

static const char* const g_IRQTypes[] = {
    "Programmable Interrupt Timer Interrupt",
    "Keyboard Interrupt",
    "Cascade",											// Used internally by the two PICs, never raised
    "COM2",												// If enabled
    "COM1",												// If enabled
    "LPT2",												// If enabled
    "Floppy Disk",
    "LPT1 / Unreliable 'spurious' interrupt",			// Usually (?)
    "CMOS real-time clock",								// If enabled
    "Free for peripherals/legacy SCSI/NIC",
    "Free for peripherals/legacy SCSI/NIC",
    "Free for peripherals/legacy SCSI/NIC",
    "PS2 Mouse",
    "FPU/Coprocessor/Inter-processor",
    "Primary ATA Hard Disk",
    "Secondary ATA Hard Disk ",
};

void x86_IRQ_RegisterIRQHandler(uint8_t irq, IRQHandler handler){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = handler;
}

void x86_IRQ_DeregisterIRQHandler(uint8_t irq){
	if (irq > 16) return; // ignore invalid IRQ number
	g_IRQHandlers[irq] = NULL;
}

// ================ IRQ Handlers ================

void x86_IRQ_Prehandler(ISR_Params* params){
	// If we have a handler to call, we call it, and 'alles gut'

	uint8_t irq = (uint8_t) params->vector;
	irq = irq - IRQ_MASTER_PIC_REMAP_OFFSET; // clamp irq to 0-15

	if (g_IRQHandlers[irq] != NULL){
		g_IRQHandlers[irq](params);
		x86_PIC_SendEIO(irq);
		return;
	}

	// Otherwise we PANIC !
	const char* interrupt_type = g_IRQTypes[irq];
	printf("Unhandled IRQ number %d - %s\n", irq, interrupt_type);
	printf("\tvector=%p eflags=%p err=%p\n", params->vector, params->eflags, params->err);
	printf("\teax=%p ebx=%p ecx=%p edx=%p esi=%p edi=%p\n",
		params->eax, params->ebx, params->ecx, params->edx, params->esi, params->edi
	);
	printf("\teip=%p esp=%p ebp=%p\n", params->eip, params->esp, params->ebp);
	printf("\tcs=%p ds=%p ss=%p\n", params->cs, params->ds, params->ss);
	PANIC();
}

void x86_IRQ_Timer(ISR_Params* params){
	// Nothing to do yet
}

void x86_IRQ_Keyboard(ISR_Params* params){
	#define KBD_PORT_DATA			0x60
	#define KBD_PORT_STATUS_REG		0x64
	#define KBD_PORT_CMD			0x64
	static bool g_IsEscaped = false;
	
	uint8_t keycode = x86_inb(KBD_PORT_DATA);

	// Keyboard error (error code depends on kb's mode, we don't know yet)
	if (keycode == 0x00 || keycode == 0xff)	{
		puts("Keyboard error");
		return;
	}

	if (keycode == 0xe0 || keycode == 0xe1) {
		// Set flag for incomming escaped IRQ
		g_IsEscaped = true;
		return;
	}

	if (g_IsEscaped){
		g_IsEscaped = false;
		return; // for now we just ignore escaped commands
	}

	if (keycode >= 0x80) return; // key released

	unsigned char chr = 0xff;
	switch (keycode){
		case 0x01: chr = '\0'; break; // échap
		case 0x02: chr = '&'; break;
		case 0x03: chr = 130; break; // é
		case 0x04: chr = '"'; break;
		case 0x05: chr = '\''; break;
		case 0x06: chr = '('; break;
		case 0x07: chr = '-'; break;
		case 0x08: chr = 138; break; // è
		case 0x09: chr = '_'; break;
		case 0x0a: chr = 135; break; // ç
		case 0x0b: chr = 133; break; // à
		case 0x0c: chr = ')'; break;
		case 0x0d: chr = '='; break;
		case 0x0e: chr = '\0'; break; // backspace
		case 0x0f: chr = '\t'; break;
		case 0x10: chr = 'a'; break;
		case 0x11: chr = 'z'; break;
		case 0x12: chr = 'e'; break;
		case 0x13: chr = 'r'; break;
		case 0x14: chr = 't'; break;
		case 0x15: chr = 'y'; break;
		case 0x16: chr = 'u'; break;
		case 0x17: chr = 'i'; break;
		case 0x18: chr = 'o'; break;
		case 0x19: chr = 'p'; break;
		case 0x1a: chr = '^'; break;
		case 0x1b: chr = '$'; break;
		case 0x1c: chr = '\n'; break; // enter
		case 0x1d: chr = '\0'; break; // left ctrl
		case 0x1e: chr = 'q'; break;
		case 0x1f: chr = 's'; break;
		case 0x20: chr = 'd'; break;
		case 0x21: chr = 'f'; break;
		case 0x22: chr = 'g'; break;
		case 0x23: chr = 'h'; break;
		case 0x24: chr = 'j'; break;
		case 0x25: chr = 'k'; break;
		case 0x26: chr = 'l'; break;
		case 0x27: chr = 'm'; break;
		case 0x28: chr = 163; break; // ù
		case 0x29: chr = 253; break; // ²
		case 0x2a: chr = '\0'; break; // left shift
		case 0x2b: chr = '*'; break;
		case 0x2c: chr = 'w'; break;
		case 0x2d: chr = 'x'; break;
		case 0x2e: chr = 'c'; break;
		case 0x2f: chr = 'v'; break;
		case 0x30: chr = 'b'; break;
		case 0x31: chr = 'n'; break;
		case 0x32: chr = ','; break;
		case 0x33: chr = ';'; break;
		case 0x34: chr = ':'; break;
		case 0x35: chr = '!'; break;
		case 0x36: chr = '\0'; break; // right shift
		case 0x37: chr = '*'; break;
		case 0x38: chr = '\0'; break; // left alt
		case 0x39: chr = ' '; break;
		case 0x3a: chr = '\0'; break; // Caps lock
		case 0x3b: chr = '\0'; break; // F1
		case 0x3c: chr = '\0'; break; // F2
		case 0x3d: chr = '\0'; break; // F3
		case 0x3e: chr = '\0'; break; // F4
		case 0x3f: chr = '\0'; break; // F5
		case 0x40: chr = '\0'; break; // F6
		case 0x41: chr = '\0'; break; // F7
		case 0x42: chr = '\0'; break; // F8
		case 0x43: chr = '\0'; break; // F9
		case 0x44: chr = '\0'; break; // F10
		case 0x45: chr = '\0'; break; // Num lock
		case 0x46: chr = '\0'; break; // Scroll lock
		case 0x47: chr = '7'; break; // Keypad 7 OR Home
		case 0x48: chr = '8'; break; // Keypad 8 OR Up
		case 0x49: chr = '9'; break; // Keypad 9 OR Page Up
		case 0x4a: chr = '-'; break; // Keypad -
		case 0x4b: chr = '4'; break; // Keypad 4 OR Left
		case 0x4c: chr = '5'; break; // Keypad 5
		case 0x4d: chr = '6'; break; // Keypad 6 OR Right
		case 0x4e: chr = '+'; break; // Keypad +
		case 0x4f: chr = '1'; break; // Keypad 1 OR End
		case 0x50: chr = '2'; break; // Keypad 2 OR Down
		case 0x51: chr = '3'; break; // Keypad 3 OR Page Down
		case 0x52: chr = '0'; break; // Keypad 0 OR Insert
		case 0x53: chr = '.'; break; // Keypad . OR Del
		case 0x54: chr = '\0'; break; // Alt-SysRq
		// case 0x55: chr = '\0'; break; // ?
		case 0x56: chr = '<'; break;
		case 0x57: chr = '\0'; break; // F11
		case 0x58: chr = '\0'; break; // F12
		// case 0x59: chr = '\0'; break;
		// case 0x5a: chr = '\0'; break;
		case 0x5b: chr = '\0'; break; // Windows/Meta
		// case 0x5c: chr = '\0'; break;
		case 0x5d: chr = '\0'; break; // Menu (right click key)
	}

	// recognized character, non printable
	if (chr == '\0') return;

	// Switch did not recognize keycode
	if (chr == 0xff) { printf("%p ", keycode); return; }

	putc(chr);

	// x86_outb(KBD_PORT_STATUS_REG, 0xfe); // reset signal
}

void x86_IRQ_Initialize(){
	// Remap the PIC
	x86_PIC_Remap(IRQ_MASTER_PIC_REMAP_OFFSET, IRQ_MASTER_PIC_REMAP_OFFSET+8);

	// Register our IRQ Pre-handler
	for(int i=0 ; i<16 ; i++){
		x86_ISR_RegisterHandler(IRQ_MASTER_PIC_REMAP_OFFSET+i, x86_IRQ_Prehandler);
	}

	// Register our IRQ handlers
	x86_IRQ_RegisterIRQHandler(IRQ_Timer, x86_IRQ_Timer);
	x86_IRQ_RegisterIRQHandler(IRQ_Keyboard, x86_IRQ_Keyboard);

	x86_PIC_EnableAllIRQ();
}
