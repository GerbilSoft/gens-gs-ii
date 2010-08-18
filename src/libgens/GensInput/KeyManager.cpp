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

// Keyboard device.
// TODO: Multiple keyboard support?
Keyboard KeyManager::ms_Keyboard;

/**
 * Init(): Initialize the Key Manager.
 */
void KeyManager::Init(void)
{
	// TODO
}


/**
 * End(): Shut down the Key Manager.
 */
void KeyManager::End(void)
{
	// TODO
}


/**
 * GetKeyName(): Get the name of the given key.
 * @param key Key value.
 * @return Key name, or NULL if the key is invalid.
 */
const char *KeyManager::GetKeyName(GensKey_t key)
{
	GensKey_u gkey;
	gkey.keycode = key;
	
	// TODO: Other device types.
	switch (gkey.type)
	{
		case GKT_KEYBOARD:
			return Keyboard::GetKeyName(gkey.key16);
		
		default:
			return NULL;
	}
}


/**
 * KeyPressEvent(): Key press event.
 * @param key KeyVal keycode.
 */
void KeyManager::KeyPressEvent(GensKey_t key)
{
	GensKey_u gkey;
	gkey.keycode = key;
	
	// TODO: Other device types.
	switch (gkey.type)
	{
		case GKT_KEYBOARD:
			// TODO: Multiple keyboard support.
			ms_Keyboard.keyPressEvent(gkey.key16);
			break;
		
		default:
			break;
	}
}


/**
 * KeyReleaseEvent(): Key release event.
 * @param key KeyVal keycode.
 */
void KeyManager::KeyReleaseEvent(GensKey_t key)
{
	GensKey_u gkey;
	gkey.keycode = key;
	
	// TODO: Other device types.
	switch (gkey.type)
	{
		case GKT_KEYBOARD:
			// TODO: Multiple keyboard support.
			ms_Keyboard.keyReleaseEvent(gkey.key16);
			break;
		
		default:
			break;
	}
}


/**
 * MousePressEvent(): Mouse press event.
 * @param button MouseButton number.
 */
void KeyManager::MousePressEvent(int button)
{
	if (button < MBTN_UNKNOWN || button >= MBTN_LAST)
		return;
	
	// TODO: Make a separate mouse device?
	ms_Keyboard.keyPressEvent(KEYV_MOUSE_UNKNOWN + button);
}


/**
 * MouseReleaseEvent(): Mouse release event.
 * @param button MouseButton number.
 */
void KeyManager::MouseReleaseEvent(int button)
{
	if (button < MBTN_UNKNOWN || button >= MBTN_LAST)
		return;
	
	// TODO: Make a separate mouse device?
	ms_Keyboard.keyReleaseEvent(KEYV_MOUSE_UNKNOWN + button);
}


/**
 * IsKeyPressed(): Check if a key is pressed.
 * @param key Gens keycode.
 */
bool KeyManager::IsKeyPressed(GensKey_t key)
{
	GensKey_u gkey;
	gkey.keycode = key;
	
	if (gkey.type == 0)
	{
		// Keyboard input.
		// TODO: Multiple keyboard support.
		if (gkey.dev_id == 0)
			return (ms_Keyboard.isKeyPressed(gkey.key16));
	}
	
	// TODO: Other types of input.
	return false;
}

}
