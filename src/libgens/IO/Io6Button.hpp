/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io6Button.hpp: 6-button gamepad device.                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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
		Io6Button();
		Io6Button(const IoBase *other);
		virtual ~Io6Button() { }
		
		/**
		 * reset(): Reset function.
		 * Called when the system is reset.
		 */
		void reset(void);
		
		void writeCtrl(uint8_t ctrl);
		
		void writeData(uint8_t data);
		uint8_t readData(void);
		
		// Scanline counter function.
		void doScanline(void);
		
		// Controller configuration.
		IoType devType(void) const
			{ return IOT_6BTN; }
		int numButtons(void) const
			{ return 12; }
		int nextLogicalButton(int button) const;
		const char *buttonName(int button) const;
	
	protected:
		/**
		 * m_counter: TH (SELECT) counter.
		 * This is used to determine which set of buttons
		 * to return to the system.
		 */
		int m_counter;
		
		/**
		 * m_scanlines: Scanline counter.
		 * This is used to determine when to reset m_counter.
		 * (e.g. after 25 scanlines without a TH rising edge)
		 */
		int m_scanlines;
		
		static const int SCANLINE_COUNT_MAX = 25;
};

}

#endif /* __LIBGENS_IO_IO6BUTTON_HPP__ */
