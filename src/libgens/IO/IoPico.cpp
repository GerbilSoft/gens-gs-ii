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
#include "lg_osd.h"

// TODO: Add support for:
// - Pen position.

namespace LibGens { namespace IO {

IoPico::IoPico()
	: Device()
	, m_page_num(0)
{ }

/**
 * Reset Device data that only affects the device
 * and not the emulation-side registers.
 *
 * Should be overridden by subclasses that have
 * device-specific data.
 */
void IoPico::resetDev(void)
{
	Device::resetDev();	// TODO: typedef super?
	m_page_num = 0;		// Reset to title page.
}

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

	// Check for page control buttons.
	// TODO: If both buttons are pressed, do nothing?
	if ((this->buttons_prev & 0x20) &&
	    !(this->buttons & 0x20))
	{
		// Page Down was pressed.
		if (m_page_num < (PICO_MAX_PAGES - 1)) {
			m_page_num++;
			lg_osd(OSD_PICO_PAGEDOWN, m_page_num);
		}
	}
	if ((this->buttons_prev & 0x40) &&
	    !(this->buttons & 0x40))
	{
		// Page Up was pressed.
		if (m_page_num > 0) {
			m_page_num--;
			lg_osd(OSD_PICO_PAGEUP, m_page_num);
		}
	}
}

/** Pico-specific functions. **/

/**
 * Get the current page number.
 * @return Page number. (0 == title; 1-7 == regular page)
 */
uint8_t IoPico::picoCurPageNum(void) const
{
	return m_page_num & 7;
}

/**
 * Set the current page number.
 * @param pg Page number. (0 == title; 1-7 == regular page)
 */
void IoPico::setPicoCurPageNum(uint8_t pg)
{
	if (m_page_num == pg)
		return;

	if (/*pg >= 0 &&*/ pg <= PICO_MAX_PAGES) {
		m_page_num = pg;
		lg_osd(OSD_PICO_PAGESET, m_page_num);
	}
}

/**
 * Get the current page register value.
 * @return Page register value.
 */
uint8_t IoPico::picoCurPageReg(void) const
{
	static const uint8_t pg_reg[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
	return pg_reg[m_page_num & 7];
}

} }
