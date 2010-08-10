/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io3Button.hpp: 3-button gamepad device.                                 *
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

#ifndef __LIBGENS_IO_IO3BUTTON_HPP__
#define __LIBGENS_IO_IO3BUTTON_HPP__

#include "IoBase.hpp"

namespace LibGens
{

class Io3Button : public IoBase
{
	public:
		Io3Button()
		{
			m_buttons = ~0;
		}
		Io3Button(const IoBase *other)
			: IoBase(other)
		{
			m_buttons = ~0;
		}
		virtual ~Io3Button() { }
		
		uint8_t readData(void);
		
		// Keypress handling functions.
		void keyPress(int key);
		void keyRelease(int key);
	
	protected:
		/**
		 * m_buttons: Controller bitfield.
		 * Format: SACBRLDU
		 * NOTE: ACTIVE LOW! (1 == released; 0 == pressed)
		 */
		uint8_t m_buttons;
		
		// Button bitfield values.
		enum CtrlButtons
		{
			BTN_UP		= 0x01,
			BTN_DOWN	= 0x02,
			BTN_LEFT	= 0x04,
			BTN_RIGHT	= 0x08,
			BTN_B		= 0x10,
			BTN_C		= 0x20,
			BTN_A		= 0x40,
			BTN_START	= 0x80
		};
};

}

#endif /* __LIBGENS_IO_IO3BUTTON_HPP__ */
