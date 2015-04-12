/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoMasterTap.cpp: Sega Master System custom multitap adapter.            *
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

/**
 * Master Tap adapter for "BOoM"
 * References:
 * - http://www.smspower.org/Homebrew/BOoM-SMS
 * - http://www.smspower.org/uploads/Homebrew/BOoM-SMS-sms4p_2.png
 */

#include "IoMasterTap.hpp"

#include "macros/common.h"

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

namespace LibGens { namespace IO {

IoMasterTap::IoMasterTap()
	: Device()
{
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
void IoMasterTap::resetDev(void)
{
	Device::resetDev();	// TODO: typedef super?
	this->scanlines = 0;
}

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t IoMasterTap::type(void) const
{
	return IoManager::IOT_MASTERTAP;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void IoMasterTap::update(void)
{
	// Update the tristate input cache.
	const uint8_t oldTrisIn = this->mdData_tris;
	this->updateTristateInputCache();

	// TODO: "unlikely()"?
	if (pin58 != 2) {
		// +5V/GND pins are wrong.
		// No valid data will be returned.
		// Also, the ICs are probably fried now.
		this->deviceData = 0xFF;
		return;
	}

	// Master Tap is a homebrew multitap for Sega Master System.
	// It *only* works with 2BTN (SMS) controllers; using controllers
	// with active logic devices can result in hardware damage, since
	// the multitap works by multiplexing GND.
	// References:
	// - http://www.smspower.org/Homebrew/BOoM-SMS
	// - http://www.smspower.org/uploads/Homebrew/BOoM-SMS-sms4p_2.png

	// Check for a TH falling transition.
	if ((oldTrisIn & IOPIN_TH) && !checkInputLine(IOPIN_TH)) {
		// TH falling transition. 
		this->counter = (this->counter + 1) & 0x03;
		this->scanlines = 0; // TODO: Keep at 0 while TH is low.
	}

	// Update the current virtual gamepad.
	Device *pad = pads[this->counter];
	if (!pad) {
		// No device is assigned.
		this->deviceData = 0xFF;
		return;
	}

	// Set the device ctrl/data values.
	pad->ctrl = this->ctrl;
	pad->mdData = this->mdData;
	pad->update();
	// TODO: Should be run on readDataMD() as well.
	this->deviceData = pad->deviceData;
}

/**
 * One scanline worth of time has passed.
 * Needed for some devices that reset after a period of time,
 * e.g. 6BTN controllers.
 */
void IoMasterTap::update_onScanline(void)
{
	// TODO: Check Master Tap schematic to determine the actual value.
	static const int SCANLINE_COUNT_MAX_MTAP = 25;
	if (!checkInputLine(IOPIN_TH)) {
		// TH is low. Reset circuit is disabled.
		// TODO: Also do this for 6BTN?
		this->scanlines = 0;
	} else {
		// TH is high. Reset circuit is charging.
		this->scanlines++;
		if (this->scanlines > SCANLINE_COUNT_MAX_MTAP) {
			// Scanline counter has reached its maximum value.
			// Reset both counters.
			this->counter = 0;
			this->scanlines = 0;
		}
	}
}

/**
 * Set a sub-device.
 * Used for multitaps.
 * @param virtPort Virtual port number. (0-3)
 * @param ioDevice I/O device.
 * @return 0 on success; non-zero on error.
 */
int IoMasterTap::setSubDevice(int virtPort, Device *ioDevice)
{
	assert(virtPort >= 0 && virtPort < ARRAY_SIZE(pads));
	if (virtPort < 0 || virtPort >= ARRAY_SIZE(pads))
		return -1;

	// Set the pad and update the active controller.
	// TODO: Also add a "set all sub devices" function
	// so we don't have to rebuild the index table
	// multiple times?
	// TODO: Verify device type.
	pads[virtPort] = ioDevice;
	update();
	return 0;
}

} }
