#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stdio.h"
#include "assert.h"
#include "Arch/io.h"
#include "Arch/PS2Controller.h"
#include "Drivers/Keycodes.h"
#include "Drivers/Keyboard.h"

#include "PS2.h"

// https://wiki.osdev.org/PS/2_Keyboard
// ASCII table: https://www.asciitable.com/

// Note 1: we support only keyboard on port 1 and mouse on port 2
// This is similar to other OSes
// Note 2: we assume that the PS/2 Controller disabled translation for port 1

// Commands
#define PS2_KB_CMD_SET_LED				0xed
#define PS2_KB_CMD_ECHO					0xee
#define PS2_KB_CMD_SET_KEYSET			0xf0 // Get/Set scancode set
#define PS2_KB_CMD_IDENTIFY				0xf2
#define PS2_KB_CMD_SET_DELAY_AND_RATE	0xf3
#define PS2_KB_CMD_ENABLE_SCANNING		0xf4
#define PS2_KB_CMD_DISABLE_SCANNING		0xf5
#define PS2_KB_CMD_SET_DEFAULT_PARAMS	0xf6
#define PS2_KB_CMD_RESEND				0xfe // Resend last byte
#define PS2_KB_CMD_RESET				0xff
// Commands data byte
#define PS2_KB_DATA_SCANCODE_GET		0x00
#define PS2_KB_DATA_SCANCODE_SET1		0x01
#define PS2_KB_DATA_SCANCODE_SET2		0x02
#define PS2_KB_DATA_SCANCODE_SET3		0x03
#define PS2_KB_DATA_LED_SCROLLLOCK		0b00000001
#define PS2_KB_DATA_LED_NUMLOCK			0b00000010
#define PS2_KB_DATA_LED_CAPSLOCK		0b00000100
// Special Bytes responses
#define PS2_KB_RES_ERROR0				0x00
#define PS2_KB_RES_SELF_TEST_PASSED		0xaa
#define PS2_KB_RES_ECHO					0xee
#define PS2_KB_RES_ACK					0xfa
#define PS2_KB_RES_SELF_TEST_FAILED0	0xfc
#define PS2_KB_RES_SELF_TEST_FAILED1	0xfd
#define PS2_KB_RES_RESEND				0xfe
#define PS2_KB_RES_ERROR1				0xff // Same as KB_RESPONSE_ERROR0, other scan code

#define IDENTIFIER_SIZE 64
typedef struct s_PS2Device {
	bool enabled;
	const char* identifier;
	// char identifier[IDENTIFIER_SIZE];
} PS2Device;

static bool g_enabled;
static PS2Device g_PS2Keyboard;
static PS2Device g_PS2Mouse;

// We need these keyboard-related definitions here for the PS/2 driver intialization
static void setLEDs(uint8_t leds);
static uint8_t g_leds = 0; // The low 3 bits represent each led's status

#pragma region PS/2 Initalize

// Send a byte to device 1, and resends (max 3 times) if response is Resend
// Returns: response on success, PS2_KB_RES_RESEND on failure
static inline uint8_t sendByteToDevice1_HandleResend(uint8_t byte){
	uint8_t buff;
	for(int i=0 ; i<3 ; i++){
		bool success;
		success = PS2Controller_sendByteToDevice1(byte);
		if (!success) return PS2_KB_RES_RESEND;
		success = PS2Controller_receiveDeviceByte(&buff);
		if (!success) return PS2_KB_RES_RESEND;
		if (buff != PS2_KB_RES_RESEND) break;
	}
	return buff;
}

// Send a byte to device 2, and resends (max 3 times) if response is Resend
// Returns: response on success, PS2_KB_RES_RESEND on failure
static inline uint8_t sendByteToDevice2_HandleResend(uint8_t byte){
	puts("TODO");
	return PS2_KB_RES_RESEND;
}

// Detect the device given by the first argument.
// Returns:
//  - Type of device as a constant string
//  - Weather the device is a keyboard in isKeyboard_out
// If the device is invalid, returns NULL (and isKeyboard_out is undefined)
static const char* detectDevice(int device, bool* isKeyboard_out){
	assert(device == 1 || device == 2);
	uint8_t buff;
	uint8_t (*sendByteToDevice)(uint8_t);
	sendByteToDevice = (device == 1) ? sendByteToDevice1_HandleResend : sendByteToDevice2_HandleResend;

	buff = sendByteToDevice(PS2_KB_CMD_DISABLE_SCANNING);
	if (buff != PS2_KB_RES_ACK) return NULL;

	buff = sendByteToDevice(PS2_KB_CMD_IDENTIFY);
	if (buff != PS2_KB_RES_ACK) return NULL;

	uint8_t byte1, byte2;
	bool byte1_present = PS2Controller_receiveDeviceByte(&byte1);
	if (!byte1_present){
		*isKeyboard_out = true;
		return "AT Keyboard";
	}

	bool byte2_present = PS2Controller_receiveDeviceByte(&byte2);

	// Only one byte to handle
	if (!byte2_present){
		*isKeyboard_out = false;
		switch (byte1){
			case 0x00: return "Standard PS/2 mouse";
			case 0x03: return "PS/2 Mouse with scroll wheel";
			case 0x04: return "PS/2 5-button mouse";
			default: return NULL;
		}
	}

	// 2 bytes to handle
	*isKeyboard_out = true;
	switch (byte1){
		case 0xab:
			switch (byte2){
				case 0x83: return "PS/2 MF2 keyboard";
				case 0xc1: return "PS/2 MF2 keyboard";
				case 0x84: return "PS/2 short keyboard";
				case 0x85: return "PS/2 NCD N-97 keyboard";
				case 0x86: return "PS/2 122-key keyboard";
				case 0x90: return "PS/2 Japanse 'G' Keyboard";
				case 0x91: return "PS/2 Japanse 'P' Keyboard";
				case 0x92: return "PS/2 Japanse 'A' Keyboard";
				default: return NULL;
			}
		case 0xac:
			switch (byte2){
				case 0xa1: return "PS/2 NCD Sun Keyboard";
				default: return NULL;
			}
		default: return NULL;
	}
}

// Note: check that port 1 is available and populated before calling this method
void PS2_initalizeKeyboard(PS2Device* keyboard){
	bool isKeyboard;
	uint8_t buff1, buff2;
	keyboard->enabled = false;

	keyboard->identifier = detectDevice(1, &isKeyboard);
	if ((keyboard->identifier == NULL) || !isKeyboard){
		return;
	}

	// Switch to scan code set 2
	buff1 = sendByteToDevice1_HandleResend(PS2_KB_CMD_SET_KEYSET);
	buff2 = sendByteToDevice1_HandleResend(PS2_KB_DATA_SCANCODE_SET2);
	if (buff1!=PS2_KB_RES_ACK || buff2!=PS2_KB_RES_ACK){
		puts("[ INFO ] PS/2 driver: PS/2 Keyboard doesn't support scan code set 1, deactivated support.");
		keyboard->enabled = false;
		return;
	}

	// Set typematic byte (repeat rate/delay) NOTE: DOESN'T SEEM TO WORK
	// Search x number to send for a wanted rate = -28/31*x + 30
	// Delay: 0b00=250ms 0b01=500ms 0b10=750ms 0b11=1000ms
	sendByteToDevice1_HandleResend(PS2_KB_CMD_SET_DELAY_AND_RATE);
	sendByteToDevice1_HandleResend(0b00000000 | 0b00010110); // Default 10.9 Hz (22) + 500 ms repeat

	// Set LEDs: by default, only NUMLOCK is set
	g_leds = PS2_KB_DATA_LED_NUMLOCK;
	setLEDs(g_leds);

	keyboard->enabled = true;
	printf("[ INFO ] PS/2 driver: Indentified keyboard as '%s'\n", keyboard->identifier);
}

// Note: check that port 2 is available and populated before calling this method
void PS2_initalizeMouse(PS2Device* mouse){
	mouse->enabled = false;
	puts("[ TODO ] PS/2 driver: PS/2 mouse not implemented (in " __FILE__ ")");
}

void PS2_initalize(){
	PS2Controller_initialize();
	PS2Controller_disableDevicesInterrupts();

	bool enabled, port1_enabled, port2_enabled;
	PS2Controller_getStatus(&enabled, &port1_enabled, &port2_enabled);
	if (!enabled){
		g_enabled = false;
		g_PS2Keyboard.enabled = false;
		g_PS2Mouse.enabled = false;
		puts("[ERROR!] PS/2 driver: Initalization failed, PS/2 Controller driver is disabled");
		return;
	}

	g_enabled = true;
	if (port1_enabled) PS2_initalizeKeyboard(&g_PS2Keyboard);
	if (port2_enabled) PS2_initalizeMouse(&g_PS2Mouse);

	// After intialization, we can re-enable scanning
	uint8_t buff;
	if (g_PS2Keyboard.enabled) {
		buff = sendByteToDevice1_HandleResend(PS2_KB_CMD_ENABLE_SCANNING);
		if (buff != PS2_KB_RES_ACK) g_PS2Keyboard.enabled = false;
	}
	if (g_PS2Mouse.enabled) {
		buff = sendByteToDevice2_HandleResend(PS2_KB_CMD_ENABLE_SCANNING);
		if (buff != PS2_KB_RES_ACK) g_PS2Mouse.enabled = false;
	}

	PS2Controller_enableDevicesInterrupts();
	puts("[  OK  ] PS/2 driver: Initialization success");
}

#pragma endregion PS/2 Initalize

#pragma region PS/2 Keyboard

#define PS2_KB_SCANCODE_ESCAPE					0xe0
#define PS2_KB_SCANCODE_ESCAPE_PAUSE			0xe1
#define PS2_KB_SCANCODE_BREAK					0xf0
#define PS2_KB_SCANCODE_SYSRQ					0x84

// This keyboard driver is implemented as a state machine
// These are its states
static bool g_isEscapedState = false;
static bool g_isBreakedState = false;
static int g_pressedPrintScreenSequence = 0; // index in the pressed print screen sequence
static int g_releasedPrintScreenSequence = 0; // index in the released print screen sequence
static int g_pauseSequence = 0; // index in the pause sequence (pause has no release)

static inline void setLEDs(uint8_t leds){
	// Note: we ignore errors for the sake of simplicity
	sendByteToDevice1_HandleResend(PS2_KB_CMD_SET_LED);
	sendByteToDevice1_HandleResend(leds);
}

static inline void resetKeyboardState(){
	g_isEscapedState = false;
	g_isBreakedState = false;
	g_pressedPrintScreenSequence = 0;
	g_releasedPrintScreenSequence = 0;
	g_pauseSequence = 0;
}

// Returns the universal keycode corresponding to the provided PS/2 scancode.
// Returns KEY_RESERVED on unrecognized keycode
static Keycode getKeycode(uint8_t scancode){
	switch (scancode){
		case 0x01: return KEY_F9;
		case 0x03: return KEY_F5;
		case 0x04: return KEY_F3;
		case 0x05: return KEY_F1;
		case 0x06: return KEY_F2;
		case 0x07: return KEY_F12;
		case 0x09: return KEY_F10;
		case 0x0a: return KEY_F8;
		case 0x0b: return KEY_F6;
		case 0x0c: return KEY_F4;
		case 0x0d: return KEY_TAB;
		case 0x0e: return KEY_GRAVE;
		case 0x11: return KEY_LALT;
		case 0x12: return KEY_LSHIFT;
		case 0x14: return KEY_LCTRL;
		case 0x15: return KEY_Q;
		case 0x16: return KEY_1;
		case 0x1a: return KEY_Z;
		case 0x1b: return KEY_S;
		case 0x1c: return KEY_A;
		case 0x1d: return KEY_W;
		case 0x1e: return KEY_2;
		case 0x21: return KEY_C;
		case 0x22: return KEY_X;
		case 0x23: return KEY_D;
		case 0x24: return KEY_E;
		case 0x25: return KEY_4;
		case 0x26: return KEY_3;
		case 0x29: return KEY_SPACE;
		case 0x2a: return KEY_V;
		case 0x2b: return KEY_F;
		case 0x2c: return KEY_T;
		case 0x2d: return KEY_R;
		case 0x2e: return KEY_5;
		case 0x31: return KEY_N;
		case 0x32: return KEY_B;
		case 0x33: return KEY_H;
		case 0x34: return KEY_G;
		case 0x35: return KEY_Y;
		case 0x36: return KEY_6;
		case 0x3a: return KEY_M;
		case 0x3b: return KEY_J;
		case 0x3c: return KEY_U;
		case 0x3d: return KEY_7;
		case 0x3e: return KEY_8;
		case 0x41: return KEY_COMMA;
		case 0x42: return KEY_K;
		case 0x43: return KEY_I;
		case 0x44: return KEY_O;
		case 0x45: return KEY_0;
		case 0x46: return KEY_9;
		case 0x49: return KEY_DOT;
		case 0x4a: return KEY_SLASH;
		case 0x4b: return KEY_L;
		case 0x4c: return KEY_SEMICOLON;
		case 0x4d: return KEY_P;
		case 0x4e: return KEY_MINUS;
		case 0x52: return KEY_QUOTE;
		case 0x54: return KEY_LBRACKET;
		case 0x55: return KEY_EQUAL;
		case 0x58: return KEY_CAPSLOCK;
		case 0x59: return KEY_RSHIFT;
		case 0x5a: return KEY_ENTER;
		case 0x5b: return KEY_RBRACKET;
		case 0x5d: return KEY_BACKSLASH;
		case 0x61: return KEY_LESSTHAN;
		case 0x66: return KEY_BACKSPACE;
		case 0x69: return KEY_NUMPAD_1;
		case 0x6b: return KEY_NUMPAD_4;
		case 0x6c: return KEY_NUMPAD_7;
		case 0x70: return KEY_NUMPAD_0;
		case 0x71: return KEY_NUMPAD_DOT;
		case 0x72: return KEY_NUMPAD_2;
		case 0x73: return KEY_NUMPAD_5;
		case 0x74: return KEY_NUMPAD_6;
		case 0x75: return KEY_NUMPAD_8;
		case 0x76: return KEY_ESC;
		case 0x77: return KEY_NUMLOCK;
		case 0x78: return KEY_F11;
		case 0x79: return KEY_NUMPAD_PLUS;
		case 0x7a: return KEY_NUMPAD_3;
		case 0x7b: return KEY_NUMPAD_MINUS;
		case 0x7c: return KEY_NUMPAD_ASTERISK;
		case 0x7d: return KEY_NUMPAD_9;
		case 0x7e: return KEY_SCROLLLOCK;
		case 0x83: return KEY_F7;

		default:
			return KEY_RESERVED;
	}
}

// Returns the universal keycode corresponding to the provided PS/2 scancode.
// This is in the case the keyscan was escaped
static Keycode getKeycodeEscaped(uint8_t scancode){
	switch (scancode){
		case 0x10: return KEY_WWW; // "(multimedia) WWW search";
		case 0x11: return KEY_RALT;
		case 0x14: return KEY_RCTRL;
		case 0x15: return KEY_PREVIOUSSONG; // (multimedia) previous track
		case 0x1f: return KEY_LMETA;
		case 0x20: return KEY_REFRESH; // (multimedia) WWW refresh
		case 0x21: return KEY_VOLUMEDOWN;
		case 0x23: return KEY_MUTE;
		case 0x27: return KEY_RMETA;
		case 0x28: return KEY_STOP; // (multimedia) WWW stop
		case 0x2b: return KEY_CALC;
		case 0x2f: return KEY_MENU;
		case 0x30: return KEY_FORWARD; // (multimedia) WWW forward
		case 0x32: return KEY_VOLUMEUP;
		case 0x34: return KEY_PLAYPAUSE;
		case 0x37: return KEY_POWER;
		case 0x38: return KEY_BACK; // (multimedia) WWW back
		case 0x3b: return KEY_STOP;
		case 0x3f: return KEY_SLEEP;
		case 0x48: return KEY_EMAIL;
		case 0x4a: return KEY_NUMPAD_SLASH;
		case 0x4d: return KEY_NEXTSONG;
		case 0x50: return KEY_MEDIA;
		case 0x5a: return KEY_NUMPAD_ENTER;
		case 0x5e: return KEY_WAKEUP;
		case 0x69: return KEY_END;
		case 0x6b: return KEY_LEFT;
		case 0x6c: return KEY_HOME;
		case 0x70: return KEY_INSERT;
		case 0x71: return KEY_DELETE;
		case 0x72: return KEY_DOWN;
		case 0x74: return KEY_RIGHT;
		case 0x75: return KEY_UP;
		case 0x7a: return KEY_PAGEDOWN;
		case 0x7d: return KEY_PAGEUP;

		default:
			return KEY_RESERVED;
	}
}

// Returns the new pauseSequence index depending on scancode ; 0x7fffffff if sequence was completed
static int cyclePauseSequence(int pauseSequenceIndex, uint8_t scancode){
	assert(pauseSequenceIndex > 0);

	// Check for Pause
	// SEQUENCE=[0xe1,0x14,0x77,0xe1,0xf0,0x14,0xf0,0x77]

	switch (pauseSequenceIndex){
		case 1:
			if (scancode != 0x14) break;
			return pauseSequenceIndex + 1;
		case 2:
			if (scancode != 0x77) break;
			return pauseSequenceIndex + 1;
		case 3:
			if (scancode != 0xe1) break;
			return pauseSequenceIndex + 1;
		case 4:
			if (scancode != 0xf0) break;
			return pauseSequenceIndex + 1;
		case 5:
			if (scancode != 0x14) break;
			return pauseSequenceIndex + 1;
		case 6:
			if (scancode != 0xf0) break;
			return pauseSequenceIndex + 1;
		case 7:
			if (scancode != 0x77) break;
			return 0x7fffffff;
		default:
			return 0;
	}

	return 0;
}

static inline bool isResponseCode(uint8_t code){
	if ((code == PS2_KB_RES_ERROR0)
		|| (code == PS2_KB_RES_SELF_TEST_PASSED)
		|| (code == PS2_KB_RES_ECHO)
		|| (code == PS2_KB_RES_ACK)
		|| (code == PS2_KB_RES_SELF_TEST_FAILED0)
		|| (code == PS2_KB_RES_SELF_TEST_FAILED1)
		|| (code == PS2_KB_RES_RESEND)
		|| (code == PS2_KB_RES_ERROR1)){
		return true;
	}
	return false;
}

static void handleScancode(uint8_t scancode){
	// Check for Print Screen sequence PRESSED=[0xe0,0x12,0xe0,0x7c]
	if (g_pressedPrintScreenSequence>0){
		switch (g_pressedPrintScreenSequence) {
			case 1:
				if (scancode != 0x12) {g_pressedPrintScreenSequence = 0; break;}
				g_pressedPrintScreenSequence++;
				g_isEscapedState = false;
				return;
			case 2:
				if (scancode != 0xe0) {g_pressedPrintScreenSequence = 0; break;}
				g_pressedPrintScreenSequence++;
				return;
			case 3:
				if (scancode != 0x7c) {g_pressedPrintScreenSequence = 0; break;}
				Keyboard_notifyPressed(KEY_PRINTSCREEN);
				g_pressedPrintScreenSequence = 0;
				return;
			default:
				break;
		}
	}
	// Check for Print Screen sequence RELEASED=[0xe0,0xf0,0x7c,0xe0,0xf0,0x12]
	if (g_releasedPrintScreenSequence>0){
		switch (g_releasedPrintScreenSequence) {
			case 1:
				if (scancode != 0xf0) {g_releasedPrintScreenSequence = 0; break;}
				g_releasedPrintScreenSequence++;
				g_isBreakedState = true;
				break;
			case 2:
				if (scancode != 0x7c) {g_releasedPrintScreenSequence = 0; break;}
				g_releasedPrintScreenSequence++;
				return;
			case 3:
				if (scancode != 0xe0) {g_releasedPrintScreenSequence = 0; g_isEscapedState = false; g_isBreakedState = false; break;}
				g_releasedPrintScreenSequence++;
				return;
			case 4:
				if (scancode != 0xf0) {g_releasedPrintScreenSequence = 0; break;}
				g_releasedPrintScreenSequence++;
				return;
			case 5:
				if (scancode != 0x12) {g_releasedPrintScreenSequence = 0; break;}
				Keyboard_notifyReleased(KEY_PRINTSCREEN);
				g_releasedPrintScreenSequence = 0;
				g_isEscapedState = false;
				g_isBreakedState = false;
				return;
			default:
				break;
		}
	}

	// Check for Pause sequence
	if (g_pauseSequence > 0){
		g_pauseSequence = cyclePauseSequence(g_pauseSequence, scancode);
		// Sequence finished
		if (g_pauseSequence == 0x7fffffff){
			g_pauseSequence = 0;
			Keyboard_notifyPressed(KEY_PAUSE);
		}
		return;
	}

	if (scancode == PS2_KB_SCANCODE_ESCAPE) {
		g_isEscapedState = true;
		g_pressedPrintScreenSequence = 1;
		g_releasedPrintScreenSequence = 1;
		return;
	}
	if (scancode == PS2_KB_SCANCODE_ESCAPE_PAUSE) {
		g_pauseSequence = 1;
		return;
	}
	if (scancode == PS2_KB_SCANCODE_BREAK) {
		g_isBreakedState = true;
		return;
	}

	if (scancode == PS2_KB_SCANCODE_SYSRQ){
		if (g_isBreakedState) return; // do not notify on release
		Keyboard_NotifySysRq();
		return;
	}

	Keycode keycode;
	if (g_isEscapedState) {
		keycode = getKeycodeEscaped(scancode);
		g_isEscapedState = false;
	}
	else keycode = getKeycode(scancode);

	switch (keycode){
		case KEY_RESERVED: // Driver did not recognize the scancode
			printf("Unrecognized scancode %p\n", scancode);
			resetKeyboardState();
			return;
		case KEY_NUMLOCK:
			if(g_isBreakedState) break;
			g_leds ^= PS2_KB_DATA_LED_NUMLOCK; // flip numlock bit
			setLEDs(g_leds);
			break;
		case KEY_SCROLLLOCK:
			if(g_isBreakedState) break;
			g_leds ^= PS2_KB_DATA_LED_SCROLLLOCK;
			setLEDs(g_leds);
			break;
		case KEY_CAPSLOCK:
			if(g_isBreakedState) break;
			g_leds ^= PS2_KB_DATA_LED_CAPSLOCK;
			setLEDs(g_leds);
			break;
		default:
			// Key was recognized
			break;
	}

	// Update the keyboard driver
	(g_isBreakedState) ? Keyboard_notifyReleased(keycode) : Keyboard_notifyPressed(keycode);
	resetKeyboardState();
}

void PS2_notifyKeyboard(){
	if(!g_PS2Keyboard.enabled) return;
	uint8_t code;

	// Debug print
	// code = inb(0x60);
	// if (code == 0x5a) puts("");
	// else printf("%p ", code);
	// return;

	bool res = PS2Controller_receiveDeviceByte(&code);
	// this check should not be needed on x86, but might be
	// on other plateforms ? so it's here just in case
	if (!res) return;

	// Verify that it IS a keycode and not a response from a command (should not happen though)
	if (isResponseCode(code)){
		printf("[ INT  ] Got a reponse byte instead of a keycode, ignoring (byte=%p)\n", code);
		return;
	}

	handleScancode(code);
}

#pragma endregion PS/2 Keyboard

#pragma region PS/2 Mouse

// STUB
void PS2_notifyMouse(){
	uint8_t data = inb(0x60);
	printf("[INT] PS/2 mouse interrupt data=%p\n", data);
}

#pragma endregion PS/2 Mouse
