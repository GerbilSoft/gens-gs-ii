/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io4WPS.cpp: EA 4-Way Play Slave device.                                 *
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

#include "Io4WPS.hpp"

#include "macros/common.h"

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

namespace LibGens { namespace IO {

Io4WPS::Io4WPS()
	: Device()
{
	m_type = IoManager::IOT_4WP_SLAVE;

	// No controllers are associated initially.
	memset(pads, 0, sizeof(pads));

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
void Io4WPS::resetDev(void)
{
	Device::resetDev();	// TODO: typedef super?
	// Default to device detection.
	player = 0x7;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void Io4WPS::update(void)
{
	// Update the tristate input cache.
	this->updateTristateInputCache();

	if (player >= 4) {
		// Multitap detection.
		this->deviceData = 0x70;
		return;
	}

	// Update the current virtual gamepad.
	Device *pad = pads[player];
	if (!pad) {
		// No device is assigned.
		this->deviceData = 0xFF;
		return;
	}

	// Set the device ctrl/data values.
	pad->ctrl = this->ctrl;
	pad->mdData = this->mdData;
	pad->setPin58(this->m_pin58);	// FIXME: May run update().
	pad->update();
	// TODO: Should be run on readDataMD() as well.
	this->deviceData = pad->deviceData;
}

/**
 * Set the current player.
 * Should only be called by Io4WPM.
 * @param player Current player.
 */
void Io4WPS::setCurrentPlayer(uint8_t player)
{
	assert(/*player >= 0 &&*/ player <= 7);
	this->player = (player & 7);
	update();
}

/**
 * Set a sub-device.
 * Used for multitaps.
 * @param virtPort Virtual port number. (0-3)
 * @param ioDevice I/O device.
 * @return 0 on success; non-zero on error.
 */
int Io4WPS::setSubDevice(int virtPort, Device *ioDevice)
{
	assert(virtPort >= 0 && virtPort < ARRAY_SIZE(pads));
	if (virtPort < 0 || virtPort >= ARRAY_SIZE(pads))
		return -1;

	// Set the pad.
	// TODO: Also add a "set all sub devices" function
	// so we don't have to rebuild the index table
	// multiple times?
	// TODO: Verify device type.
	// TODO: update()?
	pads[virtPort] = ioDevice;
	return 0;
}

} }
