/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DeviceFactory.cpp: I/O device factory.                                  *
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

#include "DeviceFactory.hpp"
#include "Device.hpp"

// C includes.
#include <cassert>

// I/O device classes.
#include "Io3BTN.hpp"
#include "Io6BTN.hpp"
#include "Io2BTN.hpp"
#include "IoMegaMouse.hpp"
#include "IoPico.hpp"

// Multitaps.
#include "IoTeamPlayer.hpp"
#include "Io4WPM.hpp"
#include "Io4WPS.hpp"
#include "IoMasterTap.hpp"

namespace LibGens { namespace IO {

/**
 * Create a Device for the given IoType.
 * @param ioType I/O type.
 * @param virtPort Virtual port number. (Used to enforce constraints.)
 * @return Device, or nullptr on error.
 */
Device *DeviceFactory::createDevice(IoManager::IoType_t ioType, int virtPort)
{
	assert(ioType >= IoManager::IOT_NONE && ioType < IoManager::IOT_MAX);
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	Device *dev = nullptr;
	switch (ioType) {
		case IoManager::IOT_NONE:
			if (virtPort <= IoManager::VIRTPORT_EXT) {
				// Physical ports must not be nullptr,
				// so create a dummy device that does nothing.
				dev = new Device();
			}
			break;
		case IoManager::IOT_3BTN:
			dev = new Io3BTN();
			break;
		case IoManager::IOT_6BTN:
			dev = new Io6BTN();
			break;
		case IoManager::IOT_2BTN:
			dev = new Io2BTN();
			break;
		case IoManager::IOT_MEGA_MOUSE:
			dev = new IoMegaMouse();
			break;
		// TODO: XE1AP, ColecoVision.
		case IoManager::IOT_PICO:
			dev = new IoPico();
			break;
		case IoManager::IOT_TEAMPLAYER:
			dev = new IoTeamPlayer();
			break;
		case IoManager::IOT_4WP_MASTER:
			assert(virtPort == IoManager::VIRTPORT_2);
			if (virtPort == IoManager::VIRTPORT_2) {
				// EA 4-Way Play Master device must be on port 2.
				dev = new Io4WPM();
			}
			break;
		case IoManager::IOT_4WP_SLAVE:
			assert(virtPort == IoManager::VIRTPORT_1);
			if (virtPort == IoManager::VIRTPORT_1) {
				// EA 4-Way Play Slave device must be on port 1.
				dev = new Io4WPS();
			}
			break;
		case IoManager::IOT_MASTERTAP:
			dev = new IoMasterTap();
			break;
		default:
			// Unknown device...
			break;
	}

	return dev;
}

} }
