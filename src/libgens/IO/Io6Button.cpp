/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io6Button.cpp: 6-button gamepad device.                                 *
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
#include "KeyManager.hpp"

namespace LibGens
{

/**
 * writeCtrl(): Set the I/O tristate value.
 * TODO: Combine with writeData().
 * @param ctrl I/O tristate value.
 */
void Io6Button::writeCtrl(uint8_t ctrl)
{
	// Check if the SELECT line has changed.
	bool oldSelect = isSelect();
	m_ctrl = ctrl;
	updateSelectLine();
	
	if (!oldSelect && isSelect())
	{
		// IOPIN_TH rising edge.
		// Increment the counter.
		m_counter = ((m_counter + 2) & 0x06);
		
		// TODO: Reset the no-TH counter.
	}
	
	// TODO: Reset the counter after ~8.192 ms of no TH rising edges.
}


/**
 * writeData(): Write data to the controller.
 * * TODO: Combine with writeCtrl().
 * @param data Data to the controller.
 */
void Io6Button::writeData(uint8_t data)
{
	// Check if the SELECT line has changed.
	bool oldSelect = isSelect();
	m_lastData = data;
	updateSelectLine();
	
	if (!oldSelect && isSelect())
	{
		// IOPIN_TH rising edge.
		// Increment the counter.
		m_counter = ((m_counter + 2) & 0x06);
		
		// TODO: Reset the no-TH counter.
	}
	
	// TODO: Reset the counter after ~8.192 ms of no TH rising edges.
}


/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t Io6Button::readData(void)
{
	uint8_t ret;
	
	// Use the TH counter to determine the controller state.
	switch (m_counter | !isSelect())
	{
		case 0:
		case 2:
		case 4:
			// TH=1: First/Second/Third
			// Format: D1CBRLDU
			// (Same as 3-button.)
			ret = (m_buttons & 0x3F) | 0x40;
			break;
		
		case 1:
		case 3:
			// TH=0: First/Second
			// Format: D0SA00DU
			// (Same as 6-button.)
			ret = (m_buttons & 0xC0) >> 2;
			ret |= (m_buttons & 0x03);
			break;
		
		case 5:
			// TH=0: Third
			// Format: D0SA0000
			ret = (m_buttons & 0xC0) >> 2;
			break;
		
		case 6:
			// TH=1: Fourth
			// Format: D1CBMXYZ
			ret = (m_buttons & 0x30) | 0x40;
			ret |= ((m_buttons & 0xF00) >> 8);
			break;
		
		case 7:
			// TH=0: Fourth
			// Format: D0SA1111
			ret = (m_buttons & 0xC0) >> 2;
			ret |= (m_buttons & 0x03);
			ret |= 0x0F;
			break;
		
		default:
			ret = 0xFF;
			break;
	}
	
	return applyTristate(ret);
}


/**
 * update(): I/O device update function.
 */
void Io6Button::update(void)
{
	// TODO: Allow customizable keymaps.
	m_buttons = 0;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_RSHIFT);	// Mode
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_q);		// X
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_w);		// Y
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_e);		// Z
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_RETURN);	// Start
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_a);		// A
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_d);		// C
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_s);		// B
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_RIGHT);	// Right
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_LEFT);	// Left
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_DOWN);	// Down
	m_buttons <<= 1;
	m_buttons |= KeyManager::IsKeyPressed(KEYV_UP);		// Up
	m_buttons = ~m_buttons;
}


/** Controller Configuration. **/


/**
 * nextLogicalButton(): Get the next logical button.
 * @return Next logical button, or -1 if we're at the end.
 */
int Io6Button::nextLogicalButton(int button) const
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
		case BTNI_C:		return BTNI_MODE;
		case BTNI_MODE:		return BTNI_X;
		case BTNI_X:		return BTNI_Y;
		case BTNI_Y:		return BTNI_Z;
		case BTNI_Z:
		default:
			return -1;
	}
}


/**
 * buttonName(): Get the name for a given button index.
 * @param button Button index.
 * @return Button name, or NULL if the button index is invalid.
 */
const char *Io6Button::buttonName(int button) const
{
	static const char *btnNames[] =
	{
		"Up", "Down", "Left", "Right",
		"B", "C", "A", "Start",
		"Z", "Y", "X", "Mode"
	};
	
	if (button >= BTNI_UP && button <= BTNI_MODE)
		return btnNames[button];
	return NULL;
}

}
