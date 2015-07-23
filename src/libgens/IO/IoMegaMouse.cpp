/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoMegaMouse.cpp: Sega Mega Drive mouse.                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include "IoMegaMouse.hpp"

namespace LibGens { namespace IO {

IoMegaMouse::IoMegaMouse()
	: Device()
{
	// resetDev() can't be called from the base constructor.
	resetDev();
}

/**
 * Reset Device data that only affects the device
 * and not the emulation-side registers.
 *
 * Should be overridden by subclasses that have
 * device-specific data.
 */
void IoMegaMouse::resetDev(void)
{
	Device::resetDev();	// TODO: typedef super?
	this->relX = 0;
	this->relY = 0;
	latch.signOver = 0;
	latch.relX = 0;
	latch.relY = 0;
}

/**
 * Device type.
 * Should be overridden by subclasses.
 */
IoManager::IoType_t IoMegaMouse::type(void) const
{
	return IoManager::IOT_MEGA_MOUSE;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void IoMegaMouse::update(void)
{
	// Update the tristate input cache.
	const uint8_t oldTrisIn = this->mdData_tris;
	this->updateTristateInputCache();

	// TODO: "unlikely()"?
	if (m_pin58 != 2) {
		// +5V/GND pins are wrong.
		// No valid data will be returned.
		// Also, the IC is probably fried now.
		this->deviceData = 0xFF;
		return;
	}

	/**
	 * Sega Mega Mouse protocol documentation by Charles MacDonald:
	 * http://gendev.spritesmind.net/forum/viewtopic.php?t=579
	 */

	// Check for RESET.
	if (checkInputLine(IOPIN_TH)) {
		// TH line is high. Reset the counter.
		this->counter = 0;
	} else {
		// Check if the device data needs to be updated.
		if (this->counter == 0) {
			// Wait for TH falling edge.
			if ((oldTrisIn & IOPIN_TH) && !checkInputLine(IOPIN_TH)) {
				// TH falling edge.
				this->counter++;
				latchData();
			}
		} else {
			// Check for TR transition.
			// NOTE: checkInputLine returns 0 or IOPIN_TR.
			if ((oldTrisIn & IOPIN_TR) != checkInputLine(IOPIN_TR)) {
				// IOPIN_TR has changed.
				this->counter++;
			}
		}
	}

	// Determine the new device data.
	uint8_t data;
	switch (this->counter) {
		case 0:
			// ID #0: $0 (+BF)
			data = 0x10;
			break;
		case 1:
			// ID #1: $B (+BF)
			data = 0x1B;
			break;
		case 2:
			// ID #2: $F (-BF)
			data = 0x0F;
			break;
		case 3:
			// ID #3: $F (+BF)
			data = 0x1F;
			break;
		case 4:
			// Axis sign and overflow. (-BF)
			// Format: [YOVER XOVER YSIGN XSIGN]
			// OVER == overflow occurred
			// SIGN == 0 for positive, 1 for negative
			data = latch.signOver;
			break;
		case 5:
			// Mouse buttons. (+BF)
			// Format: [START MIDDLE RIGHT LEFT]

			// NOTE: Mega Mouse buttons are Active High.
			// m_buttons is active low, so it needs to be inverted.
			data = 0x10 | (~this->buttons & 0x0F);
			break;
		case 6:
			// X axis MSN. (-BF)
			data = (latch.relX >> 4) & 0xF;
			break;
		case 7:
			// X axis LSN. (+BF)
			data = 0x10 | (latch.relX & 0xF);
			break;
		case 8:
			// Y axis MSN. (-BF)
			data = (latch.relY >> 4) & 0xF;
			break;
		case 9:
		default:
			// Y axis LSN. (+BF)
			// Also returned if the mouse is polled
			// more than 10 times.
			data = 0x10 | (latch.relY & 0xF);
			break;
	}

	// Save the device data.
	this->deviceData = data;
}

/**
 * Latch relX, relY, and signOver.
 */
void IoMegaMouse::latchData(void)
{
	// TODO: Optimize this function!

	// Format: [YOVER XOVER YSIGN XSIGN]
	// OVER == overflow occurred
	// SIGN == 0 for positive, 1 for negative

	latch.signOver = 0;

	// X axis.
	if (this->relX > 255) {
		latch.signOver = 0x4;
		latch.relX = 255;
	} else if (this->relX >= 0) {
		latch.signOver = 0x0;
		latch.relX = (this->relX & 0xFF);
	} else if (this->relX < -255) {
		latch.signOver = 0x5;
		latch.relX = 255;
	} else /*if (this->relX < 0)*/ {
		latch.signOver = 0x1;
		latch.relX = (this->relX & 0xFF);
	}

	// Y axis.
	if (this->relY > 255) {
		latch.signOver |= 0x8;
		latch.relY = 255;
	} else if (this->relY >= 0) {
		latch.relY = (this->relY & 0xFF);
	} else if (this->relY < -255) {
		latch.signOver |= 0xA;
		latch.relY = 255;
	} else /*if (this->relY < 0)*/ {
		latch.signOver |= 0x2;
		latch.relY = (this->relY & 0xFF);
	}

	// Clear the accumulators.
	this->relX = 0;
	this->relY = 0;
}

} }
