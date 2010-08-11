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

/**
 * Sega Mega Mouse protocol documentation by Charles MacDonald:
 * http://gendev.spritesmind.net/forum/viewtopic.php?t=579
 */

#include "IoMegaMouse.hpp"

namespace LibGens
{

/**
 * writeCtrl(): Set the I/O tristate value.
 * @param ctrl I/O tristate value.
 */
void IoMegaMouse::writeCtrl(uint8_t ctrl)
{
	// TODO: Combine with writeData().
	bool oldTR = (!(m_ctrl & IOPIN_TR) || (m_lastData & IOPIN_TR));
	bool oldSelect = isSelect();
	
	m_ctrl = ctrl;
	updateSelectLine();
	if (isSelect())
	{
		// TH line is high. Reset the counter.
		m_counter = 0;
		return;
	}
	
	// Check if the TH line has gone from HI to LO.
	if (oldSelect && !isSelect())
	{
		// TH line went low.
		// Assume oldTR == true;
		oldTR = true;
	}
	
	// Check if the TR line has changed.
	bool newTR = (!(m_ctrl & IOPIN_TR) || (m_lastData & IOPIN_TR));
	if (oldTR ^ newTR)
	{
		// IOPIN_TR has changed.
		m_counter++;
	}
}


/**
 * writeData(): Write data to the controller.
 * @param data Data to the controller.
 */
void IoMegaMouse::writeData(uint8_t data)
{
	// TODO: Combine with writeCtrl().
	bool oldTR = (!(m_ctrl & IOPIN_TR) || (m_lastData & IOPIN_TR));
	bool oldSelect = isSelect();
	
	m_lastData = data;
	updateSelectLine();
	if (isSelect())
	{
		// TH line is high. Reset the counter.
		m_counter = 0;
		return;
	}
	
	// Check if the TH line has gone from HI to LO.
	if (oldSelect && !isSelect())
	{
		// TH line went low.
		// Assume oldTR == false;
		oldTR = false;
	}
	
	// Check if the TR line has changed.
	bool newTR = (!(m_ctrl & IOPIN_TR) || (m_lastData & IOPIN_TR));
	if (oldTR ^ newTR)
	{
		// IOPIN_TR has changed.
		m_counter++;
	}
}


/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t IoMegaMouse::readData(void)
{
	// TL bit (0x10) is Busy Flag (BF).
	
	// Use the TR counter to determine the controller state.
	uint8_t ret;
	switch (m_counter)
	{
		case 0:
			// ID #0: $0 (+BF)
			ret = 0x10;
			break;
		
		case 1:
			// ID #1: $B (+BF)
			ret = 0x1B;
			latchMouseMovement();
			break;
		
		case 2:
			// ID #2: $F (-BF)
			ret = 0x0F;
			break;
		
		case 3:
			// ID #3: $F (+BF)
			ret = 0x1F;
			break;
		
		case 4:
			// Axis sign and overflow. (-BF)
			// Format: [YOVER XOVER YSIGN XSIGN]
			// OVER == overflow occurred
			// SIGN == 0 for positive, 1 for negative
			ret = m_latchSignOver;
			break;
		
		case 5:
			// Mouse buttons. (+BF)
			// Format: [START MIDDLE RIGHT LEFT]
			ret = 0x10 | (m_buttons & 0x0F);
			break;
		
		case 6:
			// X axis MSN. (-BF)
			ret = ((m_latchRelX >> 4) & 0x0F);
			break;
		
		case 7:
			// X axis LSN. (+BF)
			ret = 0x10 | (m_latchRelX & 0x0F);
			break;
		
		case 8:
			// Y axis MSN. (-BF)
			ret = ((m_latchRelY >> 4) & 0x0F);
			break;
		
		case 9:
		default:
			// Y axis LSN. (+BF)
			// Also returned if the mouse is polled
			// more than 10 times.
			ret = 0x10 | (m_latchRelY & 0x0F);
			break;
	}
	
	return applyTristate(ret);
}


/**
 * keyPress(): Key press handler.
 * @param key Key.
 */
void IoMegaMouse::keyPress(int key)
{
	// TODO: Allow customizable keymaps.
	// TODO: Use LibGens keycodes.
	// NOTE: Mega Mouse buttons are Active High.
	if (key == ' ')
	{
		// Qt::Key_Space
		m_buttons |= BTN_MOUSE_START;
	}
}


/**
 * keyRelease(): Key release handler.
 * @param key Key.
 */
void IoMegaMouse::keyRelease(int key)
{
	// TODO: Allow customizable keymaps.
	// TODO: Use LibGens keycodes.
	// NOTE: Mega Mouse buttons are Active High.
	if (key == ' ')
	{
		// Qt::Key_Space
		m_buttons &= ~BTN_MOUSE_START;
	}
}


/**
 * mouseMove(): Mouse has moved.
 * @param relX Relative X movement.
 * @param relY Relative Y movement.
 */
void IoMegaMouse::mouseMove(int relX, int relY)
{
	// Update the mouse movement variables.
	// NOTE: Y axis is inverted for some reason.
	// TODO: This doesn't work too well...
	m_relX += relX;
	m_relY -= relY;
}


/**
 * mousePress(): Mouse button has been pressed.
 * @param button Mouse button.
 */
void IoMegaMouse::mousePress(int button)
{
	// TODO: Allow customizable keymaps.
	// TODO: Use LibGens keycodes.
	// NOTE: Mega Mouse buttons are Active High.
	if (button & 0x01)	// Qt::LeftButton
		m_buttons |= BTN_MOUSE_LEFT;
	if (button & 0x04)	// Qt::MiddleButton
		m_buttons |= BTN_MOUSE_MIDDLE;
	if (button & 0x02)	// Qt::RightButton
		m_buttons |= BTN_MOUSE_RIGHT;
}


/**
 * mousePress(): Mouse button has been released.
 * @param button Mouse button.
 */
void IoMegaMouse::mouseRelease(int button)
{
	// TODO: Allow customizable keymaps.
	// TODO: Use LibGens keycodes.
	// NOTE: Mega Mouse buttons are Active High.
	if (button & 0x01)	// Qt::LeftButton
		m_buttons &= ~BTN_MOUSE_LEFT;
	if (button & 0x04)	// Qt::MiddleButton
		m_buttons &= ~BTN_MOUSE_MIDDLE;
	if (button & 0x02)	// Qt::RightButton
		m_buttons &= ~BTN_MOUSE_RIGHT;
}

#include <stdio.h>
/**
 * latchMouseMovement(): Latch the mouse movement variables.
 */
void IoMegaMouse::latchMouseMovement(void)
{
	// TODO: Optimize this function!
	
	// Format: [YOVER XOVER YSIGN XSIGN]
	// OVER == overflow occurred
	// SIGN == 0 for positive, 1 for negative
	
	m_latchSignOver = 0;
	
	// X axis.
	if (m_relX > 255)
	{
		m_latchSignOver = 0x04;
		m_latchRelX = 255;
	}
	else if (m_relX >= 0)
	{
		m_latchSignOver = 0x00;
		m_latchRelX = (m_relX & 0xFF);
	}
	else if (m_relX < -255)
	{
		m_latchSignOver = 0x05;
		m_latchRelX = 255;
	}
	else //if (m_relX < 0)
	{
		m_latchSignOver = 0x01;
		m_latchRelX = (m_relX & 0xFF);
	}
	
	// Y axis.
	if (m_relY > 255)
	{
		m_latchSignOver |= 0x08;
		m_latchRelY = 255;
	}
	else if (m_relY >= 0)
	{
		m_latchRelY = (m_relY & 0xFF);
	}
	else if (m_relY < -255)
	{
		m_latchSignOver |= 0x0A;
		m_latchRelY = 255;
	}
	else //if (m_relY < 0)
	{
		m_latchSignOver |= 0x02;
		m_latchRelY = (m_relY & 0xFF);
	}
	
	// Clear the accumulators.
	m_relX = 0;
	m_relY = 0;
}


/** Controller Configuration. **/


/**
 * nextLogicalButton(): Get the next logical button.
 * @return Next logical button, or -1 if we're at the end.
 */
int IoMegaMouse::nextLogicalButton(int button) const
{
	if (button >= BTNI_MOUSE_LEFT && button < BTNI_MOUSE_START)
		return (button + 1);
	return -1;
}

/**
 * buttonName(): Get the name for a given button index.
 * @param button Button index.
 * @return Button name, or NULL if the button index is invalid.
 */
const char *IoMegaMouse::buttonName(int button) const
{
	static const char *btnNames[] =
	{
		"Left", "Right", "Middle", "Start"
	};
	
	if (button >= BTNI_MOUSE_LEFT && button <= BTNI_MOUSE_START)
		return btnNames[button];
	return NULL;
}

}
