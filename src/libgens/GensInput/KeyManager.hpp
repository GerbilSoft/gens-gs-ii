/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * KeyManager.hpp: Keyboard manager.                                       *
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

#ifndef __LIBGENS_IO_KEYMANAGER_HPP__
#define __LIBGENS_IO_KEYMANAGER_HPP__

#include "GensKey_t.h"

// Input devices.
#include "Keyboard.hpp"

namespace LibGens
{

class KeyManager
{
	public:
		static void Init(void);
		static void End(void);
		
		static const char *GetKeyName(GensKey_t key);
		
		static void KeyPressEvent(GensKey_t key);
		static void KeyReleaseEvent(GensKey_t key);
		
		static void MousePressEvent(int button);
		static void MouseReleaseEvent(int button);
		
		/**
		 * IsKeyPressed(): Check if a Gens Keycode is pressed.
		 * @param key Gens Keycode.
		 * @return True if the key is pressed; false if not.
		 */
		static bool IsKeyPressed(GensKey_t key);
		
		/**
		 * Update(): Update the GensInput subsystem.
		 * This polls joysticks and Wii Remotes.
		 * On Win32, it also polls left/right virtual keys.
		 */
		static void Update(void);
	
	protected:
		static Keyboard ms_Keyboard;
	
	private:
		KeyManager() { }
		~KeyManager() { }
};

}

#endif /* __LIBGENS_IO_KEYMANAGER_HPP__ */
