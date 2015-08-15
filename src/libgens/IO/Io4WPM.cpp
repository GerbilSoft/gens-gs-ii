/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io4WPM.cpp: EA 4-Way Play Master device.                                *
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

#include "Io4WPM.hpp"

// C includes. (C++ namespace)
#include <cassert>

namespace LibGens { namespace IO {

Io4WPM::Io4WPM()
	: Device()
{
	m_type = IoManager::IOT_4WP_MASTER;
	m_hasDPad = false;

	// No slave device is associated initially.
	slave = nullptr;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void Io4WPM::update(void)
{
	// Update the tristate input cache.
	this->updateTristateInputCache();

	// TODO: "unlikely()"?
	if (m_pin58 != 2) {
		// +5V/GND pins are wrong.
		// No valid data will be returned.
		// Also, the IC is probably fried now.
		this->deviceData = 0xFF;
		return;
	}

	// Update the slave port number.
	assert(slave != nullptr);
	if (slave) {
		assert(slave->type() == IoManager::IOT_4WP_SLAVE);
		const uint8_t player = (this->mdData_tris >> 4) & 0x07;
		slave->setCurrentPlayer(player);
	}

	// Device data is always 0x7F.
	this->deviceData = 0x7F;
}

} }
