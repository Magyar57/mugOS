#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include "Keycodes.h"

// Keyboard driver/subsystem
// It handles keycodes so that the application registering to
// the service only has to handle multiple character keys
// Aka, combinations ^+o=ô has to be client-handled.

#define KB_MODIFIER_CTRL			0b00000001
#define KB_MODIFIER_SHIFT			0b00000010
#define KB_MODIFIER_ALT				0b00000100
#define KB_MODIFIER_META			0b00001000
#define KB_MODIFIER_NUMLOCK			0b00010000
#define KB_MODIFIER_CAPSLOCK		0b00100000

enum KeypressMode {
	KB_KEYMODE_KEY_PRESSED,
	KB_KEYMODE_KEY_REPEAT,
	KB_KEYMODE_KEY_RELEASED
};

/// @brief Callback function type `void key_callback(int @p keycode, int @p character, uint8_t @p mode, uint8_t @p modifier_keys)`
/// @param keycode The keycode for which the key was pressed (see Keycodes.h)
/// @param chr The ASCII character corresponding to the key (0 if non-printable)
/// @param mode KeyMode, either KB_KEYMODE_KEY_PRESSED (or KB_KEYMODE_KEY_REPEAT [never emitted yet]) or KB_KEYMODE_KEY_RELEASED
/// @param modifierKeys Flags of modifiers. Bit set (=1) means modifier is pressed (see `KB_MODIFIER_*` for bits descriptions)
typedef void(*keycallback_t)(keycode_t keycode, int chr, enum KeypressMode mode, uint8_t modifierKeys);

/// @brief Adds a keycallback_t `callback` function as callback (see keycallback_t for parameters description)
/// @returns True on success, false on error (callback array full)
bool Keyboard_registerKeyCallback(keycallback_t callback);

/// @brief Removes a keycallback_t @p callback function from the internal array
void Keyboard_unregisterKeyCallback(keycallback_t callback);

/// @brief Returns a string representing the input NON-PRINTABLE `keycode` argument (e.g.: KEY_RSHIFT => "RSHIFT")
const char* Keyboard_getKeyString(keycode_t keycode);

// Internal use
void Keyboard_init();
void Keyboard_notifyReleased(keycode_t keycode);
void Keyboard_notifyPressed(keycode_t keycode);
void Keyboard_notifySysRq();

#endif
