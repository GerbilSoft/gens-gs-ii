/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io3Button.cpp: 3-button gamepad device.                                 *
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
	if (isSelect())
	{
		// TH=1.
		ret = (m_buttons & 0x3F) | 0x40;
	}
	else
	{
		// TH=0.
		ret = (m_buttons & 0xC0) >> 2;
		ret |= (m_buttons & 0x03);
	}
	
	return applyTristate(ret);
}


/**
 * keyPress(): Key press handler.
 * @param key Key.
 */
void Io3Button::keyPress(int key)
{
	// TODO: Allow customizable keymaps.
	// TODO: Use LibGens keycodes.
	switch (key)
	{
		case 0x01000013:
			// Qt::Key_Up
			m_buttons &= ~BTN_UP;
			break;
		case 0x01000015:
			// Qt::Key_Down
			m_buttons &= ~BTN_DOWN;
			break;
		case 0x01000012:
			// Qt::Key_Left
			m_buttons &= ~BTN_LEFT;
			break;
		case 0x01000014:
			// Qt::Key_Right
			m_buttons &= ~BTN_RIGHT;
			break;
		case 0x01000004:
			// Qt::Key_Return
			m_buttons &= ~BTN_START;
			break;
		case 'A':
			// Qt::Key_A
			m_buttons &= ~BTN_A;
			break;
		case 'S':
			// Qt::Key_S
			m_buttons &= ~BTN_B;
			break;
		case 'D':
			// Qt::Key_D
			m_buttons &= ~BTN_C;
			break;
		
		default:
			break;
	}
}


/**
 * keyRelease(): Key release handler.
 * @param key Key.
 */
void Io3Button::keyRelease(int key)
{
	// TODO: Allow customizable keymaps.
	// TODO: Use LibGens keycodes.
	switch (key)
	{
		case 0x01000013:
			// Qt::Key_Up
			m_buttons |= BTN_UP;
			break;
		case 0x01000015:
			// Qt::Key_Down
			m_buttons |= BTN_DOWN;
			break;
		case 0x01000012:
			// Qt::Key_Left
			m_buttons |= BTN_LEFT;
			break;
		case 0x01000014:
			// Qt::Key_Right
			m_buttons |= BTN_RIGHT;
			break;
		case 0x01000004:
			// Qt::Key_Return
			m_buttons |= BTN_START;
			break;
		case 'A':
			// Qt::Key_A
			m_buttons |= BTN_A;
			break;
		case 'S':
			// Qt::Key_S
			m_buttons |= BTN_B;
			break;
		case 'D':
			// Qt::Key_D
			m_buttons |= BTN_C;
			break;
		
		default:
			break;
	}
}


/** Controller Configuration. **/


/**
 * nextLogicalButton(): Get the next logical button.
 * @return Next logical button, or -1 if we're at the end.
 */
int Io3Button::nextLogicalButton(int button) const
{
	switch (button)
	{
		case BTNI_UP:		return BTNI_DOWN;
		case BTNI_DOWN:		return BTNI_LEFT;
		case BTNI_LEFT:		return BTNI_RIGHT;
		case BTNI_RIGHT:	return BTNI_START;
		case BTNI_START:	return BTNI_A;
		case BTNI_A:		return BTNI_B;
		case BTNI_B:		return BTNI_C;
		case BTNI_C:
		default:
			return -1;
	}
}


/**
 * buttonName(): Get the name for a given button index.
 * @param button Button index.
 * @return Button name, or NULL if the button index is invalid.
 */
const char *Io3Button::buttonName(int button) const
{
	static const char *btnNames[] =
	{
		"Up", "Down", "Left", "Right",
		"B", "C", "A", "Start"
	};
	
	if (button >= BTNI_UP && button <= BTNI_START)
		return btnNames[button];
	return NULL;
}

}
