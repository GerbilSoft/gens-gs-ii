/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Keyboard.hpp: Keyboard class.                                           *
 * Represents an input device of type 0x00.                                *
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

#ifndef __LIBGENS_IO_KEYBOARD_HPP__
#define __LIBGENS_IO_KEYBOARD_HPP__

#include "GensKey_t.h"

// C includes.
#include <string.h>

namespace LibGens
{

class Keyboard
{
	public:
		Keyboard();
		~Keyboard();
		
		inline void keyPressEvent(uint16_t key16)
		{
			if (key16 >= KEYV_LAST)
				return;
			m_keyPress[key16] = true;
		}
		
		inline void keyReleaseEvent(uint16_t key16)
		{
			if (key16 >= KEYV_LAST)
				return;
			m_keyPress[key16] = false;
		}
		
		inline bool isKeyPressed(uint16_t key16) const
		{
			return ((key16 < KEYV_LAST) && m_keyPress[key16]);
		}
		
		static const char *GetKeyName(GensKey_t key)
		{
			GensKey_u gkey;
			gkey.keycode = key;
			if (gkey.type != 0x00 || gkey.key16 >= KEYV_LAST)
				return NULL;
			return ms_KeyNames[gkey.key16];
		}
		
	protected:
		// Key names.
		static const char *ms_KeyNames[KEYV_LAST];
		
		// Keypress array.
		bool m_keyPress[KEYV_LAST];
};

}

#endif /* __LIBGENS_IO_KEYBOARD_HPP__ */
