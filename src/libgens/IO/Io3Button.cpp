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

#include "Io3Button.hpp"

namespace LibGens
{

/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t Io3Button::readData(void)
{
	/**
	 * Data formats: (D == last written MSB)
	 * TH=1: D1CBRLDU
	 * TH=0: D0SA00DU
	 */
	
	uint8_t ret;
	if (!(m_ctrl & IOPIN_TH) || (m_lastData & IOPIN_TH))
	{
		// TH=1.
		ret = (m_buttons & 0x0F);
		ret |= ((m_buttons & 0xC0) >> 2);
		ret |= (m_lastData & 0x80) | 0x40;
		return ret;
	}
	else
	{
		// TH=0.
		ret = (m_buttons & 0x33);
		ret |= (m_lastData & 0x80);
		return ret;
	}
}


/**
 * keyPress(): Key press handler.
 * @param key Key.
 */
void Io3Button::keyPress(int key)
{
	// TODO
}


/**
 * keyRelease(): Key release handler.
 * @param key Key.
 */
void Io3Button::keyRelease(int key)
{
	// TODO
}

}
