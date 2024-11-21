#include "stdio.h"
#include "Drivers/Keycodes.h"

#include "Keyboard.h"

// Returns the ascii char corresponding to the keycode, in AZERTY (Q is A, W is Z...)
// If 'keycode' is non printable, returns 0
char getChar_AZERTY_NoModifier(Keycode keycode){
	const int NON_PRINTABLE = 0;

	switch (keycode){
		case KEY_RESERVED:			return NON_PRINTABLE;
		case KEY_1:					return '&';
		case KEY_2:					return 130; // é
		case KEY_3:					return '"';
		case KEY_4:					return '\'';
		case KEY_5:					return '(';
		case KEY_6:					return '-';
		case KEY_7:					return 138; // è
		case KEY_8:					return '_';
		case KEY_9:					return 135; // ç
		case KEY_0:					return 133; // à
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
		case KEY_QUOTE:				return 163; // ù
		case KEY_GRAVE:				return 253; // ²
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

		default:
			return NON_PRINTABLE;
	}
}

// TODO
void Keyboard_notifyReleased(Keycode keycode){
	printf("[Keyboard driver] Key %p released\n", keycode);
}

// TODO
void Keyboard_notifyPressed(Keycode keycode){
	printf("[Keyboard driver] Key %p pressed\n", keycode);

	// if (keycode == KEY_NUMLOCK){
	// 	puts("numlock hit");
	// 	return;
	// }

	// char c = getChar_AZERTY_NoModifier(keycode);
	// if (c == 0){
	// 	printf("%p ", keycode);
	// 	return;
	// }

	// putc(c);
}
