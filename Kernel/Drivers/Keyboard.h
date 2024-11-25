#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>
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

#define KB_CALLBACK_KEY_PRESSED		0x01
// #define KB_CALLBACK_KEY_REPEAT		0x02
#define KB_CALLBACK_KEY_RELEASED	0x03

/// @brief Callback function type `void key_callback(int @p keycode, int @p character, uint8_t @p mode, uint8_t @p modifier_keys)`
/// @param keycode The keycode for which the key was pressed (see Keycodes.h)
/// @param character The ASCII character corresponding to the key (0 if non-printable)
/// @param mode Key mode, either KB_CALLBACK_KEY_PRESSED (or KB_CALLBACK_KEY_REPEAT [not supported yet]) or KB_CALLBACK_KEY_RELEASED
/// @param modifier_keys Flags of modifiers. Bit set (=1) means modifier is pressed (see `KB_MODIFIER_*` for bits descriptions)
/// @returns void
typedef void(*KeyCallback)(int keycode, int character, uint8_t mode, uint8_t modifier_keys);

/// @brief Adds a KeyCallback @p callback function as callback (see KeyCallback for parameters description)
/// @returns True on success, false on error (callback array full)
bool Keyboard_registerKeyCallback(KeyCallback callback);

/// @brief Removes a KeyCallback @p callback function from the internal array
/// @returns void
void Keyboard_unregisterKeyCallback(KeyCallback callback);

// Internal use
void Keyboard_initalize();
void Keyboard_notifyReleased(Keycode keycode);
void Keyboard_notifyPressed(Keycode keycode);
void Keyboard_NotifySysRq(); // Notify SysRq key pressed (special case)

#endif
