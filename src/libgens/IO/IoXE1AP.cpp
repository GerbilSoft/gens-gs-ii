/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoXE1AP.hpp: Dempa XE-1Ap analog controller.                            *
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

#include "IoXE1AP.hpp"

namespace LibGens { namespace IO {

IoXE1AP::IoXE1AP()
	: Device()
{
	m_type = IoManager::IOT_XE_1AP;

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
void IoXE1AP::resetDev(void)
{
	this->latency = 0;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void IoXE1AP::update(void)
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

	// Protocol reference:
	// https://code.google.com/p/genplus-gx/issues/detail?id=156#c17

	// XE-1AP device was written to.
	// Check for TH falling edge.
	if ((oldTrisIn & IOPIN_TH) && !checkInputLine(IOPIN_TH)) {
		// TH falling edge.
		// Next acquisition cycle.
		this->latency = 0;
		this->counter = 0;
	} else {
		// Make sure we've had enough reads for this sequence.
		if (this->latency > 2) {
			// Next acquisition sequence.
			// Each sequence is 8 cycles, so we want to
			// mask out the individual cycles in the
			// current sequence while keeping the
			// current sequence number.
			this->counter = (this->counter & ~0x07) + 8;
			if (this->counter > 32) {
				// Too many cycles.
				this->counter = 32;
			}

			// Do the first READ cycle.
			update_onRead();
		}
	}
}

/**
 * Device port was read.
 * Only applies to devices on physical ports.
 * Needed for some devices that have partially-unclocked protocols.
 */
void IoXE1AP::update_onRead(void)
{
	uint8_t data = 0x40;

	// Determine what data is being requested.
	// Each acquisition sequence has eight internal data cycles,
	// of which only a few are actually usable.
	// TODO: Add analog sources to IoManager.
	const uint8_t x = 128;
	const uint8_t y = 128;
	const uint8_t z = 128;

	switch ((this->counter >> 2) & 0x0F) {
		case 0:
			// E1, E2, Start, Select
			data |= (this->buttons & 0x0F);
			break;
		case 1:
			// A, B, C, D
			data |= ((this->buttons >> 4) & 0x0F);
			break;
		case 2:
			// X, MSB
			data |= ((x >> 4) & 0xF);
			break;
		case 3:
			// Y, MSB
			data |= ((y >> 4) & 0xF);
			break;
		case 4:
			// Unknown data...
			break;
		case 5:
			// Z, MSB
			data |= ((z >> 4) & 0xF);
			break;
		case 6:
			// X, LSB
			data |= (x & 0xF);
			break;
		case 7:
			// Y, LSB
			data |= (y & 0xF);
			break;
		case 8:
			// Unknown data...
			break;
		case 9:
			// Z, LSB
			data |= (z & 0xF);
			break;
		default:
			// Unknown data cycle.
			break;
	}

	// Determine the internal cycle number.
	uint8_t cycle = this->counter & 0x07;

	// TL indicates which part of the data acquisition is being returned.
	// 0 == 1st part, 1 == 2nd part
	data |= ((cycle & 4) << 2);

	// TR indicates if data is ready.
	// 1 == not ready, 0 == ready
	// First part of cycle needs to be "not ready".
	// Some games expect to see the "Not Ready" cycle,
	// e.g. "Fastest One".
	data |= (!(cycle & 3) << 5);

	// Save the output data.
	this->deviceData = data;

	// Go to the next cycle.
	cycle = (cycle + 1) & 0x07;
	this->counter = (this->counter & ~0x07) | cycle;
	this->latency++;
}

} }
