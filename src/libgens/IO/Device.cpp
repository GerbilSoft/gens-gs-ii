/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Device.cpp: Base I/O device.                                            *
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

#include "Device.hpp"

namespace LibGens { namespace IO {

Device::Device()
{
	// TODO: Add pointer-based 'copy' constructor
	// to copy MD-side data.
	reset();
}

Device::~Device()
{ }

/**
 * Completely reset the Device.
 * This should be done on emulation startup or Hard Reset only.
 */
void Device::reset(void)
{
	ctrl = 0;
	mdData = 0xFF;
	serCtrl = 0;
	serLastTx = 0xFF;
	resetDev();
}

/**
 * Reset Device data that only affects the device
 * and not the emulation-side registers.
 *
 * Should be overridden by subclasses that have
 * device-specific data.
 */
void Device::resetDev(void) {
	counter = 0;
	buttons = ~0;
	deviceData = 0xFF;
	updateTristateInputCache();
}

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t Device::type(void) const
{
	return IoManager::IOT_NONE;
}

/**
 * Update the I/O device.
 * Saves the new buttons, then runs the internal device update.
 * @param buttons New button state.
 */
void Device::update(uint32_t buttons)
{
	// Save the buttons and update the device.
	this->buttons = buttons;
	update();
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void Device::update(void)
{
	// Update the tristate input cache.
	this->updateTristateInputCache();
}

/**
 * One scanline worth of time has passed.
 * Needed for some devices that reset after a period of time,
 * e.g. 6BTN controllers.
 */
void Device::update_onScanline(void)
{
	// We don't care about the onScanline event
	// in the base class, but the function needs
	// to exist in the vtable.
}

/**
 * Device port was read.
 * Only applies to devices on physical ports.
 * Needed for some devices that have partially-unclocked protocols.
 */
void Device::update_onRead(void)
{
	// We don't care about the onRead event
	// in the base class, but the function needs
	// to exist in the vtable.
}

} }
