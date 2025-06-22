#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "assert.h"
#include "IRQ.h"
#include "Logging.h"
#include "HAL/PS2Controller.h"
#include "Drivers/Keycodes.h"
#include "Drivers/Keyboard.h"

#include "PS2.h"

#define MODULE "PS/2"

// Note 1: we support only keyboard on port 1 and mouse on port 2
// This is similar to other OSes

// Commands (prefixes: KB=Keyboard, MOUSE=Mouse, None=Both)
#define PS2_CMD_IDENTIFY				0xf2
#define PS2_CMD_ENABLE_SCANNING			0xf4
#define PS2_CMD_DISABLE_SCANNING		0xf5
#define PS2_CMD_SET_DEFAULT_PARAMS		0xf6
#define PS2_CMD_RESET					0xff
#define PS2_KB_CMD_SET_LED				0xed
#define PS2_KB_CMD_ECHO					0xee
#define PS2_KB_CMD_SET_SCANCODE_SET		0xf0 // Get/Set scancode set
#define PS2_KB_CMD_SET_DELAY_AND_RATE	0xf3
#define PS2_MOUSE_CMD_SET_SAMPLERATE	0xf3
#define PS2_MOUSE_CMD_SET_RESOLUTION	0xe8
// Commands data byte
#define PS2_KB_SCANCODE_GET				0x00
#define PS2_KB_SCANCODE_SET1			0x01
#define PS2_KB_SCANCODE_SET2			0x02
#define PS2_KB_SCANCODE_SET3			0x03
#define PS2_KB_LED_SCROLLLOCK			0b00000001
#define PS2_KB_LED_NUMLOCK				0b00000010
#define PS2_KB_LED_CAPSLOCK				0b00000100
// Special Bytes responses
#define PS2_RES_ERROR0					0x00
#define PS2_RES_SELFTEST_PASSED			0xaa
#define PS2_RES_ECHO					0xee
#define PS2_RES_ACK						0xfa
#define PS2_RES_SELFTEST_FAILED0		0xfc
#define PS2_RES_SELFTEST_FAILED1		0xfd
#define PS2_RES_RESEND					0xfe
#define PS2_RES_ERROR1					0xff // Same as KB_RESPONSE_ERROR0, other scancode

struct PS2Keyboard {
	bool enabled;
	bool translated;
	uint8_t scancodeSet; // 1, 2 or 3
	const char* name;
	uint8_t LEDs; // The low 3 bits represent each LED's status
};

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

#pragma region PS/2 Keyboard

#define PS2_KB_SCANCODE_ESCAPE					0xe0
#define PS2_KB_SCANCODE_ESCAPE_PAUSE			0xe1
#define PS2_KB_SCANCODE_BREAK					0xf0
#define PS2_KB_SCANCODE1_SYSRQ					0x54
#define PS2_KB_SCANCODE2_SYSRQ					0x84

// Translates the PS/2 scancode into mugOS Keycode. Returns KEY_RESERVED on unrecognized keycode
static Keycode getKeycodeSet1(uint8_t scancode){
	switch (scancode){
		case 0x00: return KEY_RESERVED;
		case 0x01: return KEY_ESC;
		case 0x02: return KEY_1;
		case 0x03: return KEY_2;
		case 0x04: return KEY_3;
		case 0x05: return KEY_4;
		case 0x06: return KEY_5;
		case 0x07: return KEY_6;
		case 0x08: return KEY_7;
		case 0x09: return KEY_8;
		case 0x0a: return KEY_9;
		case 0x0b: return KEY_0;
		case 0x0c: return KEY_MINUS;
		case 0x0d: return KEY_EQUAL;
		case 0x0e: return KEY_BACKSPACE;
		case 0x0f: return KEY_TAB;
		case 0x10: return KEY_Q;
		case 0x11: return KEY_W;
		case 0x12: return KEY_E;
		case 0x13: return KEY_R;
		case 0x14: return KEY_T;
		case 0x15: return KEY_Y;
		case 0x16: return KEY_U;
		case 0x17: return KEY_I;
		case 0x18: return KEY_O;
		case 0x19: return KEY_P;
		case 0x1a: return KEY_LBRACKET;
		case 0x1b: return KEY_RBRACKET;
		case 0x1c: return KEY_ENTER;
		case 0x1d: return KEY_LCTRL;
		case 0x1e: return KEY_A;
		case 0x1f: return KEY_S;
		case 0x20: return KEY_D;
		case 0x21: return KEY_F;
		case 0x22: return KEY_G;
		case 0x23: return KEY_H;
		case 0x24: return KEY_J;
		case 0x25: return KEY_K;
		case 0x26: return KEY_L;
		case 0x27: return KEY_SEMICOLON;
		case 0x28: return KEY_QUOTE;
		case 0x29: return KEY_GRAVE;
		case 0x2a: return KEY_LSHIFT;
		case 0x2b: return KEY_BACKSLASH;
		case 0x2c: return KEY_Z;
		case 0x2d: return KEY_X;
		case 0x2e: return KEY_C;
		case 0x2f: return KEY_V;
		case 0x30: return KEY_B;
		case 0x31: return KEY_N;
		case 0x32: return KEY_M;
		case 0x33: return KEY_COMMA;
		case 0x34: return KEY_DOT;
		case 0x35: return KEY_SLASH;
		case 0x36: return KEY_RSHIFT;
		case 0x37: return KEY_NUMPAD_ASTERISK;
		case 0x38: return KEY_LALT;
		case 0x39: return KEY_SPACE;
		case 0x3a: return KEY_CAPSLOCK;
		case 0x3b: return KEY_F1;
		case 0x3c: return KEY_F2;
		case 0x3d: return KEY_F3;
		case 0x3e: return KEY_F4;
		case 0x3f: return KEY_F5;
		case 0x40: return KEY_F6;
		case 0x41: return KEY_F7;
		case 0x42: return KEY_F8;
		case 0x43: return KEY_F9;
		case 0x44: return KEY_F10;
		case 0x45: return KEY_NUMLOCK;
		case 0x46: return KEY_SCROLLLOCK;
		case 0x47: return KEY_NUMPAD_7;
		case 0x48: return KEY_NUMPAD_8;
		case 0x49: return KEY_NUMPAD_9;
		case 0x4a: return KEY_NUMPAD_MINUS;
		case 0x4b: return KEY_NUMPAD_4;
		case 0x4c: return KEY_NUMPAD_5;
		case 0x4d: return KEY_NUMPAD_6;
		case 0x4e: return KEY_NUMPAD_PLUS;
		case 0x4f: return KEY_NUMPAD_1;
		case 0x50: return KEY_NUMPAD_2;
		case 0x51: return KEY_NUMPAD_3;
		case 0x52: return KEY_NUMPAD_0;
		case 0x53: return KEY_NUMPAD_DOT;
		case 0x54: return KEY_RESERVED; // SysRq
		case 0x56: return KEY_LESSTHAN;
		case 0x57: return KEY_F11;
		case 0x58: return KEY_F12;
		case 0x5d: return KEY_F13;
		case 0x5e: return KEY_F14;
		case 0x5f: return KEY_F15;
		case 0x70: return KEY_KATAKANAHIRAGANA;
		case 0x77: return KEY_HIRAGANA;
		case 0x79: return KEY_HENKAN;
		case 0x7b: return KEY_MUHENKAN;
		case 0x7d: return KEY_YEN;

		default: return KEY_RESERVED;
	}
}

// Same as getKeycodeSet2, but for escaped characters
static Keycode getKeycodeEscapedSet1(uint8_t scancode){
	switch (scancode){
		case 0x10: return KEY_PREVIOUSSONG;
		case 0x19: return KEY_NEXTSONG;
		case 0x1c: return KEY_NUMPAD_ENTER;
		case 0x1d: return KEY_RCTRL;
		case 0x20: return KEY_MUTE;
		case 0x21: return KEY_CALC;
		case 0x22: return KEY_PLAYPAUSE;
		case 0x23: return KEY_PASTE;
		case 0x24: return KEY_STOPCD;
		case 0x25: return KEY_CUT;
		case 0x26: return KEY_COPY;
		case 0x2a: return KEY_IGNORE; // Fake LShift
		case 0x2e: return KEY_VOLUMEDOWN;
		case 0x30: return KEY_VOLUMEUP;
		case 0x32: return KEY_HOMEPAGE;
		case 0x35: return KEY_NUMPAD_SLASH;
		case 0x36: return KEY_IGNORE; // Fake RShift
		case 0x37: return KEY_IGNORE; // Ctrl+PrintScreen
		case 0x38: return KEY_RALT;
		case 0x46: return KEY_IGNORE; // Ctrl+Break
		case 0x47: return KEY_HOME;
		case 0x48: return KEY_UP;
		case 0x49: return KEY_PAGEUP;
		case 0x4b: return KEY_LEFT;
		case 0x4d: return KEY_RIGHT;
		case 0x4f: return KEY_END;
		case 0x50: return KEY_DOWN;
		case 0x51: return KEY_PAGEDOWN;
		case 0x52: return KEY_INSERT;
		case 0x53: return KEY_DELETE;
		case 0x5b: return KEY_LMETA;
		case 0x5c: return KEY_RMETA;
		case 0x5d: return KEY_MENU;
		case 0x5e: return KEY_POWER;
		case 0x5f: return KEY_SLEEP;
		case 0x63: return KEY_WAKEUP;
		case 0x65: return KEY_SEARCH;
		case 0x66: return KEY_BOOKMARKS;
		case 0x67: return KEY_REFRESH;
		case 0x68: return KEY_STOP;
		case 0x69: return KEY_FORWARD;
		case 0x6a: return KEY_BACK;
		case 0x6b: return KEY_COMPUTER;
		case 0x6c: return KEY_MAIL;
		case 0x6d: return KEY_MEDIA;

		default: return KEY_RESERVED;
	}
}

static bool cycleLedSequence(bool startSequence, uint8_t leds, bool acked){
	static int led_sequence = 0;

	if (startSequence)
		led_sequence = 1;

	if (acked)
		led_sequence++;

	switch (led_sequence){
	case 0:
		return false;
	case 1:
		// Send SET_LED command
		PS2Controller_sendByteToDevice(1, PS2_KB_CMD_SET_LED);
		return false;
	case 2:
		// Send LEDs value
		PS2Controller_sendByteToDevice(1, leds);
		return false;
	case 3:
		led_sequence = 0;
		return true;
	default:
		break;
	}

	return false;
}

static inline void handleScancodeSet1(uint8_t scancode){
	// Scancode handler states
	static bool released = false;
	static bool escaped_state = false;
	static int print_screen_sequence_pressed = 0;
	static int print_screen_sequence_released = 0;
	static int pause_sequence = 0;
	static bool numlock_was_set=false, capslock_was_set=false, scrolllock_was_set=false;
	static uint8_t wanted_leds = 0;
	static int resend_count = 0;

	// 1. Check for sequences progression
	// Check for print screen pressed sequence : 0xe0, 0x2a, 0xe0, 0x37
	switch (print_screen_sequence_pressed) {
	case 0:
		break;
	case 1:
		if (scancode != 0x2a) {print_screen_sequence_pressed = 0; break;}
		print_screen_sequence_pressed++;
		escaped_state = false;
		return;
	case 2:
		if (scancode != 0xe0) {print_screen_sequence_pressed = 0; break;}
		print_screen_sequence_pressed++;
		return;
	case 3:
		if (scancode != 0x37) {print_screen_sequence_pressed = 0; escaped_state = true; break;}
		Keyboard_notifyPressed(KEY_PRINTSCREEN);
		goto reset_state;
	default:
		break;
	}
	// Check for print screen released sequence : 0xe0, 0xb7, 0xe0, 0xaa
	switch (print_screen_sequence_released) {
	case 0:
		break;
	case 1:
		if (scancode != 0xb7) {print_screen_sequence_released = 0; break;}
		print_screen_sequence_released++;
		return;
	case 2:
		if (scancode != 0xe0) {print_screen_sequence_released = 0; break;}
		print_screen_sequence_released++;
		return;
	case 3:
		if (scancode != 0xaa) {print_screen_sequence_released = 0; break;}
		Keyboard_notifyReleased(KEY_PRINTSCREEN);
		goto reset_state;
	default:
		break;
	}
	// Check for pause sequence: 0xe1, 0x1d, 0x45, 0xe1, 0x9d, 0xc5
	switch (pause_sequence){
	case 0:
		break;
	case 1:
		if (scancode != 0x1d) {pause_sequence = 0 ; break;}
		pause_sequence++;
		return;
	case 2:
		if (scancode != 0x45) {pause_sequence = 0 ; break;}
		pause_sequence++;
		return;
	case 3:
		if (scancode != 0xe1) {pause_sequence = 0 ; break;}
		pause_sequence++;
		return;
	case 4:
		if (scancode != 0x9d) {pause_sequence = 0 ; break;}
		pause_sequence++;
		return;
	case 5:
		if (scancode != 0xc5) {pause_sequence = 0 ; break;}
		Keyboard_notifyPressed(KEY_PAUSE);
		goto reset_state;
	default:
		break;
	}

	// 2. Preprocess scancode & handle special cases
	switch (scancode){
		case PS2_KB_SCANCODE1_SYSRQ:
		// Note: In reality, sysrq is the sequence:
		// 0x38, 0xb8, 0x38, 0xb8, 0x38, 0x54, 0xd4, 0xb8, 0x38, 0xb8
		// For the sake of simplicity, we chose to "ignore" the alt presses,
		// and just send them as spurious presses.
		Keyboard_notifySysRq();
		return;
	case PS2_KB_SCANCODE_ESCAPE:
		escaped_state = true;
		print_screen_sequence_pressed = 1;
		print_screen_sequence_released = 1;
		return;
	case PS2_KB_SCANCODE_ESCAPE_PAUSE:
		pause_sequence = 1;
		return;
	case PS2_KB_SCANCODE1_SYSRQ | 0x80:
		// Ignore released
		return;
	case PS2_RES_ACK:
		if (cycleLedSequence(false, wanted_leds, true)){
			// Since keyboard ACKed the leds, they are set
			// We can now update our buffered value, and notify the keyboard subsystem
			m_PS2Keyboard.LEDs = wanted_leds;
			if (numlock_was_set) Keyboard_notifyPressed(KEY_NUMLOCK);
			if (scrolllock_was_set) Keyboard_notifyPressed(KEY_SCROLLLOCK);
			if (capslock_was_set) Keyboard_notifyPressed(KEY_CAPSLOCK);
		}
		return;
	case PS2_RES_RESEND:
		resend_count++;
		// We limit the number of resend allowed to avoid infinite loops
		if (resend_count == 3) { resend_count = 0; goto reset_state; }
		cycleLedSequence(false, wanted_leds, false);
		return;
	default:
		// If first bit is set, key is released
		if (scancode & 0x80){
			scancode &= 0x7f;
			released = true;
		}
	}

	// 3. If we're here, we got a simple one-press scancode. Convert it to keycode
	Keycode keycode;
	keycode = (escaped_state) ? getKeycodeEscapedSet1(scancode) : getKeycodeSet1(scancode);

	// 4. Handle keycode
	switch (keycode){
	case KEY_RESERVED: // Driver did not recognize the scancode
		log(ERROR, MODULE, "Unrecognized scancode %p", scancode);
		goto reset_state;
	case KEY_NUMLOCK:
		if (released) { numlock_was_set = false; break; } // when released, unlock flip
		if (numlock_was_set) break; // toggle led only at the first press
		wanted_leds = m_PS2Keyboard.LEDs ^ PS2_KB_LED_NUMLOCK; // flip numlock bit
		cycleLedSequence(true, wanted_leds, false);
		numlock_was_set = true;
		return;
	case KEY_SCROLLLOCK:
		if (released) { scrolllock_was_set = false; break; }
		if (scrolllock_was_set) break;
		wanted_leds = m_PS2Keyboard.LEDs ^ PS2_KB_LED_SCROLLLOCK;
		cycleLedSequence(true, wanted_leds, false);
		scrolllock_was_set = true;
		return;
	case KEY_CAPSLOCK:
		if (released) { capslock_was_set = false; break; }
		if (capslock_was_set) break;
		wanted_leds = m_PS2Keyboard.LEDs ^ PS2_KB_LED_CAPSLOCK;
		cycleLedSequence(true, wanted_leds, false);
		capslock_was_set = true;
		return;
	case KEY_IGNORE:
		goto reset_state;
	default:
		// Key was recognized
		break;
	}

	// 5. Message the keyboard driver
	(released) ? Keyboard_notifyReleased(keycode) : Keyboard_notifyPressed(keycode);

	reset_state:
	released = false;
	escaped_state = false;
	print_screen_sequence_pressed = 0;
	print_screen_sequence_released = 0;
	pause_sequence = 0;
}

// Translates the PS/2 scancode into mugOS Keycode. Returns KEY_RESERVED on unrecognized keycode
static Keycode getKeycodeSet2(uint8_t scancode){
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
		case 0x13: return KEY_KATAKANAHIRAGANA;
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
		case 0x62: return KEY_HIRAGANA;
		case 0x64: return KEY_HENKAN;
		case 0x66: return KEY_BACKSPACE;
		case 0x67: return KEY_MUHENKAN;
		case 0x69: return KEY_NUMPAD_1;
		case 0x6a: return KEY_YEN;
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

// Same as getKeycodeSet2, but for escaped characters
static Keycode getKeycodeEscapedSet2(uint8_t scancode){
	switch (scancode){
		case 0x10: return KEY_SEARCH;
		case 0x11: return KEY_RALT;
		case 0x14: return KEY_RCTRL;
		case 0x15: return KEY_PREVIOUSSONG;
		case 0x18: return KEY_BOOKMARKS;
		case 0x1f: return KEY_LMETA;
		case 0x20: return KEY_REFRESH;
		case 0x21: return KEY_VOLUMEDOWN;
		case 0x23: return KEY_MUTE;
		case 0x27: return KEY_RMETA;
		case 0x28: return KEY_STOP;
		case 0x2b: return KEY_CALC;
		case 0x2f: return KEY_MENU;
		case 0x30: return KEY_FORWARD;
		case 0x32: return KEY_VOLUMEUP;
		case 0x33: return KEY_PASTE;
		case 0x34: return KEY_PLAYPAUSE;
		case 0x37: return KEY_POWER;
		case 0x38: return KEY_BACK;
		case 0x3a: return KEY_HOMEPAGE;
		case 0x3b: return KEY_STOP;
		case 0x3f: return KEY_SLEEP;
		case 0x42: return KEY_CUT;
		case 0x48: return KEY_MAIL;
		case 0x4a: return KEY_NUMPAD_SLASH;
		case 0x4b: return KEY_PASTE;
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

static inline void handleScancodeSet2(uint8_t scancode){
	// Scancode handler states
	static bool breaked_state = false;
	static bool escaped_state = false;
	static int print_screen_sequence_pressed = 0;
	static int print_screen_sequence_released = 0;
	static int pause_sequence = 0;
	static bool numlock_was_set=false, capslock_was_set=false, scrolllock_was_set=false;
	static uint8_t wanted_leds = 0;
	static int resend_count = 0;

	// Note: See handleScancodeSet1 for full commented function

	// Check for print screen pressed sequence : 0xe0, 0x12, 0xe0, 0x7c
	switch (print_screen_sequence_pressed){
	case 0:
		break;
	case 1:
		if (scancode != 0x12) {print_screen_sequence_pressed = 0; escaped_state = true; break;}
		print_screen_sequence_pressed++;
		escaped_state = false;
		return;
	case 2:
		if (scancode != 0xe0) {print_screen_sequence_pressed = 0; break;}
		print_screen_sequence_pressed++;
		return;
	case 3:
		if (scancode != 0x7c) {print_screen_sequence_pressed = 0; escaped_state = true; break;}
		Keyboard_notifyPressed(KEY_PRINTSCREEN);
		print_screen_sequence_pressed = 0;
		return;
	default:
		break;
	}
	// Check for print screen released sequence : 0xe0, 0xf0, 0x7c, 0xe0, 0xf0, 0x12
	switch (print_screen_sequence_released){
	case 1:
		if (scancode != 0xf0) {print_screen_sequence_released = 0; break;}
		print_screen_sequence_released++;
		breaked_state = true;
		break;
	case 2:
		if (scancode != 0x7c) {print_screen_sequence_released = 0; break;}
		print_screen_sequence_released++;
		return;
	case 3:
		if (scancode != 0xe0) {print_screen_sequence_released = 0; escaped_state = false; breaked_state = false; break;}
		print_screen_sequence_released++;
		return;
	case 4:
		if (scancode != 0xf0) {print_screen_sequence_released = 0; break;}
		print_screen_sequence_released++;
		return;
	case 5:
		if (scancode != 0x12) {print_screen_sequence_released = 0; break;}
		Keyboard_notifyReleased(KEY_PRINTSCREEN);
		print_screen_sequence_released = 0;
		escaped_state = false;
		breaked_state = false;
		return;
	default:
		break;
	}
	// Check for pause sequence: 0xe1, 0x14, 0x77, 0xe1, 0xf0, 0x14, 0xf0, 0x77
	switch (pause_sequence){
	case 0:
		break;
	case 1:
		if (scancode != 0x14) {pause_sequence = 0; break;}
		pause_sequence++;
		return;
	case 2:
		if (scancode != 0x77) {pause_sequence = 0; break;}
		pause_sequence++;
		return;
	case 3:
		if (scancode != 0xe1) {pause_sequence = 0; break;}
		pause_sequence++;
		return;
	case 4:
		if (scancode != 0xf0) {pause_sequence = 0; break;}
		pause_sequence++;
		return;
	case 5:
		if (scancode != 0x14) {pause_sequence = 0; break;}
		pause_sequence++;
		return;
	case 6:
		if (scancode != 0xf0) {pause_sequence = 0; break;}
		pause_sequence++;
		return;
	case 7:
		if (scancode != 0x77) {pause_sequence = 0; break;}
		Keyboard_notifyPressed(KEY_PAUSE);
		goto reset_state;
	default:
		break;
	}

	switch (scancode){
	case 0x12:
		// Some keyboard send "0xe0 0x12 0xe0 0x70" for INSERT instead of simply "0xe0 0x70"
		// We just ignore the additional 0x12
		if (escaped_state)
			goto reset_state;
		break;
	case PS2_KB_SCANCODE2_SYSRQ:
		if (!breaked_state) Keyboard_notifySysRq();
		// Note: We don't invert the breaked_state to manipulate the incoming spurious alt presses from the sysrq sequence
		// This way, we invert 0xf0,0x11,0x11 break alt make alt => make alt release alt, and end up with a released alt key
		return;
	case PS2_KB_SCANCODE_ESCAPE:
		escaped_state = true;
		print_screen_sequence_pressed = 1;
		print_screen_sequence_released = 1;
		return;
	case PS2_KB_SCANCODE_ESCAPE_PAUSE:
		pause_sequence = 1;
		return;
	case PS2_KB_SCANCODE_BREAK:
		breaked_state = true;
		return;
	case PS2_RES_ACK:
		if (cycleLedSequence(false, wanted_leds, true)){
			m_PS2Keyboard.LEDs = wanted_leds;
			if (numlock_was_set) Keyboard_notifyPressed(KEY_NUMLOCK);
			if (scrolllock_was_set) Keyboard_notifyPressed(KEY_SCROLLLOCK);
			if (capslock_was_set) Keyboard_notifyPressed(KEY_CAPSLOCK);
		}
		return;
	case PS2_RES_RESEND:
		resend_count++;
		// We limit the number of resend allowed to avoid infinite loops
		if (resend_count == 3) { resend_count = 0; goto reset_state; }
		cycleLedSequence(false, wanted_leds, false);
		return;
	default:
		break;
	}

	Keycode keycode;
	keycode = (escaped_state) ? getKeycodeEscapedSet2(scancode) : getKeycodeSet2(scancode);

	switch (keycode){
		case KEY_RESERVED: // Driver did not recognize the scancode
			log(ERROR, MODULE, "Unrecognized scancode %p", scancode);
			goto reset_state;
		case KEY_NUMLOCK:
			if (breaked_state) { numlock_was_set = false; break; }
			if (numlock_was_set) break;
			wanted_leds = m_PS2Keyboard.LEDs ^ PS2_KB_LED_NUMLOCK;
			cycleLedSequence(true, wanted_leds, false);
			numlock_was_set = true;
			return;
		case KEY_SCROLLLOCK:
			if (breaked_state) { scrolllock_was_set = false; break; }
			if (scrolllock_was_set) break;
			wanted_leds = m_PS2Keyboard.LEDs ^ PS2_KB_LED_SCROLLLOCK;
			cycleLedSequence(true, wanted_leds, false);
			scrolllock_was_set = true;
			return;
		case KEY_CAPSLOCK:
			if (breaked_state) { capslock_was_set = false; break; }
			if (capslock_was_set) break;
			wanted_leds = m_PS2Keyboard.LEDs ^ PS2_KB_LED_CAPSLOCK;
			cycleLedSequence(true, wanted_leds, false);
			capslock_was_set = true;
			return;
		default:
			// Key was recognized
			break;
	}

	(breaked_state) ? Keyboard_notifyReleased(keycode) : Keyboard_notifyPressed(keycode);

	reset_state:
	breaked_state = false;
	escaped_state = false;
	print_screen_sequence_pressed = 0;
	print_screen_sequence_released = 0;
	pause_sequence = 0;
}

static void keyboardIRQ(void*){
	if(!m_PS2Keyboard.enabled) return;
	uint8_t code;

	bool res = PS2Controller_receiveByte(&code);
	if (!res) return; // Ignore spurious IRQ

	// debug("Received keycode %#.2hhx", code);

	switch (m_PS2Keyboard.scancodeSet){
	case 1:
		handleScancodeSet1(code);
		break;
	case 2:
		handleScancodeSet2(code);
		break;
	default: // 3 or invalid value
		break;
	}
}

#pragma endregion PS/2 Keyboard

#pragma region PS/2 Mouse

static void handleMousePacket(uint8_t flags, uint8_t raw_dx, uint8_t raw_dy, uint8_t wheelAndThumbBtn){
	// flags bits (8 to 1): Y overflow, X overflow, Y sign bit, X sign bit, Always 1, Middle Btn, Right Btn, Left Btn
	// wheelAndThumbBtn is optional

	int dx, dy;
	bool btn1, btn2, btn3=false, btn4=false, btn5=false;
	int wheel = 0; // wheel movement, from -8 to +7

	// Mouse movement
	// If sign (minus) bit is set, the right part becomes raw_dx - 0x100, which produces
	// the 2 complement of the first operand raw_dx/y
	dx = raw_dx - ((flags << 4) & 0x100);
	dy = raw_dy - ((flags << 3) & 0x100);

	// Left and right buttons
	btn1 = (flags & 0b00000001);
	btn2 = (flags & 0b00000010);

	// Wheel
	if (m_PS2Mouse.type >= PS2MOUSETYPE_WHEEL){
		btn3 = (flags & 0b00000100);
		wheel = (wheelAndThumbBtn & 0b00001000) ?
			(wheelAndThumbBtn | ~0b00000111) : (wheelAndThumbBtn & 0b00000111);
	}

	// Thumb buttons
	if (m_PS2Mouse.type >= PS2MOUSETYPE_5BUTTONS){
		btn4 = (wheelAndThumbBtn & 0b00010000);
		btn5 = (wheelAndThumbBtn & 0b00100000);
	}

	debug("mouse IRQ: btn1=%1d btn2=%1d btn3=%1d btn4=%1d btn5=%1d dx=%- 4d dy=%- 4d wheel=%+.1d", btn1, btn2, btn3, btn4, btn5, dx, dy, wheel);
}

static void mouseIRQ(void*){
	static int packet_index = 0; // current index in packet streams
	static uint8_t flags, dx, dy;
	uint8_t data;

	bool res = PS2Controller_receiveByte(&data);
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
		handleMousePacket(flags, dx, dy, data);
		packet_index = 0;
	}
}

#pragma endregion PS/2 Mouse

#pragma region PS/2 Initialize

// Response buffer, used only during initialization by the temporary irq handler
#define RESPONSE_BUFFER_SIZE 5
static uint8_t m_responseBuffer[RESPONSE_BUFFER_SIZE];
static int m_inBuffer = 0;
#define TIMEOUT 1llu<<24

static bool receiveByte(uint8_t* byte_out){
	// receiveByte is simply popResponseBuffer, with a wait timeout:
	// we try to wait a certain time for the buffer to be filled

	unsigned long long counter = 0;
	while (m_inBuffer<1 && counter<TIMEOUT)
		counter++;
	if (m_inBuffer < 1) return false;

	unsigned long flags;
	IRQ_disableSave(flags); // lock the buffer (TODO: change for a lock)

	*byte_out = m_responseBuffer[0];
	memmove(m_responseBuffer, m_responseBuffer+1, RESPONSE_BUFFER_SIZE-1);
	m_inBuffer--;

	IRQ_restore(flags);
	return true;
}

static void pushResponseBuffer(uint8_t value){
	unsigned long flags;
	IRQ_disableSave(flags); // lock the buffer (change for a lock here too)

	if (m_inBuffer == RESPONSE_BUFFER_SIZE) goto end;
	m_responseBuffer[m_inBuffer] = value;
	m_inBuffer++;

	end:
	IRQ_restore(flags);
}

static void initIRQ(void*){
	uint8_t value;

	bool res = PS2Controller_receiveByte(&value);
	if (!res){
		log(WARNING, MODULE, "Received intialization interrupt, but no value is available to be read");
		return;
	}

	// debug("initIRQ %#.2hhx", value);
	pushResponseBuffer(value);
}

/// @brief Send a byte to a device, and resends (max 3 times) if response is Resend
/// @returns response on success, PS2_KB_RES_RESEND on failure (timed out)
static uint8_t sendByteToDeviceHandleResend(int device, uint8_t byte){
	uint8_t response;

	for(int i=0 ; i<3 ; i++){
		bool success;
		success = PS2Controller_sendByteToDevice(device, byte);
		if (!success) return PS2_RES_RESEND;

		// receiveByte includes a timeout
		if (!receiveByte(&response))
			return PS2_RES_RESEND;

		if (response != PS2_RES_RESEND) break;
	}

	return response;
}

// Get a keyboard name/type (scanning must be disabled !!)
// Returns its name (NULL if unrecognized or is a mouse)
static const char* getKeyboardName(){
	uint8_t buff, byte1, byte2;
	bool res;

	buff = sendByteToDeviceHandleResend(1, PS2_CMD_IDENTIFY);
	if (buff != PS2_RES_ACK) return NULL;

	res = receiveByte(&byte1);
	if (!res){
		return "AT Keyboard";
	}

	res = receiveByte(&byte2);

	// Only one byte: mouse
	if (!res){
		return NULL;
	}

	switch (byte1){
		case 0xab:
			switch (byte2){
				case 0x41: return "PS/2 MF2 keyboard";
				case 0x54: return "PS/2 short keyboard";
				case 0x83: return "PS/2 MF2 keyboard";
				case 0x84: return "PS/2 short keyboard";
				case 0x85: return "PS/2 NCD N-97 keyboard";
				case 0x86: return "PS/2 122-key keyboard";
				case 0x90: return "PS/2 Japanse 'G' keyboard";
				case 0x91: return "PS/2 Japanse 'P' keyboard";
				case 0x92: return "PS/2 Japanse 'A' keyboard";
				case 0xc1: return "PS/2 MF2 keyboard";
				default: break;
			}
		case 0xac:
			switch (byte2){
				case 0xa1: return "PS/2 NCD Sun keyboard";
				default: break;
			}
		default:
			break;
	}

	log(ERROR, MODULE, "Unknown keyboard identify sequence '%#hhx, %#hhx'", byte1, byte2);
	return NULL;
}

// Detect a mouse on port 2 and returns its id (0xff if unrecognized or is a keyboard)
static enum PS2MouseType getMouseType(){
	uint8_t buff;

	buff = sendByteToDeviceHandleResend(2, PS2_CMD_IDENTIFY);
	if (buff != PS2_RES_ACK) return 0xff;

	if (!receiveByte(&buff))
		return 0xff;

	return buff;
}

static inline bool resetDevice(int device){
	assert(device == 1 || device == 2);
	bool res;
	uint8_t buff;

	// Reset
	buff = sendByteToDeviceHandleResend(device, PS2_CMD_RESET);
	if (buff != PS2_RES_ACK) return false;

	// Next byte: self-test passed or failed
	res = receiveByte(&buff);
	if (!res || buff != PS2_RES_SELFTEST_PASSED) return false;

	// If it's a mouse, it sends an additional identification byte. Consume it
	if (device == 2) receiveByte(&buff);

	return true;
}

// Note: check that port 1 is available and populated before calling this method
void PS2_initKeyboard(struct PS2Keyboard* keyboard){
	uint8_t buff1, buff2;
	keyboard->enabled = false;

	// Enable interrupts for keyboard
	PS2Controller_setDevicesIRQ(true, false);

	// Disable scanning for initialization
	// Note that we ignore any error if any, to let the self-test message pop instead
	sendByteToDeviceHandleResend(1, PS2_CMD_DISABLE_SCANNING);

	// Reset the device, and check self-test
	if (!resetDevice(1)){
		log(WARNING, MODULE, "PS/2 keyboard self-test failed, deactivated");
		return;
	}

	// Re-disable scanning (might have been enabled by the reset)
	sendByteToDeviceHandleResend(1, PS2_CMD_DISABLE_SCANNING);

	// Switch to scancode set 2
	buff1 = sendByteToDeviceHandleResend(1, PS2_KB_CMD_SET_SCANCODE_SET);
	buff2 = sendByteToDeviceHandleResend(1, PS2_KB_SCANCODE_SET2);
	if (buff1!=PS2_RES_ACK || buff2!=PS2_RES_ACK){
		log(WARNING, MODULE, "PS/2 keyboard error when setting scancode set 2, deactivated");
		keyboard->enabled = false;
		return;
	}
	keyboard->scancodeSet = (keyboard->translated) ? 1 : 2;

	// Get the keyboard name string (any error is printed by getKeyboardName)
	keyboard->name = getKeyboardName();
	if (keyboard->name == NULL)	return;

	keyboard->enabled = true;

	// Set typematic byte (repeat rate/delay) NOTE: DOESN'T SEEM TO WORK
	// Search x number to send for a wanted rate = -28/31*x + 30
	// Delay: 0b00=250ms 0b01=500ms 0b10=750ms 0b11=1000ms
	sendByteToDeviceHandleResend(1, PS2_KB_CMD_SET_DELAY_AND_RATE);
	sendByteToDeviceHandleResend(1, 0b00000000 | 0b00010110); // Default 10.9 Hz (22) + 500 ms repeat

	// Set LEDs: by default, only NUMLOCK is set
	m_PS2Keyboard.LEDs = PS2_KB_LED_NUMLOCK;
	sendByteToDeviceHandleResend(1, PS2_KB_CMD_SET_LED);
	sendByteToDeviceHandleResend(1, m_PS2Keyboard.LEDs);

	log(INFO, MODULE, "Indentified keyboard as '%s' (using scancode set %d)", keyboard->name, keyboard->scancodeSet);
}

// Note: check that port 2 is available and populated before calling this method
void PS2_initMouse(struct PS2Mouse* mouse){
	mouse->enabled = false;

	// Enable interrupts for keyboard
	PS2Controller_setDevicesIRQ(false, true);

	// Disable scanning for initialization
	// Note that we ignore any error if any, to let the self-test message pop instead
	sendByteToDeviceHandleResend(2, PS2_CMD_DISABLE_SCANNING);

	// Reset the device, and check self-test
	if (!resetDevice(2)){
		log(WARNING, MODULE, "PS/2 mouse self-test failed, deactivated");
		return;
	}

	mouse->type = getMouseType();
	if (mouse->type == 0xff){
		log(INFO, MODULE, "No mouse detected, deactivated");
		return;
	}

	// Try to enable scroll wheel
	// Magic sequence: set sample rate to 200, then 100, then 80
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 200);
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 100);
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 80);
	mouse->type = getMouseType(); // update mouse type

	// Try to enable mouse button 4 and 5
	// Magic sequence: set sample rate to 200, then 200, then 80
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 200);
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 200);
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 80);
	mouse->type = getMouseType(); // update mouse type

	// Set Samplerate and Resolution
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_SAMPLERATE);
	sendByteToDeviceHandleResend(2, 40); // 40 samples/sec (as recommended by osdev.org)
	sendByteToDeviceHandleResend(2, PS2_MOUSE_CMD_SET_RESOLUTION);
	sendByteToDeviceHandleResend(2, 0x03); // 8 count/mm

	// Set the mouse name
	switch (mouse->type){
	case PS2MOUSETYPE_STD:
		mouse->name = "Standard PS/2 mouse";
		mouse->packetSize = 3;
		break;
	case PS2MOUSETYPE_WHEEL:
		mouse->name = "PS/2 mouse with scroll wheel";
		mouse->packetSize = 4;
		break;
	case PS2MOUSETYPE_5BUTTONS:
		mouse->name = "PS/2 5-button mouse";
		mouse->packetSize = 4;
		break;
	default:
		log(WARNING, MODULE, "Couldn't initialize PS/2 mouse (invalid mouse id)");
		return;
	}

	mouse->enabled = true;
	log(INFO, MODULE, "Identified mouse as '%s'", mouse->name);
}

void PS2_init(){
	PS2Controller_init();
	// By default, the PS/2 controller leaves the device's IRQs disabled

	// Temporary handler for intialization
	IRQ_registerHandler(IRQ_PS2_KEYBOARD, initIRQ);
	IRQ_registerHandler(IRQ_PS2_MOUSE, initIRQ);

	bool enabled, port1_enabled, port2_enabled;
	PS2Controller_getStatus(&enabled, &port1_enabled, &port2_enabled, &m_PS2Keyboard.translated);
	if (!enabled){
		m_enabled = false;
		m_PS2Keyboard.enabled = false;
		m_PS2Mouse.enabled = false;
		log(ERROR, MODULE, "Initalization failed, PS/2 Controller driver is disabled");
		return;
	}
	m_enabled = true;

	// Note: the initialize function print necessary informations already
	if (port1_enabled) PS2_initKeyboard(&m_PS2Keyboard);
	if (port2_enabled) PS2_initMouse(&m_PS2Mouse);

	// After intialization, we can re-enable irqs for functionning devices
	PS2Controller_setDevicesIRQ(m_PS2Keyboard.enabled, m_PS2Mouse.enabled);

	// Same for scanning ; and we can now register the scancodes-capable IRQ handlers
	if (m_PS2Keyboard.enabled) {
		sendByteToDeviceHandleResend(1, PS2_CMD_ENABLE_SCANNING);
		IRQ_registerHandler(IRQ_PS2_KEYBOARD, keyboardIRQ);
	} else {
		IRQ_deregisterHandler(IRQ_PS2_KEYBOARD);
	}
	if (m_PS2Mouse.enabled) {
		sendByteToDeviceHandleResend(2, PS2_CMD_ENABLE_SCANNING);
		IRQ_registerHandler(IRQ_PS2_MOUSE, mouseIRQ);
	} else {
		IRQ_deregisterHandler(IRQ_PS2_MOUSE);
	}

	log(SUCCESS, MODULE, "Initialization success");
}

#pragma endregion PS/2 Initialize
