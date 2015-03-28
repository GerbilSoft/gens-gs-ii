/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.cpp: I/O manager. (Private Class)                             *
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

#define __LIBGENS_IN_IOMANAGER_CLASS__
#include "IoManager_p.hpp"

namespace LibGens {

/**
 * Device information.
 */
const IoManagerPrivate::IoDevInfo IoManagerPrivate::ioDevInfo[IoManager::IOT_MAX] =
{
	{'NONE', 0, true},	// IOT_NONE
	{'3BTN', 8, true},	// IOT_3BTN
	{'6BTN', 12, true},	// IOT_6BTN
	{'2BTN', 7, true},	// IOT_2BTN
	{'TEAM', 0, true},	// IOT_TEAMPLAYER
	{'4WPM', 0, true},	// IOT_4WP_MASTER
	{'4WPS', 0, true},	// IOT_4WP_SLAVE

	// Miscellaneous Master System peripherals.
	{'PADL', 2, false},	// IOT_PADDLE
	{'SPAD', 2, false},	// IOT_SPORTS_PAD

	// Miscellaneous Mega Drive peripherals.
	{'MOUS', 4, false},	// IOT_MEGA_MOUSE
	{'XE1A', 8, false},	// IOT_XE_1AP
	{'ACTV', 0, false},	// IOT_ACTIVATOR

	// Light guns.
	{'PHAS', 0, false},	// IOT_PHASER
	{'MENA', 0, false},	// IOT_MENACER
	{'JUST', 0, false},	// IOT_JUSTIFIER
};

IoManagerPrivate::IoManagerPrivate(IoManager *q)
	: q(q)
	, ea4wp_curPlayer(7)
{ }

/**
 * Reset all devices.
 */
void IoManagerPrivate::reset(void)
{
	for (int i = 0; i < ARRAY_SIZE(ioDevices); i++)
		ioDevices[i].reset();

	// Rebuild Team Player controller index tables for TP devices.
	for (int i = IoManager::VIRTPORT_1;
	     i <= IoManager::VIRTPORT_2; i++) {
		if (ioDevices[i].type == IoManager::IOT_TEAMPLAYER)
			rebuildCtrlIndexTable(i);
	}

	// EA 4-Way Play.
	ea4wp_curPlayer = 7;
}

/**
 * Update the scanline counter for all controllers.
 * This is used by the 6-button controller,
 * which resets its internal counter after
 * around 25 scanlines of no TH rising edges.
 */
void IoManagerPrivate::doScanline(void)
{
	for (int i = 0; i < ARRAY_SIZE(ioDevices); i++) {
		IoDevice *const dev = &ioDevices[i];
		if (dev->type != IoManager::IOT_6BTN)
			continue;

		dev->scanlines++;
		if (dev->scanlines > SCANLINE_COUNT_MAX_6BTN) {
			// Scanline counter has reached its maximum value.
			// Reset both counters.
			dev->counter = 0;
			dev->scanlines = 0;
		}
	}
}

}
