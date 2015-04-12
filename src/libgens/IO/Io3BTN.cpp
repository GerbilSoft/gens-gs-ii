/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io3BTN.cpp: Sega Mega Drive 3-button controller.                        *
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

#include "Io3BTN.hpp"

namespace LibGens { namespace IO {

Io3BTN::Io3BTN()
	: Device()
{ }

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t Io3BTN::type(void) const
{
	return IoManager::IOT_3BTN;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void Io3BTN::update(void)
{
	// Update the tristate input cache.
	this->updateTristateInputCache();

	// TODO: "unlikely()"?
	if (pin58 != 2) {
		// +5V/GND pins are wrong.
		// No valid data will be returned.
		// Also, the IC is probably fried now.
		this->deviceData = 0xFF;
		return;
	}

	/**
	 * Data formats: (D == last written MSB)
	 * TH=1: D1CBRLDU
	 * TH=0: D0SA00DU
	 */
	uint8_t data;
	if (checkInputLine(IOPIN_TH)) {
		// TH=1.
		data = (buttons & 0x3F) | 0x40;
	} else {
		// TH=0.
		data = (buttons & 0xC0) >> 2;
		data |= (buttons & 0x03);
	}

	this->deviceData = data;
}

} }
