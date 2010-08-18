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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

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
		
		static bool IsKeyPressed(GensKey_t key);
		
#ifdef _WIN32
		// QWidget doesn't properly differentiate L/R modifier keys,
		// and neither do WM_KEYDOWN/WM_KEYUP.
		static inline void WinKeySet(GensKey_t key, int virtKey)
		{
			if (key < KEYV_UNKNOWN && key >= KEYV_LAST)
				return;
			ms_KeyPress[key] = !!(GetAsyncKeyState(virtKey) & 0x8000);
		}
#endif /* _WIN32 */
	
	protected:
		static Keyboard ms_Keyboard;
	
	private:
		KeyManager() { }
		~KeyManager() { }
};

}

#endif /* __LIBGENS_IO_KEYMANAGER_HPP__ */
