/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoColecoVision.cpp: ColecoVision controller.                            *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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
 * Reference:
 * - http://arcarc.xmission.com/Web%20Archives/Deathskull%20%28May-2006%29/games/tech/cvcont.html
 */

#include "IoColecoVision.hpp"

namespace LibGens { namespace IO {

IoColecoVision::IoColecoVision()
	: Device()
{
	// resetDev() can't be called from the base constructor.
	resetDev();
}

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t IoColecoVision::type(void) const
{
	return IoManager::IOT_COLECOVISION;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void IoColecoVision::update(void)
{
	// Update the tristate input cache.
	this->updateTristateInputCache();

	// FIXME: Device mode is set by adjusting pins 5 and 8.
	// These map to +5V and GND on Mega Drive ports.
	// That update will need to be done "out of band".
	// this->mode, bit 1 == pin 5
	// this->mode, bit 0 == pin 8

	uint8_t data = 0xFF;
	if (!(m_pin58 & 1)) {
		// Joystick/quadrature input.
		// TODO: Get quadrature input from the frontend.
		// Format: [ x QA QB TL R L D U]

		// Low 5 bits of this->buttons match exactly.
		data &= (0xE0 | (this->buttons & 0x1F));
	}
	if (!(m_pin58 & 2)) {
		// Keypad input.
		// Format: [ x  x  x TR 4 3 2 1]

		// Get the right fire button. (SAC Red)
		data &= (0xEF | ((this->buttons & (1 << IoManager::BTNI_CV_TR_RED)) >> 1));

		/**
		 * Keypad matrix.
		 * Pressing certain buttons causes certain pins
		 * to be grounded: (x == grounded)
		 *
		 * | BTN | Pin4 | Pin3 | Pin2 | Pin1 |
		 * |  1  |      |      |      |   x  |
		 * |  2  |      |      |   x  |      |
		 * |  3  |   x  |      |      |   x  |
		 * |  4  |   x  |   x  |   x  |      |
		 * |  5  |      |   x  |   x  |      |
		 * |  6  |   x  |      |      |      |
		 * |  7  |      |      |   x  |   x  |
		 * |  8  |      |   x  |   x  |   x  |
		 * |  9  |      |   x  |      |      |
		 * |  *  |      |   x  |      |   x  |
		 * |  0  |   x  |   x  |      |      |
		 * |  #  |   x  |      |   x  |      |
		 * | Pur |   x  |   x  |      |   x  |
		 * | Blu |   x  |      |   x  |   x  |
		 *
		 * Pressing multiple keys can result in the wrong
		 * key registering as pressed, and in some cases,
		 * invalid keys.
		 */
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_1))			data &= ~0x01;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_2))			data &= ~0x02;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_3))			data &= ~0x09;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_4))			data &= ~0x0E;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_5))			data &= ~0x06;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_6))			data &= ~0x08;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_7))			data &= ~0x03;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_8))			data &= ~0x07;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_9))			data &= ~0x04;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_ASTERISK))		data &= ~0x05;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_0))			data &= ~0x0C;
		if (this->buttons & (1 << IoManager::BTNI_CV_KEYPAD_OCTOTHORPE))	data &= ~0x0A;
		if (this->buttons & (1 << IoManager::BTNI_CV_PURPLE))			data &= ~0x0D;
		if (this->buttons & (1 << IoManager::BTNI_CV_BLUE))			data &= ~0x0B;		
	}

	this->deviceData = data;
}

} }
