/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io2Button.cpp: 2-button gamepad device.                                 *
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

#include "Io2Button.hpp"
#include "KeyManager.hpp"

namespace LibGens
{

Io2Button::Io2Button()
{
	// Resize the keymap.
	m_keyMap.resize(numButtons());
	
	// Create the default keymap.
	// TODO: Initialize elsewhere.
	m_keyMap[BTNI_UP]	= KEYV_UP;
	m_keyMap[BTNI_DOWN]	= KEYV_DOWN;
	m_keyMap[BTNI_LEFT]	= KEYV_LEFT;
	m_keyMap[BTNI_RIGHT]	= KEYV_RIGHT;
	m_keyMap[BTNI_1]	= KEYV_s;
	m_keyMap[BTNI_2]	= KEYV_d;
}

Io2Button::Io2Button(const IoBase *other)
	: IoBase(other)
{
	// Resize the keymap.
	m_keyMap.resize(numButtons());
	
	// Create the default keymap.
	// TODO: Initialize elsewhere.
	// TODO: Copy from other controller?
	m_keyMap[BTNI_UP]	= KEYV_UP;
	m_keyMap[BTNI_DOWN]	= KEYV_DOWN;
	m_keyMap[BTNI_LEFT]	= KEYV_LEFT;
	m_keyMap[BTNI_RIGHT]	= KEYV_RIGHT;
	m_keyMap[BTNI_1]	= KEYV_s;
	m_keyMap[BTNI_2]	= KEYV_d;
}


/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t Io2Button::readData(void)
{
	/**
	 * Data format: (x == tristate value)
	 * - xxCBRLDU
	 * B == button 1
	 * C == button 2
	 */
	
	uint8_t ret = (0xC0 | (m_buttons & 0x3F));
	return applyTristate(ret);
}


/** Controller Configuration. **/


/**
 * nextLogicalButton(): Get the next logical button.
 * @return Next logical button, or -1 if we're at the end.
 */
int Io2Button::nextLogicalButton(int button) const
{
	if (button >= BTNI_UP && button < BTNI_2)
		return (button + 1);
	return -1;
}


/**
 * buttonName(): Get the name for a given button index.
 * @param button Button index.
 * @return Button name, or NULL if the button index is invalid.
 */
const char *Io2Button::buttonName(int button) const
{
	static const char *btnNames[] =
	{
		"Up", "Down", "Left", "Right",
		"1", "2"
	};
	
	if (button >= BTNI_UP && button <= BTNI_2)
		return btnNames[button];
	return NULL;
}

}
