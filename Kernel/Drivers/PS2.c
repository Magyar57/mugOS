#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "string.h"
#include "assert.h"
#include "io.h"
#include "Logging.h"
#include "Drivers/PS2Controller.h"
#include "Drivers/Keycodes.h"
#include "Drivers/Keyboard.h"

#include "PS2.h"

#define MODULE "PS/2 driver"

// https://wiki.osdev.org/PS/2_Keyboard
// ASCII table: https://www.asciitable.com/

// Note 1: we support only keyboard on port 1 and mouse on port 2
// This is similar to other OSes
// Note 2: we assume that the PS/2 Controller disabled translation for port 1

// Commands (prefixes: KB=Keyboard, MOUSE=Mouse, None=Both)
#define PS2_KB_CMD_SET_LED				0xed
#define PS2_KB_CMD_ECHO					0xee
#define PS2_KB_CMD_SET_KEYSET			0xf0 // Get/Set scancode set
#define PS2_CMD_IDENTIFY				0xf2
#define PS2_KB_CMD_SET_DELAY_AND_RATE	0xf3
#define PS2_KB_CMD_ENABLE_SCANNING		0xf4
#define PS2_KB_CMD_DISABLE_SCANNING		0xf5
#define PS2_KB_CMD_SET_DEFAULT_PARAMS	0xf6
#define PS2_KB_CMD_RESEND				0xfe // Resend last byte
#define PS2_KB_CMD_RESET				0xff
#define PS2_MOUSE_CMD_SET_SAMPLERATE	0xf3
#define PS2_MOUSE_CMD_ENABLE_STREAMING	0xf4 // Automatic packets
#define PS2_MOUSE_CMD_DISABLE_STREAMING	0xf5 // Automatic packets
#define PS2_MOUSE_CMD_SET_RESOLUTION	0xe8
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

struct PS2Keyboard {
	bool enabled;
	const char* name;
} PS2Keyboard;

enum PS2MouseType {
	PS2MOUSETYPE_STD = 0x00,
	PS2MOUSETYPE_WHEEL = 0x03,
	PS2MOUSETYPE_5BUTTONS = 0x04
};

struct PS2Mouse {
	bool enabled;
	enum PS2MouseType type;
	const char* name;
	uint8_t packetSize;
};

static bool m_enabled;
static struct PS2Keyboard m_PS2Keyboard;
static struct PS2Mouse m_PS2Mouse;

// We need these keyboard-related definitions here for the PS/2 driver intialization
static void setLEDs(uint8_t leds);
static uint8_t g_leds = 0; // The low 3 bits represent each led's status

#pragma region PS/2 Initialize

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
	uint8_t buff;
	for(int i=0 ; i<3 ; i++){
		bool success;
		success = PS2Controller_sendByteToDevice2(byte);
		if (!success) return PS2_KB_RES_RESEND;
		success = PS2Controller_receiveDeviceByte(&buff);
		if (!success) return PS2_KB_RES_RESEND;
		if (buff != PS2_KB_RES_RESEND) break;
	}
	return buff;
}

// Get a keyboard name/type (scanning must be disabled !!)
// Returns its name (NULL if unrecognized or is a mouse)
static const char* getKeyboardName(){
	uint8_t buff, byte1, byte2;

	buff = sendByteToDevice1_HandleResend(PS2_CMD_IDENTIFY);
	if (buff != PS2_KB_RES_ACK) return NULL;

	bool byte1_present = PS2Controller_receiveDeviceByte(&byte1);
	if (!byte1_present){
		return "AT Keyboard";
	}

	bool byte2_present = PS2Controller_receiveDeviceByte(&byte2);

	// Only one byte: mouse
	if (!byte2_present){
		return NULL;
	}

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
		default:
			return NULL;
	}
}

// Detect a mouse on port 2 and returns its id (0xff if unrecognized or is a keyboard)
static enum PS2MouseType getMouseType(){
	uint8_t buff;

	buff = sendByteToDevice2_HandleResend(PS2_CMD_IDENTIFY);
	if (buff != PS2_KB_RES_ACK) return 0xff;

	bool byte_present = PS2Controller_receiveDeviceByte(&buff);
	if (!byte_present) return 0xff;

	return buff;
}

// Note: check that port 1 is available and populated before calling this method
void PS2_initializeKeyboard(struct PS2Keyboard* keyboard){
	uint8_t buff1, buff2;
	keyboard->enabled = false;

	// Disable scanning for initialization
	buff1 = sendByteToDevice1_HandleResend(PS2_KB_CMD_DISABLE_SCANNING);
	if (buff1 != PS2_KB_RES_ACK) return;

	// Get the keyboard name string
	keyboard->name = getKeyboardName();
	if (keyboard->name == NULL)	return;
	keyboard->enabled = true;

	// Switch to scan code set 2
	buff1 = sendByteToDevice1_HandleResend(PS2_KB_CMD_SET_KEYSET);
	buff2 = sendByteToDevice1_HandleResend(PS2_KB_DATA_SCANCODE_SET2);
	if (buff1!=PS2_KB_RES_ACK || buff2!=PS2_KB_RES_ACK){
		Logging_log(INFO, MODULE, "PS/2 Keyboard doesn't support scan code set 2, deactivated.");
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

	Logging_log(INFO, MODULE, "Indentified keyboard as '%s'", keyboard->name);
}

// Note: check that port 2 is available and populated before calling this method
void PS2_initializeMouse(struct PS2Mouse* mouse){
	mouse->enabled = false;

	mouse->type = getMouseType();

	// Try to enable scroll wheel
	// Magic sequence: set sample rate to 200, then 100, then 80
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(200);
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(100);
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(80);
	mouse->type = getMouseType(); // update mouse type

	// Try to enable mouse button 4 and 5
	// Magic sequence: set sample rate to 200, then 200, then 80
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(200);
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(200);
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(80);
	mouse->type = getMouseType(); // update mouse type

	// Set Samplerate and Resolution
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDevice2_HandleResend(40); // 40 samples/sec (as recommended by osdev.org)
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_SET_RESOLUTION);
	sendByteToDevice2_HandleResend(0x03); // 8 count/mm

	// Enable packets
	sendByteToDevice2_HandleResend(PS2_MOUSE_CMD_ENABLE_STREAMING);

	// Set the mouse name
	switch (mouse->type){
		case PS2MOUSETYPE_STD:
			mouse->name = "Standard PS/2 mouse";
			mouse->packetSize = 3;
			break;
		case PS2MOUSETYPE_WHEEL:
			mouse->name = "PS/2 Mouse with scroll wheel";
			mouse->packetSize = 4;
			break;
		case PS2MOUSETYPE_5BUTTONS:
			mouse->name = "PS/2 5-button mouse";
			mouse->packetSize = 4;
			break;
		default:
			Logging_log(INFO, MODULE, "Couldn't initialize PS/2 mouse (invalid mouse id)");
			return;
	}

	mouse->enabled = true;
	Logging_log(INFO, MODULE, "Identified mouse as '%s'", mouse->name);
}

void PS2_initialize(){
	PS2Controller_initialize();
	PS2Controller_disableDevicesInterrupts();

	bool enabled, port1_enabled, port2_enabled;
	PS2Controller_getStatus(&enabled, &port1_enabled, &port2_enabled);
	if (!enabled){
		m_enabled = false;
		m_PS2Keyboard.enabled = false;
		m_PS2Mouse.enabled = false;
		Logging_log(ERROR, MODULE, "Initalization failed, PS/2 Controller driver is disabled");
		return;
	}

	m_enabled = true;
	if (port1_enabled) PS2_initializeKeyboard(&m_PS2Keyboard);
	if (port2_enabled) PS2_initializeMouse(&m_PS2Mouse);

	// After intialization, we can re-enable scanning
	uint8_t buff;
	if (m_PS2Keyboard.enabled) {
		buff = sendByteToDevice1_HandleResend(PS2_KB_CMD_ENABLE_SCANNING);
		if (buff != PS2_KB_RES_ACK) m_PS2Keyboard.enabled = false;
	}
	if (m_PS2Mouse.enabled) {
		buff = sendByteToDevice2_HandleResend(PS2_KB_CMD_ENABLE_SCANNING);
		if (buff != PS2_KB_RES_ACK) m_PS2Mouse.enabled = false;
	}

	PS2Controller_enableDevicesInterrupts();
	Logging_log(SUCCESS, MODULE, "Initialization success");
}

#pragma endregion PS/2 Initialize

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
		case 0x2f: return KEY_F13;
		case 0x31: return KEY_N;
		case 0x32: return KEY_B;
		case 0x33: return KEY_H;
		case 0x34: return KEY_G;
		case 0x35: return KEY_Y;
		case 0x36: return KEY_6;
		case 0x37: return KEY_F14;
		case 0x3a: return KEY_M;
		case 0x3b: return KEY_J;
		case 0x3c: return KEY_U;
		case 0x3d: return KEY_7;
		case 0x3e: return KEY_8;
		case 0x3f: return KEY_F15;
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
static inline int cyclePauseSequence(int pauseSequenceIndex, uint8_t scancode){
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
	if (g_pressedPrintScreenSequence > 0){
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
	if (g_releasedPrintScreenSequence > 0){
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

	// Note: In reality, sysrq is the sequence: [0x11,0xf0,0x11,0x11,0xf0,0x11,0x11,0x84,0xf0,0x84,0xf0,0x11,0x11]
	// However, since it starts with "make alt, break alt, make alt, break alt", we would need bufferize each
	// input and setup a timeout, which when reached, would indicate that we're not getting the sysrq sequence but
	// actually got two presses of alt in a row from the user. Since this would be costly to implement, we chose
	// instead to implement it so that we simply sends spurious alt presses to the keyboard subsystem.
	// These won't matter anyway, since the user pressed sysrq (alt+print_screen) key, which includes the alt key.
	if (scancode == PS2_KB_SCANCODE_SYSRQ){
		// Do note notify on release
		// Note: we don't invert the g_isBreakedState to manipulate the incomming fake alt presses from the sysrq sequence
		// This way, we invert 0xf0,0x11,0x11 break alt make alt => make alt release alt, and end up with a released alt key
		if (g_isBreakedState) return;
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
			log(ERROR, MODULE, "Unrecognized scancode %p", scancode);
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
	if(!m_PS2Keyboard.enabled) return;
	uint8_t code;

	// Debug print
	// code = inb(0x60);
	// if (code == 0x5a) debug("");
	// else debug("%p ", code);
	// return;

	bool res = PS2Controller_receiveDeviceByte(&code);
	// this check should not be needed on x86, but might be
	// on other plateforms ? so it's here just in case
	if (!res) return;

	// Verify that it IS a keycode and not a response from a command (should not happen though)
	if (isResponseCode(code)){
		log(WARNING, MODULE, "Got a reponse byte instead of a keycode, ignoring (byte=%p)", code);
		return;
	}

	handleScancode(code);
}

#pragma endregion PS/2 Keyboard

#pragma region PS/2 Mouse

void PS2_handleMousePacket(uint8_t flags, uint8_t dx, uint8_t dy, uint8_t wheelAndThumbBtn){
	// byte1 bits: Y overflow, X overflow, Y sign bit, X sign bit, Always 1, Middle Btn, Right Btn, Left Btn
	// byte2: X movement
	// byte3: Y movement
	// byte4: [OPT]: wheel & thumb button

	// If X or Y overflow, we can ignore the packet !

	debug("mouse interrupt, packet=[%#hhx, %#hhx, %#hhx, %#hhx]", flags, dx, dy, wheelAndThumbBtn);
}

void PS2_notifyMouse(){
	static int packet_index = 0; // current index in packet streams
	static uint8_t flags, dx, dy;
	uint8_t data;

	bool res = PS2Controller_receiveDeviceByte(&data);
	if (!res) return;

	switch (packet_index){
	case 0:
		flags = data;	
		packet_index++;
		break;
	case 1:
		dx = data;	
		packet_index++;
		break;
	case 2:
		dy = data;
		packet_index++;
		// If packet size is 4, stop here
		if (m_PS2Mouse.packetSize > 3) break;
		// Otherwise we got a full packet, we can handle it
		data = 0xff; // wheelAndThumbBtn is dummy here
	case 3:
		// data is now 'wheelAndThumbBtn'
		PS2_handleMousePacket(flags, dx, dy, data);
		packet_index = 0;
	}
}

#pragma endregion PS/2 Mouse
