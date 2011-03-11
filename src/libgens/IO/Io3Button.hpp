/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io3Button.hpp: 3-button gamepad device.                                 *
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

#ifndef __LIBGENS_IO_IO3BUTTON_HPP__
#define __LIBGENS_IO_IO3BUTTON_HPP__

#include "IoBase.hpp"

namespace LibGens
{

class Io3Button : public IoBase
{
	public:
		Io3Button(const IoBase *other = NULL);
		
		uint8_t readData(void) const;
		
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
};


// Controller configuration. (STATIC functions)
inline IoBase::IoType Io3Button::DevType(void)
	{ return IOT_3BTN; }
inline int Io3Button::NumButtons(void)
	{ return 8; }

// Controller configuration. (virtual functions)
inline IoBase::IoType Io3Button::devType(void) const
	{ return DevType(); }
inline int Io3Button::numButtons(void) const
	{ return NumButtons(); }
inline int Io3Button::nextLogicalButton(int button) const
	{ return NextLogicalButton(button); }

/**
 * buttonName(): Get the name for a given button index. (virtual function)
 * @param button Button index.
 * @return Button name, or BTNNAME_UNKNOWN if the button index is invalid.
 */
inline IoBase::ButtonName_t Io3Button::buttonName(int button) const
	{ return ButtonName(button); }

}

#endif /* __LIBGENS_IO_IO3BUTTON_HPP__ */
