/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * KeyManager.cpp: Keyboard manager.                                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "KeyManager.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

// Static class variables.
bool KeyManager::ms_KeyPress[KEYV_LAST];


/**
 * Init(): Initialize the Key Manager.
 */
void KeyManager::Init(void)
{
	// Clear the keypress array.
	memset(ms_KeyPress, 0x00, sizeof(ms_KeyPress));
}


/**
 * End(): Shut down the Key Manager.
 */
void KeyManager::End(void)
{
}


/**
 * OS-specific key names.
 */
#if defined(__APPLE__)
#define RETURN_KEYNAME "Return"
#define SUPER_KEYNAME "Command"
#elif defined(_WIN32)
#define RETURN_KEYNAME "Enter"
#define SUPER_KEYNAME "Win"
#else
#define RETURN_KEYNAME "Enter"
#define SUPER_KEYNAME "Super"
#endif

/**
 * GetKeyName(): Get the name of the given key.
 * @param key Key value.
 * @return Key name, or NULL if the key is invalid.
 */
const char *KeyManager::GetKeyName(KeyVal key)
{
	static const char *KeyNames[KEYV_LAST] =
	{
		// 0x00
		"Unknown", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		"Backspace", "Tab", NULL, NULL, "Clear", RETURN_KEYNAME, NULL, NULL,
		NULL, NULL, NULL, "Pause", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "Escape", NULL, NULL, NULL, NULL,
		
		// 0x20
		"Space", "!", "\"", "#", "$", "%", "&", "'",
		"(", ")", "*", "+", ",", "-", ".", "/",
		"0", "1", "2", "3", "4", "5", "6", "7",
		"8", "9", ":", ";", "<", "=", ">", "?",
		
		// 0x40
		"@", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "[", "\\", "]", "_", "^",
		
		// 0x60
		"`", "A", "B", "C", "D", "E", "F", "G",
		"H", "I", "J", "K", "L", "M", "N", "O",
		"P", "Q", "R", "S", "T", "U", "V", "W",
		"X", "Y", "Z", NULL, NULL, NULL, NULL, "Delete",
		
		// 0x80
		"Numpad 0", "Numpad 1", "Numpad 2", "Numpad 3",
		"Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7",
		"Numpad 8", "Numpad 9", "Numpad .", "Numpad /",
		"Numpad *", "Numpad -", "Numpad +", "Numpad Enter",
		
		// 0x90
		"Numpad =", "Up", "Down", "Right",
		"Left", "Insert", "Home", "End",
		"Page Up", "Page Down", NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		/** @name 0xA0: Function keys */
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
		"F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
		"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24",
		"F25", "F26", "F27", "F28", "F29", "F30", "F31", "F32",
		
		/** @name 0xC0: Key state modifier keys */
		"Num Lock", "Caps Lock", "Scroll Lock", "Left Shift",
		"Right Shift", "Left Ctrl", "Right Ctrl", "Left Alt",
		"Right Alt", "Left Meta", "Right Meta", "Left " SUPER_KEYNAME,
		"Right " SUPER_KEYNAME, "Alt-Gr", "Compose", "Left Hyper",
		
		/** @name 0xD0: Key state modifier keys (continued) */
		"Right Hyper", "Left Direction", "Right Direction", NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		/** @name 0xE0: Miscellaneous function keys */
		"Help", "Print Screen", "SysRq", "Break",
		"Menu", "Power", "Euro", "Undo",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		
		/** @name 0xF0: Reserved */
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		
		/** @name 0x100: Multimedia/Internet keys */
		// NOTE: Only back and forward are implemented,
		// since ThinkPads have them as real keys.
		"Back", "Forward"
	};
	
	if (key < 0 || key >= KEYV_LAST)
		return NULL;
	
	return KeyNames[key];
}


/**
 * KeyPressEvent(): Key press event.
 * @param key KeyVal keycode.
 */
void KeyManager::KeyPressEvent(int key)
{
	if (key < KEYV_UNKNOWN || key >= KEYV_LAST)
		return;
	ms_KeyPress[key] = 1;
}


/**
 * KeyReleaseEvent(): Key release event.
 * @param key KeyVal keycode.
 */
void KeyManager::KeyReleaseEvent(int key)
{
	if (key < KEYV_UNKNOWN || key >= KEYV_LAST)
		return;
	ms_KeyPress[key] = 0;
}

}
