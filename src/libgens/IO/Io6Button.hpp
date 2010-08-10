/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io6Button.hpp: 6-button gamepad device.                                 *
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

#ifndef __LIBGENS_IO_IO6BUTTON_HPP__
#define __LIBGENS_IO_IO6BUTTON_HPP__

#include "IoBase.hpp"

namespace LibGens
{

class Io6Button : public IoBase
{
	public:
		Io6Button()
		{
			m_buttons = ~0;
			m_counter = 0;
		}
		virtual ~Io6Button() { }
		
		void writeData(uint8_t data);
		uint8_t readData(void);
		
		// Keypress handling functions.
		void keyPress(int key);
		void keyRelease(int key);
	
	protected:
		/**
		 * m_buttons: Controller bitfield.
		 * Format: ????MXYZ SACBRLDU
		 * NOTE: ACTIVE LOW! (1 == released; 0 == pressed)
		 */
		uint16_t m_buttons;
		
		/**
		 * m_counter: TH (SELECT) counter.
		 * This is used to determine which set of buttons
		 * to return to the system.
		 */
		int m_counter;
		
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
			BTN_START	= 0x80,
			BTN_Z		= 0x100,
			BTN_Y		= 0x200,
			BTN_X		= 0x400,
			BTN_MODE	= 0x800
		};
};

}

#endif /* __LIBGENS_IO_IO6BUTTON_HPP__ */
