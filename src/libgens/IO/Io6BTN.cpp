/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io6BTN.cpp: Sega Mega Drive 6-button controller.                        *
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

#include "Io6BTN.hpp"

namespace LibGens { namespace IO {

Io6BTN::Io6BTN()
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
void Io6BTN::resetDev(void)
{
	Device::resetDev();	// TODO: typedef super?
	this->scanlines = 0;
}

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t Io6BTN::type(void) const
{
	return IoManager::IOT_6BTN;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void Io6BTN::update(void)
{
	// Update the tristate input cache.
	const uint8_t oldTrisIn = this->mdData_tris;
	this->updateTristateInputCache();

	// TODO: "unlikely()"?
	if (pin58 != 2) {
		// +5V/GND pins are wrong.
		// No valid data will be returned.
		// Also, the IC is probably fried now.
		this->deviceData = 0xFF;
		return;
	}

	if (!(oldTrisIn & IOPIN_TH) && checkInputLine(IOPIN_TH)) {
		// IOPIN_TH rising edge.
		// Increment the counter.
		this->counter = ((this->counter + 2) & 0x06);

		// Reset the scanline counter.
		this->scanlines = 0;
	}

	// Use the TH counter to determine the controller state.
	// TODO: There should be a 2-NOP delay between TH change and reaction...
	const int idx = (this->counter | (checkInputLine(IOPIN_TH) ? 0 : 1));
	uint8_t data;
	switch (idx) {
		case 0:
		case 2:
		case 4:
			// TH=1: First/Second/Third
			// Format: D1CBRLDU
			// (Same as 3-button.)
			data = (this->buttons & 0x3F) | 0x40;
			break;

		case 1:
		case 3:
			// TH=0: First/Second
			// Format: D0SA00DU
			// (Same as 6-button.)
			data = (this->buttons & 0xC0) >> 2;
			data |= (this->buttons & 0x03);
			break;

		case 5:
			// TH=0: Third
			// Format: D0SA0000
			data = (this->buttons & 0xC0) >> 2;
			break;

		case 6:
			// TH=1: Fourth
			// Format: D1CBMXYZ
			data = (this->buttons & 0x30) | 0x40;
			data |= ((this->buttons & 0xF00) >> 8);
			break;

		case 7:
			// TH=0: Fourth
			// Format: D0SA1111
			data = (this->buttons & 0xC0) >> 2;
			data |= 0x0F;
			break;

		default:
			data = 0xFF;
			break;
	}

	this->deviceData = data;
}

/**
 * One scanline worth of time has passed.
 * Needed for some devices that reset after a period of time,
 * e.g. 6BTN controllers.
 */
void Io6BTN::update_onScanline(void)
{
	static const int SCANLINE_COUNT_MAX_6BTN = 25;
	this->scanlines++;
	if (this->scanlines > SCANLINE_COUNT_MAX_6BTN) {
		// Scanline counter has reached its maximum value.
		// Reset both counters.
		this->counter = 0;
		this->scanlines = 0;
	}
}

} }
