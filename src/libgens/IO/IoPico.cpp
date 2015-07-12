/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoPico.cpp: Sega Pico controller.                                       *
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

#include "IoPico.hpp"

// TODO: Add support for:
// - PageUp/PageDown: Read the page control register.
// - Pen position.

namespace LibGens { namespace IO {

IoPico::IoPico()
	: Device()
{ }

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t IoPico::type(void) const
{
	return IoManager::IOT_PICO;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void IoPico::update(void)
{
	// NOTE: The Pico controller is input-only.
	// Update tristate settings to reflect this.
	this->ctrl = 0x00;	// input only

	// NOTE: The Pico controller is hard-wired into the system.
	// There's no DE-9 interface, so m_pin58 is ignored.

	/**
	 * Data format:
	 * - PudBRLDU
	 * B = red button
	 * d = Page Down
	 * u = Page Up
	 * P = pen button
	 *
	 * NOTE: Page control buttons are NOT visible to the
	 * system here. They're mapped as buttons for
	 * convenience purposes.
	 */
	this->deviceData = (0xFF & (this->buttons | 0x60));

	// Due to the design of the MD input subsystem,
	// the high bit is normally a latched bit.
	// Pico is input-only, so all 8 bits are input.
	// Update the latched data to match the device data.
	this->mdData = this->deviceData;
}

} }
