#include <stddef.h>
#include "Logging.h"
#include "Drivers/Input/Keycodes.h"

#include "Keyboard.h"

#define MODULE "Keyboard subsystem"

// Driver state variables
static bool m_capslockOn;
static bool m_numlockOn;
static bool m_numlockKeyWasFunction[11]; // set when numpad key pressed, used when released
// Modifier keys
static uint8_t m_modifierKeys;
static bool m_leftShiftPressed;
static bool m_rightShiftPressed;
static bool m_leftCtrlPressed;
static bool m_rightCtrlPressed;
static bool m_leftAltPressed;
static bool m_rightAltPressed;
static bool m_leftMetaPressed;
static bool m_rightMetaPressed;

#define MAX_CALLBACKS 16
keycallback_t m_callbacks[MAX_CALLBACKS];

static char getNumpadKey_NoNumluck(keycode_t k){
	switch (k){
		case KEY_NUMPAD_7: return KEY_HOME;
		case KEY_NUMPAD_8: return KEY_UP;
		case KEY_NUMPAD_9: return KEY_PAGEUP;
		case KEY_NUMPAD_4: return KEY_LEFT;
		case KEY_NUMPAD_5: return (char) 0;
		case KEY_NUMPAD_6: return KEY_RIGHT;
		case KEY_NUMPAD_1: return KEY_END;
		case KEY_NUMPAD_2: return KEY_DOWN;
		case KEY_NUMPAD_3: return KEY_PAGEDOWN;
		case KEY_NUMPAD_0: return KEY_INSERT;
		case KEY_NUMPAD_DOT: return KEY_DELETE;
		default: return (char) 0;
	}
}

/// @returns Whether the keycode_t `k` is a numpad key THAT DO CHANGE depending on the NumLock state
///          e.g. KEY_NUMPAD_ENTER returns false !
static inline bool isNumpadKey(keycode_t k){
	switch (k){
		case KEY_NUMPAD_7: return true;
		case KEY_NUMPAD_8: return true;
		case KEY_NUMPAD_9: return true;
		case KEY_NUMPAD_4: return true;
		case KEY_NUMPAD_5: return true;
		case KEY_NUMPAD_6: return true;
		case KEY_NUMPAD_1: return true;
		case KEY_NUMPAD_2: return true;
		case KEY_NUMPAD_3: return true;
		case KEY_NUMPAD_0: return true;
		case KEY_NUMPAD_DOT: return true;
		default: return false;
	}
}

// ================ Get printable character from corresponding keycode_t ================

static char getChar_AZERTY_NoModifier(keycode_t k){
	switch (k){
		case KEY_RESERVED:			return (char) 0;
		case KEY_1:					return '&';
		case KEY_2:					return (char) 130; // é
		case KEY_3:					return '"';
		case KEY_4:					return '\'';
		case KEY_5:					return '(';
		case KEY_6:					return '-';
		case KEY_7:					return (char) 138; // è
		case KEY_8:					return '_';
		case KEY_9:					return (char) 135; // ç
		case KEY_0:					return (char) 133; // à
		case KEY_MINUS:				return ')';
		case KEY_EQUAL:				return '=';
		case KEY_TAB:				return '\t';
		case KEY_Q:					return 'a';
		case KEY_W:					return 'z';
		case KEY_E:					return 'e';
		case KEY_R:					return 'r';
		case KEY_T:					return 't';
		case KEY_Y:					return 'y';
		case KEY_U:					return 'u';
		case KEY_I:					return 'i';
		case KEY_O:					return 'o';
		case KEY_P:					return 'p';
		case KEY_LBRACKET:			return '^';
		case KEY_RBRACKET:			return '$';
		case KEY_ENTER:				return '\n'; // enter
		case KEY_A:					return 'q';
		case KEY_S:					return 's';
		case KEY_D:					return 'd';
		case KEY_F:					return 'f';
		case KEY_G:					return 'g';
		case KEY_H:					return 'h';
		case KEY_J:					return 'j';
		case KEY_K:					return 'k';
		case KEY_L:					return 'l';
		case KEY_SEMICOLON:			return 'm';
		case KEY_QUOTE:				return (char) 163; // ù
		case KEY_GRAVE:				return (char) 253; // ²
		case KEY_BACKSLASH:			return '*';
		case KEY_Z:					return 'w';
		case KEY_X:					return 'x';
		case KEY_C:					return 'c';
		case KEY_V:					return 'v';
		case KEY_B:					return 'b';
		case KEY_N:					return 'n';
		case KEY_M:					return ',';
		case KEY_COMMA:				return ';';
		case KEY_DOT:				return ':';
		case KEY_SLASH:				return '!';
		case KEY_NUMPAD_ASTERISK:	return '*';
		case KEY_SPACE:				return ' ';
		case KEY_NUMPAD_7:			return '7'; // Keypad 7 OR Home
		case KEY_NUMPAD_8:			return '8'; // Keypad 8 OR Up
		case KEY_NUMPAD_9:			return '9'; // Keypad 9 OR Page Up
		case KEY_NUMPAD_MINUS:		return '-'; // Keypad -
		case KEY_NUMPAD_4:			return '4'; // Keypad 4 OR Left
		case KEY_NUMPAD_5:			return '5'; // Keypad 5
		case KEY_NUMPAD_6:			return '6'; // Keypad 6 OR Right
		case KEY_NUMPAD_PLUS:		return '+'; // Keypad +
		case KEY_NUMPAD_1:			return '1'; // Keypad 1 OR End
		case KEY_NUMPAD_2:			return '2'; // Keypad 2 OR Down
		case KEY_NUMPAD_3:			return '3'; // Keypad 3 OR Page Down
		case KEY_NUMPAD_0:			return '0'; // Keypad 0 OR Insert
		case KEY_NUMPAD_DOT:		return '.'; // Keypad . OR Del
		case KEY_LESSTHAN:			return '<';
		case KEY_NUMPAD_ENTER:		return '\n';
		case KEY_NUMPAD_SLASH:		return '/';
		default:
			return (char) 0;
	}
}

static char getChar_AZERTY_CapsLock(keycode_t k){
		switch (k){
		case KEY_RESERVED:			return (char) 0;
		case KEY_1:					return '&';
		case KEY_2:					return (char) 144; // É
		case KEY_3:					return '"';
		case KEY_4:					return '\'';
		case KEY_5:					return '(';
		case KEY_6:					return '-';
		case KEY_7:					return (char) 138; // È not available so we return 'è'
		case KEY_8:					return '_';
		case KEY_9:					return (char) 128; // Ç
		case KEY_0:					return (char) 133; // À not available so we return 'à'
		case KEY_MINUS:				return ')';
		case KEY_EQUAL:				return '=';
		case KEY_TAB:				return '\t';
		case KEY_Q:					return 'A';
		case KEY_W:					return 'Z';
		case KEY_E:					return 'E';
		case KEY_R:					return 'R';
		case KEY_T:					return 'T';
		case KEY_Y:					return 'Y';
		case KEY_U:					return 'U';
		case KEY_I:					return 'I';
		case KEY_O:					return 'O';
		case KEY_P:					return 'P';
		case KEY_LBRACKET:			return '^';
		case KEY_RBRACKET:			return '$';
		case KEY_ENTER:				return '\n';
		case KEY_A:					return 'Q';
		case KEY_S:					return 'S';
		case KEY_D:					return 'D';
		case KEY_F:					return 'F';
		case KEY_G:					return 'G';
		case KEY_H:					return 'H';
		case KEY_J:					return 'J';
		case KEY_K:					return 'K';
		case KEY_L:					return 'L';
		case KEY_SEMICOLON:			return 'M';
		case KEY_QUOTE:				return (char) 163; // Ù not available so we return 'ù'
		case KEY_GRAVE:				return (char) 253; // ²
		case KEY_BACKSLASH:			return '*';
		case KEY_Z:					return 'W';
		case KEY_X:					return 'X';
		case KEY_C:					return 'C';
		case KEY_V:					return 'V';
		case KEY_B:					return 'B';
		case KEY_N:					return 'N';
		case KEY_M:					return ',';
		case KEY_COMMA:				return ';';
		case KEY_DOT:				return ':';
		case KEY_SLASH:				return '!';
		case KEY_NUMPAD_ASTERISK:	return '*';
		case KEY_SPACE:				return ' ';
		case KEY_NUMPAD_7:			return '7'; // Keypad 7 OR Home
		case KEY_NUMPAD_8:			return '8'; // Keypad 8 OR Up
		case KEY_NUMPAD_9:			return '9'; // Keypad 9 OR Page Up
		case KEY_NUMPAD_MINUS:		return '-'; // Keypad -
		case KEY_NUMPAD_4:			return '4'; // Keypad 4 OR Left
		case KEY_NUMPAD_5:			return '5'; // Keypad 5
		case KEY_NUMPAD_6:			return '6'; // Keypad 6 OR Right
		case KEY_NUMPAD_PLUS:		return '+'; // Keypad +
		case KEY_NUMPAD_1:			return '1'; // Keypad 1 OR End
		case KEY_NUMPAD_2:			return '2'; // Keypad 2 OR Down
		case KEY_NUMPAD_3:			return '3'; // Keypad 3 OR Page Down
		case KEY_NUMPAD_0:			return '0'; // Keypad 0 OR Insert
		case KEY_NUMPAD_DOT:		return '.'; // Keypad . OR Del
		case KEY_LESSTHAN:			return '<';
		case KEY_NUMPAD_ENTER:		return '\n';
		case KEY_NUMPAD_SLASH:		return '/';
		default:
			return (char) 0;
	}
}

static char getChar_AZERTY_ShiftAltGr(keycode_t k){
		switch (k){
		case KEY_RESERVED:			return (char) 0;
		case KEY_1:					return (char) 173; // ¡
		case KEY_2:					return (char) 0; // ⅛
		case KEY_3:					return (char) 156; // £
		case KEY_4:					return '$';
		case KEY_5:					return (char) 0; // ⅜
		case KEY_6:					return (char) 0; // ⅝
		case KEY_7:					return (char) 0; // ⅞
		case KEY_8:					return (char) 0; // ™
		case KEY_9:					return (char) 241; // ±
		case KEY_0:					return (char) 248; // °
		case KEY_MINUS:				return (char) 168; // ¿
		case KEY_EQUAL:				return (char) 0; // ˛
		case KEY_TAB:				return '\t';
		case KEY_Q:					return (char) 146; // Æ
		case KEY_W:					return '<';
		case KEY_E:					return (char) 155; // ¢
		case KEY_R:					return (char) 0; // ®
		case KEY_T:					return (char) 0; // Ŧ
		case KEY_Y:					return (char) 157; // ¥
		case KEY_U:					return (char) 0; // ↑
		case KEY_I:					return (char) 0; // ı
		case KEY_O:					return (char) 0; // Ø
		case KEY_P:					return (char) 0; // Þ
		case KEY_LBRACKET:			return (char) 248; // °
		case KEY_RBRACKET:			return (char) 0; // ¯
		case KEY_ENTER:				return '\n'; // enter
		case KEY_A:					return (char) 234; // Ω
		case KEY_S:					return (char) 0; // ẞ
		case KEY_D:					return (char) 0; // Ð
		case KEY_F:					return (char) 0; // ª
		case KEY_G:					return (char) 0; // Ŋ
		case KEY_H:					return (char) 0; // Ħ
		case KEY_J:					return (char) 0;
		case KEY_K:					return '&';
		case KEY_L:					return (char) 0; // Ł
		case KEY_SEMICOLON:			return (char) 248; // º
		case KEY_QUOTE:				return (char) 0; // ˇ
		case KEY_GRAVE:				return (char) 170; // ¬
		case KEY_BACKSLASH:			return (char) 0; // ˘
		case KEY_Z:					return (char) 0; // Ł
		case KEY_X:					return '>';
		case KEY_C:					return (char) 0; // ©
		case KEY_V:					return (char) 0; // ‚
		case KEY_B:					return (char) 0; // ‘
		case KEY_N:					return (char) 0; // ’
		case KEY_M:					return (char) 0; // ˝
		case KEY_COMMA:				return (char) 0; // ×
		case KEY_DOT:				return (char) 246; // ÷
		case KEY_SLASH:				return (char) 0; // ˙
		case KEY_NUMPAD_ASTERISK:	return '*';
		case KEY_SPACE:				return ' ';
		case KEY_NUMPAD_7:			return '7'; // Keypad 7 OR Home
		case KEY_NUMPAD_8:			return '8'; // Keypad 8 OR Up
		case KEY_NUMPAD_9:			return '9'; // Keypad 9 OR Page Up
		case KEY_NUMPAD_MINUS:		return '-'; // Keypad -
		case KEY_NUMPAD_4:			return '4'; // Keypad 4 OR Left
		case KEY_NUMPAD_5:			return '5'; // Keypad 5
		case KEY_NUMPAD_6:			return '6'; // Keypad 6 OR Right
		case KEY_NUMPAD_PLUS:		return '+'; // Keypad +
		case KEY_NUMPAD_1:			return '1'; // Keypad 1 OR End
		case KEY_NUMPAD_2:			return '2'; // Keypad 2 OR Down
		case KEY_NUMPAD_3:			return '3'; // Keypad 3 OR Page Down
		case KEY_NUMPAD_0:			return '0'; // Keypad 0 OR Insert
		case KEY_NUMPAD_DOT:		return '.'; // Keypad . OR Del
		case KEY_LESSTHAN:			return (char) 0; // ¦
		case KEY_NUMPAD_ENTER:		return '\n';
		case KEY_NUMPAD_SLASH:		return '/';
		default:
			return (char) 0;
	}
}

static char getChar_AZERTY_AltGr(keycode_t k){
		switch (k){
		case KEY_RESERVED:			return (char) 0;
		case KEY_1:					return (char) 0; // ¹
		case KEY_2:					return '~';
		case KEY_3:					return '#';
		case KEY_4:					return '{';
		case KEY_5:					return '[';
		case KEY_6:					return '|';
		case KEY_7:					return '`';
		case KEY_8:					return '\\';
		case KEY_9:					return '^';
		case KEY_0:					return '@';
		case KEY_MINUS:				return ']';
		case KEY_EQUAL:				return '}';
		case KEY_TAB:				return '\t';
		case KEY_Q:					return (char) 145; // æ
		case KEY_W:					return (char) 174; // «
		case KEY_E:					return (char) 0; // €
		case KEY_R:					return (char) 0; // ¶
		case KEY_T:					return (char) 0; // ŧ
		case KEY_Y:					return (char) 0; // ←
		case KEY_U:					return (char) 0; // ↓
		case KEY_I:					return (char) 0; // →
		case KEY_O:					return (char) 0; // ø
		case KEY_P:					return (char) 0; // þ
		case KEY_LBRACKET:			return (char) 0; // ¨
		case KEY_RBRACKET:			return (char) 0; // ¤
		case KEY_ENTER:				return '\n'; // enter
		case KEY_A:					return '@';
		case KEY_S:					return (char) 0; // ß
		case KEY_D:					return (char) 0; // ð
		case KEY_F:					return (char) 0; // đ
		case KEY_G:					return (char) 0; // ŋ
		case KEY_H:					return (char) 0; // ħ
		case KEY_J:					return (char) 0; // ̉
		case KEY_K:					return (char) 0; // ĸ
		case KEY_L:					return (char) 0; // ł
		case KEY_SEMICOLON:			return (char) 230; // µ
		case KEY_QUOTE:				return '^';
		case KEY_GRAVE:				return (char) 170; // ¬
		case KEY_BACKSLASH:			return '`';
		case KEY_Z:					return (char) 0; // ł
		case KEY_X:					return (char) 175; // »
		case KEY_C:					return (char) 155; // ¢
		case KEY_V:					return (char) 0; // „
		case KEY_B:					return (char) 0; // “
		case KEY_N:					return (char) 0; // ”
		case KEY_M:					return (char) 0; // ´ (as in ó)
		case KEY_COMMA:				return (char) 249; // •
		case KEY_DOT:				return (char) 250; // ·
		case KEY_SLASH:				return (char) 0;
		case KEY_NUMPAD_ASTERISK:	return '*';
		case KEY_SPACE:				return ' ';
		case KEY_NUMPAD_7:			return '7'; // Keypad 7 OR Home
		case KEY_NUMPAD_8:			return '8'; // Keypad 8 OR Up
		case KEY_NUMPAD_9:			return '9'; // Keypad 9 OR Page Up
		case KEY_NUMPAD_MINUS:		return '-'; // Keypad -
		case KEY_NUMPAD_4:			return '4'; // Keypad 4 OR Left
		case KEY_NUMPAD_5:			return '5'; // Keypad 5
		case KEY_NUMPAD_6:			return '6'; // Keypad 6 OR Right
		case KEY_NUMPAD_PLUS:		return '+'; // Keypad +
		case KEY_NUMPAD_1:			return '1'; // Keypad 1 OR End
		case KEY_NUMPAD_2:			return '2'; // Keypad 2 OR Down
		case KEY_NUMPAD_3:			return '3'; // Keypad 3 OR Page Down
		case KEY_NUMPAD_0:			return '0'; // Keypad 0 OR Insert
		case KEY_NUMPAD_DOT:		return '.'; // Keypad . OR Del
		case KEY_LESSTHAN:			return '|';
		case KEY_NUMPAD_ENTER:		return '\n';
		case KEY_NUMPAD_SLASH:		return '/';
		default:
			return (char) 0;
	}
}

static char getChar_AZERTY_Shift(keycode_t k){
	switch (k){
		case KEY_RESERVED:			return (char) 0;
		case KEY_1:					return '1';
		case KEY_2:					return '2';
		case KEY_3:					return '3';
		case KEY_4:					return '4';
		case KEY_5:					return '5';
		case KEY_6:					return '6';
		case KEY_7:					return '7';
		case KEY_8:					return '8';
		case KEY_9:					return '9';
		case KEY_0:					return '0';
		case KEY_MINUS:				return (char) 248; // °
		case KEY_EQUAL:				return '+';
		case KEY_TAB:				return (char) 0;
		case KEY_Q:					return 'A';
		case KEY_W:					return 'Z';
		case KEY_E:					return 'E';
		case KEY_R:					return 'R';
		case KEY_T:					return 'T';
		case KEY_Y:					return 'Y';
		case KEY_U:					return 'U';
		case KEY_I:					return 'I';
		case KEY_O:					return 'O';
		case KEY_P:					return 'P';
		case KEY_LBRACKET:			return (char) 0; // ¨ not available in ascii
		case KEY_RBRACKET:			return (char) 156; // £
		case KEY_ENTER:				return '\n';
		case KEY_A:					return 'Q';
		case KEY_S:					return 'S';
		case KEY_D:					return 'D';
		case KEY_F:					return 'F';
		case KEY_G:					return 'G';
		case KEY_H:					return 'H';
		case KEY_J:					return 'J';
		case KEY_K:					return 'K';
		case KEY_L:					return 'L';
		case KEY_SEMICOLON:			return 'M';
		case KEY_QUOTE:				return '%'; // ù
		case KEY_GRAVE:				return '~'; // ²
		case KEY_BACKSLASH:			return (char) 230; // µ
		case KEY_Z:					return 'W';
		case KEY_X:					return 'X';
		case KEY_C:					return 'C';
		case KEY_V:					return 'V';
		case KEY_B:					return 'B';
		case KEY_N:					return 'N';
		case KEY_M:					return '?';
		case KEY_COMMA:				return '.';
		case KEY_DOT:				return '/';
		case KEY_SLASH:				return (char) 0; // § not available in ascii
		case KEY_NUMPAD_ASTERISK:	return '*';
		case KEY_SPACE:				return ' ';
		case KEY_NUMPAD_7:			return '7';
		case KEY_NUMPAD_8:			return '8';
		case KEY_NUMPAD_9:			return '9';
		case KEY_NUMPAD_MINUS:		return '-';
		case KEY_NUMPAD_4:			return '4';
		case KEY_NUMPAD_5:			return '5';
		case KEY_NUMPAD_6:			return '6';
		case KEY_NUMPAD_PLUS:		return '+';
		case KEY_NUMPAD_1:			return '1';
		case KEY_NUMPAD_2:			return '2';
		case KEY_NUMPAD_3:			return '3';
		case KEY_NUMPAD_0:			return '0';
		case KEY_NUMPAD_DOT:		return '.';
		case KEY_LESSTHAN:			return '>';
		case KEY_NUMPAD_ENTER:		return '\n';
		case KEY_NUMPAD_SLASH:		return '/';
		default:
			return (char) 0;
	}
}

// Returns the ASCII char corresponding to the keycode, in AZERTY (Q is A, W is Z...)
// If 'keycode' is non printable, returns 0
static char getChar_AZERTY(keycode_t k){
	// Only shift
	if ((m_leftShiftPressed || m_rightShiftPressed) && !m_rightAltPressed && !m_capslockOn)
		return getChar_AZERTY_Shift(k);

	// Only AltGr
	if (m_rightAltPressed && !m_leftShiftPressed && !m_rightShiftPressed)
		return getChar_AZERTY_AltGr(k);

	// Shift + AltGr
	if ((m_leftShiftPressed || m_rightShiftPressed) && m_rightAltPressed)
		return getChar_AZERTY_ShiftAltGr(k);

	// Caps lock (it gets overriden by all the modifier keys up there, except shift)
	if (m_capslockOn && !m_leftShiftPressed && !m_rightShiftPressed)
		return getChar_AZERTY_CapsLock(k);

	// None or Ctrl or LAlt or Meta or CapsLock+Shift
	return getChar_AZERTY_NoModifier(k);
}

// ================ Public API ================

static inline int findAvailableCallbackIndex(){
	for (int i=0 ; i<MAX_CALLBACKS ; i++){
		if (m_callbacks[i] == NULL){
			return i;
		}
	}
	return -1;
}

static inline void executeCallbacks(keycode_t keycode, int character, enum KeypressMode mode, uint8_t modifierKeys){
	for (int i=0 ; i<MAX_CALLBACKS ; i++){
		if (m_callbacks[i] == NULL) continue;
		m_callbacks[i](keycode, character, mode, modifierKeys);
	}
}

static void keyCallback_printKey(keycode_t keycode, int character, enum KeypressMode mode, uint8_t){
	const char* mode_string = (mode == KB_KEYMODE_KEY_PRESSED) ? "pressed " : "released";

	// Non-printable key
	if (character == 0){
		debug("key %s %#.2x '%s'", mode_string, keycode, Keyboard_getKeyString(keycode));
		return;
	}

	// Enter ('\n') is printable, but we don't actually want to put a line feed in the output
	if (character == '\n'){
		debug("key %s %#.2x '\\n'", mode_string, keycode);
		return;
	}
	if (character == '\t'){
		debug("key %s %#.2x '\\t'", mode_string, keycode);
		return;
	}

	debug("key %s %#.2x '%c'", mode_string, keycode, character);
}

bool Keyboard_registerKeyCallback(keycallback_t callback){
	int index = findAvailableCallbackIndex();
	if (index == -1) return false;

	m_callbacks[index] = callback;
	return true;
}

void Keyboard_unregisterKeyCallback(keycallback_t callback){
	for (int i=0 ; i<MAX_CALLBACKS ; i++){
		if (m_callbacks[i] == callback){
			m_callbacks[i] = NULL;
		}
	}
}

const char* Keyboard_getKeyString(keycode_t keycode){
	switch (keycode){
		case KEY_ESC:			return "ESC";
		case KEY_BACKSPACE: 	return "BACKSPACE";
		case KEY_LCTRL:			return "LCTRL";
		case KEY_LSHIFT:		return "LSHIFT";
		case KEY_RSHIFT:		return "RSHIFT";
		case KEY_LALT:			return "LALT";
		case KEY_CAPSLOCK:		return "CAPSLOCK";
		case KEY_F1: 			return "F1";
		case KEY_F2: 			return "F2";
		case KEY_F3: 			return "F3";
		case KEY_F4: 			return "F4";
		case KEY_F5: 			return "F5";
		case KEY_F6: 			return "F6";
		case KEY_F7: 			return "F7";
		case KEY_F8: 			return "F8";
		case KEY_F9: 			return "F9";
		case KEY_F10: 			return "F10";
		case KEY_NUMLOCK: 		return "NUMLOCK";
		case KEY_SCROLLLOCK: 	return "SCROLLLOCK";
		case KEY_F11: 			return "F11";
		case KEY_F12: 			return "F12";
		case KEY_RCTRL:			return "RCTRL";
		case KEY_PRINTSCREEN: 	return "PRINTSCREEN";
		case KEY_RALT:			return "RALT";
		case KEY_HOME: 			return "HOME";
		case KEY_UP: 			return "UP";
		case KEY_PAGEUP: 		return "PAGEUP";
		case KEY_LEFT: 			return "LEFT";
		case KEY_RIGHT: 		return "RIGHT";
		case KEY_END: 			return "END";
		case KEY_DOWN: 			return "DOWN";
		case KEY_PAGEDOWN: 		return "PAGEDOWN";
		case KEY_INSERT: 		return "INSERT";
		case KEY_DELETE: 		return "DELETE";
		case KEY_MUTE:			return "MUTE";
		case KEY_VOLUMEDOWN:	return "VOLUMEDOWN";
		case KEY_VOLUMEUP:		return "VOLUMEUP";
		case KEY_POWER:			return "POWER";
		case KEY_PAUSE: 		return "PAUSE";
		case KEY_LMETA:			return "LMETA";
		case KEY_RMETA:			return "RMETA";
		case KEY_STOP:			return "STOP";
		case KEY_COPY:			return "COPY";
		case KEY_PASTE:			return "PASTE";
		case KEY_CUT:			return "CUT";
		case KEY_MENU:			return "MENU";
		case KEY_CALC:			return "CALCULATOR";
		case KEY_SLEEP:			return "SLEEP";
		case KEY_WAKEUP:		return "WAKEUP";
		case KEY_MAIL:			return "MAIL";
		case KEY_BOOKMARKS:		return "BOOKMARKS";
		case KEY_COMPUTER:		return "COMPUTER";
		case KEY_BACK:			return "BACK";
		case KEY_FORWARD:		return "FORWARD";
		case KEY_NEXTSONG:		return "NEXTSONG";
		case KEY_PLAYPAUSE:		return "PLAYPAUSE";
		case KEY_PREVIOUSSONG:	return "PREVIOUSSONG";
		case KEY_STOPCD:		return "STOPSONG";
		case KEY_HOMEPAGE:		return "HOMEPAGE";
		case KEY_REFRESH:		return "REFRESH";
		case KEY_F13: 			return "F13";
		case KEY_F14: 			return "F14";
		case KEY_F15: 			return "F15";
		case KEY_F17: 			return "F17";
		case KEY_F18: 			return "F18";
		case KEY_F19: 			return "F19";
		case KEY_F20: 			return "F20";
		case KEY_F21: 			return "F21";
		case KEY_F22: 			return "F22";
		case KEY_F23: 			return "F23";
		case KEY_F24: 			return "F24";
		case KEY_SEARCH:		return "SEARCH";
		case KEY_MEDIA:			return "MEDIA";
	}

	return NULL;
}

void Keyboard_init(){
	m_modifierKeys = 0;
	m_leftShiftPressed = false;
	m_rightShiftPressed = false;
	m_numlockOn = true;
	m_capslockOn = false;
	m_modifierKeys = KB_MODIFIER_NUMLOCK;

	for (int i=0 ; i<11 ; i++){
		m_numlockKeyWasFunction[i] = false;
	}

	for (int i=0 ; i<MAX_CALLBACKS ; i++){
		m_callbacks[i] = NULL;
	}

	Keyboard_registerKeyCallback(keyCallback_printKey);
	log(SUCCESS, MODULE, "Initialization success");
}

void Keyboard_notifyReleased(keycode_t keycode){
	switch (keycode){
		case KEY_LCTRL:
			m_rightCtrlPressed = false;
			break;
		case KEY_LSHIFT:
			m_leftShiftPressed = false;
			break;
		case KEY_RSHIFT:
			m_rightShiftPressed = false;
			break;
		case KEY_LALT:
			m_leftAltPressed = false;
			break;
		case KEY_NUMLOCK: // it is actually flipped when key is released ! who knows why
			m_numlockOn = !m_numlockOn;
			m_modifierKeys ^= KB_MODIFIER_NUMLOCK;
			break;
		case KEY_RCTRL:
			m_rightCtrlPressed = false;
			break;
		case KEY_RALT:
			m_rightAltPressed = false;
			break;
		case KEY_LMETA:
			m_leftMetaPressed = false;
			break;
		case KEY_RMETA:
			m_rightMetaPressed = false;
			break;
		default:
			break;
	}

	// No shift pressed: clear shift flag
	if (!(m_leftShiftPressed || m_rightShiftPressed)){
		m_modifierKeys &= ~KB_MODIFIER_SHIFT;
	}
	// Ctrl
	if (!(m_leftCtrlPressed || m_rightCtrlPressed)){
		m_modifierKeys &= ~KB_MODIFIER_CTRL;
	}
	// Alt
	if (!(m_leftAltPressed || m_rightAltPressed)){
		m_modifierKeys &= ~KB_MODIFIER_ALT;
	}
	// Meta
	if (!(m_leftMetaPressed || m_rightMetaPressed)){
		m_modifierKeys &= ~KB_MODIFIER_META;
	}

	// Handle numpad key (translate it for break if it was translated for make)
	if (isNumpadKey(keycode)){
		// Note: Numpad 5 has no function associated so we just don't send it
		switch (keycode){
			case KEY_NUMPAD_7:
				if (m_numlockKeyWasFunction[0] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[0] = false;
				}
				break;
			case KEY_NUMPAD_8:
				if (m_numlockKeyWasFunction[1] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[1] = false;
				}
				break;
			case KEY_NUMPAD_9:
				if (m_numlockKeyWasFunction[2] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[2] = false;
				}
				break;
			case KEY_NUMPAD_4:
				if (m_numlockKeyWasFunction[3] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[3] = false;
				}
				break;
			case KEY_NUMPAD_5:
				if (m_numlockKeyWasFunction[4]){
					m_numlockKeyWasFunction[4] = false;
					return;
				}
				break;
			case KEY_NUMPAD_6:
				if (m_numlockKeyWasFunction[5] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[5] = false;
				}
				break;
			case KEY_NUMPAD_1:
				if (m_numlockKeyWasFunction[6] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[6] = false;
				}
				break;
			case KEY_NUMPAD_2:
				if (m_numlockKeyWasFunction[7] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[7] = false;
				}
				break;
			case KEY_NUMPAD_3:
				if (m_numlockKeyWasFunction[8] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[8] = false;
				}
				break;
			case KEY_NUMPAD_0:
				if (m_numlockKeyWasFunction[9] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[9] = false;
				}
				break;
			case KEY_NUMPAD_DOT:
				if (m_numlockKeyWasFunction[10] || !m_numlockOn){
					keycode = getNumpadKey_NoNumluck(keycode);
					m_numlockKeyWasFunction[10] = false;
				}
				break;
		}
	}

	char c = getChar_AZERTY(keycode);
	executeCallbacks(keycode, (int) c, KB_KEYMODE_KEY_RELEASED, m_modifierKeys);
}

void Keyboard_notifyPressed(keycode_t keycode){
	switch (keycode){
		case KEY_LCTRL:
			m_rightCtrlPressed = true;
			break;
		case KEY_LSHIFT:
			m_leftShiftPressed = true;
			break;
		case KEY_RSHIFT:
			m_rightShiftPressed = true;
			break;
		case KEY_LALT:
			m_leftAltPressed = true;
			break;
		// case KEY_NUMLOCK: // it is actually flipped when key is released ! who knows why
		case KEY_CAPSLOCK:
			m_capslockOn = !m_capslockOn;
			m_modifierKeys ^= KB_MODIFIER_CAPSLOCK;
			break;
		case KEY_RCTRL:
			m_rightCtrlPressed = true;
			break;
		case KEY_RALT:
			m_rightAltPressed = true;
			break;
		case KEY_LMETA:
			m_leftMetaPressed = true;
			break;
		case KEY_RMETA:
			m_rightMetaPressed = true;
			break;
		default:
			break;
	}

	// Shift pressed: set shift flag
	if (m_leftShiftPressed || m_rightShiftPressed){
		m_modifierKeys |= KB_MODIFIER_SHIFT;
	}
	// Ctrl
	if (m_leftCtrlPressed || m_rightCtrlPressed){
		m_modifierKeys |= KB_MODIFIER_CTRL;
	}
	// Alt
	if (m_leftAltPressed || m_rightAltPressed){
		m_modifierKeys |= KB_MODIFIER_ALT;
	}
	// Meta
	if (m_leftMetaPressed || m_rightMetaPressed){
		m_modifierKeys |= KB_MODIFIER_META;
	}

	// Handle numpad key
	//   shift => numpad_no_numlock
	//   ctrl  => numpad_no_numlock
	//   else  => (numlock) ? numpad_numlock : numpad_no_numlock
	if (isNumpadKey(keycode)){
		if (m_leftShiftPressed || m_rightShiftPressed || m_leftCtrlPressed || m_rightCtrlPressed || !m_numlockOn){
			// Note: Numpad 5 has no function associated so we just don't send it
			switch (keycode){
				case KEY_NUMPAD_7: m_numlockKeyWasFunction[0] = true; break;
				case KEY_NUMPAD_8: m_numlockKeyWasFunction[1] = true; break;
				case KEY_NUMPAD_9: m_numlockKeyWasFunction[2] = true; break;
				case KEY_NUMPAD_4: m_numlockKeyWasFunction[3] = true; break;
				case KEY_NUMPAD_5: m_numlockKeyWasFunction[4] = true; return;
				case KEY_NUMPAD_6: m_numlockKeyWasFunction[5] = true; break;
				case KEY_NUMPAD_1: m_numlockKeyWasFunction[6] = true; break;
				case KEY_NUMPAD_2: m_numlockKeyWasFunction[7] = true; break;
				case KEY_NUMPAD_3: m_numlockKeyWasFunction[8] = true; break;
				case KEY_NUMPAD_0: m_numlockKeyWasFunction[9] = true; break;
				case KEY_NUMPAD_DOT: m_numlockKeyWasFunction[10] = true; break;
			}
			keycode = getNumpadKey_NoNumluck(keycode);
		}
	}

	char c = getChar_AZERTY(keycode);
	executeCallbacks(keycode, (int) c, KB_KEYMODE_KEY_PRESSED, m_modifierKeys);
}

void Keyboard_notifySysRq(){
	// For now, sysrq just enables keyboard print
	static bool callbackOn = true;

	callbackOn ?
		Keyboard_unregisterKeyCallback(keyCallback_printKey) :
		Keyboard_registerKeyCallback(keyCallback_printKey);

	callbackOn = !callbackOn;
}
