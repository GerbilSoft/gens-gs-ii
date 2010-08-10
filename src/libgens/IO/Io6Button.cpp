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

#include "Io6Button.hpp"

namespace LibGens
{

/**
 * writeData(): Write data to the controller.
 * @param data Data to the controller.
 */
void Io6Button::writeData(uint8_t data)
{
	// TODO: Check tristate values to determine how to interpret data!
	
	if ((data ^ m_lastData) & IOPIN_TH)
	{
		// IOPIN_TH has changed.
		m_counter++;
		m_counter &= 0x07;
	}
	
	// Save the last data value.
	m_lastData = data;
	
	// TODO: Reset the counter after ~8.192 ms of no TH changes.
}


/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t Io6Button::readData(void)
{
	// Use the TH counter to determine the controller state.
	uint8_t ret = (m_lastData & 0x80);
	switch (m_counter)
	{
		case 0:
		case 2:
		case 4:
			// Format: D1CBRLDU
			// (Same as 3-button.)
			ret |= (m_buttons & 0x3F) | 0x40;
			return ret;
		
		case 1:
		case 3:
			// Format: D0SA00DU
			// (Same as 6-button.)
			ret |= (m_buttons & 0xC0) >> 2;
			ret |= (m_buttons & 0x03);
			return ret;
		
		case 5:
			// Format: D0SA0000
			ret |= (m_buttons & 0xC0) >> 2;
			return ret;
		
		case 6:
			// Format: D1CBMXYZ
			ret |= (m_buttons & 0x30) | 0x40;
			ret |= ((m_buttons & 0xF00) >> 8);
			return ret;
		
		case 7:
			// Format: D0SA1111
			ret |= (m_buttons & 0xC0) >> 2;
			ret |= (m_buttons & 0x03);
			ret |= 0x0F;
			return ret;
		
		default:
			return 0xFF;
	}
}


/**
 * keyPress(): Key press handler.
 * @param key Key.
 */
void Io6Button::keyPress(int key)
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
		case 0x01000020:
			// Qt::Key_Shift
			// TODO: Distinguish left/right shift.
			m_buttons &= ~BTN_MODE;
			break;
		case 'Q':
			// Qt::Key_Q
			m_buttons &= ~BTN_X;
			break;
		case 'W':
			// Qt::Key_W
			m_buttons &= ~BTN_Y;
			break;
		case 'E':
			// Qt::Key_E
			m_buttons &= ~BTN_Z;
			break;
		
		default:
			break;
	}
}


/**
 * keyRelease(): Key release handler.
 * @param key Key.
 */
void Io6Button::keyRelease(int key)
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
		case 0x01000020:
			// Qt::Key_Shift
			// TODO: Distinguish left/right shift.
			m_buttons |= BTN_MODE;
			break;
		case 'Q':
			// Qt::Key_Q
			m_buttons |= BTN_X;
			break;
		case 'W':
			// Qt::Key_W
			m_buttons |= BTN_Y;
			break;
		case 'E':
			// Qt::Key_E
			m_buttons |= BTN_Z;
			break;
		
		default:
			break;
	}
}

}
