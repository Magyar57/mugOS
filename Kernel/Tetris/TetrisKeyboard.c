#include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
#include <stddef.h>
// #include <sys/time.h>
// #include <termio.h>
#include "Drivers/Keyboard.h"

#include "TetrisKeyboard.h"

static Keycode g_lastKeycodePressed;

static void onKey(int keycode, int character, uint8_t mode, uint8_t modifier_keys){
	if (mode == KB_CALLBACK_KEY_PRESSED)
		g_lastKeycodePressed = keycode;
	else if (mode == KB_CALLBACK_KEY_RELEASED)
		g_lastKeycodePressed = KEY_RESERVED;
}

void TetrisKeyboard_initalize(){
	Keyboard_registerKeyCallback(onKey);
	g_lastKeycodePressed = KEY_RESERVED;
}

Keycode TetrisKeyboard_getKey(){
	Keycode res = g_lastKeycodePressed;

	// hardcoded azerty
	if (g_lastKeycodePressed == KEY_SEMICOLON){
		res = KEY_M;
	}

	g_lastKeycodePressed = KEY_RESERVED; // Return only for initial press
	return res;
}
