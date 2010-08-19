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

// Win32 includes.
// Needed for getting L/R modifier key state.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

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


/**
 * Update(): Update the GensInput subsystem.
 * This polls joysticks and Wii Remotes.
 * On Win32, it also polls left/right virtual keys.
 */
void KeyManager::Update(void)
{
#ifdef _WIN32
	// Update Shift/Control/Alt states.
	// TODO: Only do this if the input backend doesn't support L/R modifiers natively.
	// QWidget doesn't; GLFW does.
	// TODO: When should these key states be updated?
	// - Beginning of frame.
	// - Before VBlank.
	// - End of frame.
	ms_Keyboard.setKeyState(KEYV_LSHIFT,	(!!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)));
	ms_Keyboard.setKeyState(KEYV_RSHIFT,	(!!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)));
	ms_Keyboard.setKeyState(KEYV_LCTRL,	(!!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)));
	ms_Keyboard.setKeyState(KEYV_RCTRL,	(!!(GetAsyncKeyState(VK_RCONTROL) & 0x8000)));
	ms_Keyboard.setKeyState(KEYV_LALT,	(!!(GetAsyncKeyState(VK_LMENU) & 0x8000)));
	ms_Keyboard.setKeyState(KEYV_RALT,	(!!(GetAsyncKeyState(VK_RMENU) & 0x8000)));
#endif
	
	// TODO: Joysticks, Wii Remotes.
}

}
