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
		Io6Button(const IoBase *other = NULL);
		
		/**
		 * reset(): Reset function.
		 * Called when the system is reset.
		 */
		void reset(void);
		
		void writeCtrl(uint8_t ctrl);
		
		void writeData(uint8_t data);
		uint8_t readData(void) const;
		
		// Scanline counter function.
		void doScanline(void);
		
		// Controller configuration. (STATIC functions)
		static IoType DevType(void);
		static int NumButtons(void);
		static int NextLogicalButton(int button);
		
		// Controller configuration. (virtual functions)
		IoType devType(void) const;
		int numButtons(void) const;
		int nextLogicalButton(int button) const;
		
		// Get button names.
		static ButtonName_t ButtonName(int button);
		ButtonName_t buttonName(int button) const;
	
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


// Controller configuration. (STATIC functions)
inline IoBase::IoType Io6Button::DevType(void)
	{ return IOT_6BTN; }
inline int Io6Button::NumButtons(void)
	{ return 12; }

// Controller configuration. (virtual functions)
inline IoBase::IoType Io6Button::devType(void) const
	{ return DevType(); }
inline int Io6Button::numButtons(void) const
	{ return NumButtons(); }
inline int Io6Button::nextLogicalButton(int button) const
	{ return NextLogicalButton(button); }

/**
 * buttonName(): Get the name for a given button index. (virtual function)
 * @param button Button index.
 * @return Button name, or BTNNAME_UNKNOWN if the button index is invalid.
 */
inline IoBase::ButtonName_t Io6Button::buttonName(int button) const
	{ return ButtonName(button); }

}

#endif /* __LIBGENS_IO_IO6BUTTON_HPP__ */
