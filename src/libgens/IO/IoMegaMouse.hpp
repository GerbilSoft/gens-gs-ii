/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoMegaMouse.hpp: Sega Mega Mouse device.                                *
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

#ifndef __LIBGENS_IO_IOMEGAMOUSE_HPP__
#define __LIBGENS_IO_IOMEGAMOUSE_HPP__

#include "IoBase.hpp"

namespace LibGens
{

class IoMegaMouse : public IoBase
{
	public:
		IoMegaMouse()
		{
			m_buttons = 0;	// Mega Mouse uses Active High logic.
			m_counter = 0;
			m_relX = 0;
			m_relY = 0;
			m_latchSignOver = 0;
			m_latchRelX = 0;
			m_latchRelY = 0;
		}
		IoMegaMouse(const IoBase *other)
			: IoBase(other)
		{
			m_buttons = 0;	// Mega Mouse uses Active High logic.
			m_counter = 0;
			m_relX = 0;
			m_relY = 0;
			m_latchSignOver = 0;
			m_latchRelX = 0;
			m_latchRelY = 0;
		}
		virtual ~IoMegaMouse() { }
		
		void writeCtrl(uint8_t ctrl);
		
		void writeData(uint8_t data);
		uint8_t readData(void);
		
		// Keypress handling functions.
		void keyPress(int key);
		void keyRelease(int key);
		
		// Mouse handling functions.
		void mouseMove(int relX, int relY);
		void mousePress(int button);
		void mouseRelease(int button);
		
		// Controller configuration.
		virtual int numButtons(void) const { return 12; }
		virtual int nextLogicalButton(int button) const;
		virtual const char *buttonName(int button) const;
	
	protected:
		/**
		 * m_counter: TR (SELECT) counter.
		 * This counter is reset when TH is set.
		 */
		int m_counter;
		
		// Current mouse movement.
		int m_relX, m_relY;
		
		// Latched mouse movement.
		void latchMouseMovement(void);
		uint8_t m_latchSignOver;
		uint8_t m_latchRelX, m_latchRelY;
};

}

#endif /* __LIBGENS_IO_IOMEGAMOUSE_HPP__ */
