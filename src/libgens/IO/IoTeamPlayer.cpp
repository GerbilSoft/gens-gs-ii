/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoTeamPlayer.cpp: Sega Mega Drive Team Player adapter.                  *
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

#include "IoTeamPlayer.hpp"
#include "IoMegaMouse.hpp"

#include "macros/common.h"

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

namespace LibGens { namespace IO {

IoTeamPlayer::IoTeamPlayer()
	: Device()
{
	m_type = IoManager::IOT_TEAMPLAYER;
	m_hasDPad = false;

	// No controllers are associated initially.
	memset(pads, 0, sizeof(pads));
	memset(padTypes, 0, sizeof(padTypes));
	memset(ctrlIndexTbl, 0, sizeof(ctrlIndexTbl));
}

/**
 * Reset Device data that only affects the device
 * and not the emulation-side registers.
 *
 * Should be overridden by subclasses that have
 * device-specific data.
 */
void IoTeamPlayer::resetDev(void)
{
	Device::resetDev();	// TODO: typedef super?
	rebuildCtrlIndexTable();
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void IoTeamPlayer::update(void)
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

	// Check if either TH or TR has changed.
	// NOTE: checkInputLine(line) returns either 0 or line.
	// TODO: Optimize this, maybe?
	if ((oldTrisIn & IOPIN_TH) != checkInputLine(IOPIN_TH) ||
	    (oldTrisIn & IOPIN_TR) != checkInputLine(IOPIN_TR))
	{
		// Check if TH is high.
		if (checkInputLine(IOPIN_TH)) {
			// TH high. Reset the counter.
			this->counter = TP_DT_INIT;
		} else {
			// Increment the counter.
			this->counter++;
		}
	}

	if (this->counter >= TP_DT_MAX) {
		// Counter has overflowed.
		this->counter = TP_DT_MAX;
		this->deviceData = 0xFF;
		return;
	}

	// Check the controller data index table.
	uint8_t data = 0;
	switch (this->counter) {
		case TP_DT_INIT:
			// Initial state.
			data = 0x73;
			break;

		case TP_DT_START:
			// Start request.
			data = 0x3F;
			break;

		case TP_DT_ACK1:
		case TP_DT_ACK2:
			// Acknowledgement request.
			// TH=0, TR=0/1 -> RLDU = 0000
			data = 0x00;
			break;
		
		case TP_DT_PADTYPE_A:
		case TP_DT_PADTYPE_B:
		case TP_DT_PADTYPE_C:
		case TP_DT_PADTYPE_D:
			// Controller type.
			data = padTypes[this->counter - TP_DT_PADTYPE_A];
			break;

		default:
			// Check the controller data index table.
			// TODO: What value should be returned for errors?
			int adj_counter = (this->counter - TP_DT_PADA_RLDU);
			if (adj_counter >= ARRAY_SIZE(ctrlIndexTbl)) {
				// Counter is out of bounds.
				data = 0x0F;
				break;
			}

			// Look up the data type from the controller index table.
			TP_DataType dataType = (TP_DataType)ctrlIndexTbl[adj_counter];
			if (dataType < TP_DT_PADA_RLDU || dataType >= TP_DT_MAX) {
				// Invalid counter state.
				data = 0x0F;
				break;
			}

			// Readjust the counter for the correct data type.
			adj_counter = dataType - TP_DT_PADA_RLDU;

			// Controller data.
			// TODO: Move dtPerPortMax to a class constant?
			static const int dtPerPortMax = (TP_DT_PADA_MOUSE_Y_LSN - TP_DT_PADA_RLDU) + 1;
			const int state = (adj_counter % dtPerPortMax);
			const int tpPort = (adj_counter / dtPerPortMax);
			const Device *tpDev = pads[tpPort];
			if (!tpDev) {
				// No device is assigned.
				data = 0x0F;
				break;
			}

			// NOTE: Added masks to switch/case for jump table optimization.
			// TODO: Add a Team Player read data function to Device?
			switch (padTypes[tpPort] & 0xF) {
				case TP_PT_NONE:
				default:
					data = 0xF;
					break;

				case TP_PT_3BTN:
				case TP_PT_6BTN: {
					const int shift = state * 4;
					data = (tpDev->getButtons() >> shift) & 0xF;
					break;
				}

				case TP_PT_MOUSE: {
					// Mouse.
					// NOTE: We're not setting "BF" here.
					const IoMegaMouse *mouse = (const IoMegaMouse*)tpDev;
					switch (state & 7) {
						case 0:	// MOUSE_SIGNOVER
							data = mouse->latch.signOver;
							break;
						case 1:	// MOUSE_BUTTONS
							// NOTE: Active high!
							data = ~mouse->getButtons() & 0xF;
							break;
						case 2:	// MOUSE_X_MSN
							data = mouse->latch.relX >> 4;
							break;
						case 3: // MOUSE_X_LSN
							data = mouse->latch.relX & 0xF;
							break;
						case 4:	// MOUSE_Y_MSN
							data = mouse->latch.relY >> 4;
							break;
						case 5: // MOUSE_Y_LSN
							data = mouse->latch.relY & 0xF;
							break;
						default:
							data = 0xF;
							break;
					}
					break;
				}
			}
			break;
	}

	// TL should match TR.
	// (from Genesis Plus GX)
	// NOTE: TR is always an MD output line.
	if (checkInputLine(IOPIN_TR))
		data |= IOPIN_TL;
	else
		data &= ~IOPIN_TL;

	this->deviceData = data;
}

/**
 * Rebuild the controller index table.
 */
void IoTeamPlayer::rebuildCtrlIndexTable(void)
{
	// Check controller types.
	int i = 0;	// ctrlIndexTbl index
	for (int pad = 0; pad < 4; pad++) {
		// TODO: Move dtPerPortMax to a class constant?
		static const int dtPerPortMax = (TP_DT_PADA_MOUSE_Y_LSN - TP_DT_PADA_RLDU) + 1;
		const int dtBase = (TP_DT_PADA_RLDU + (pad * dtPerPortMax));

		if (!pads[pad]) {
			// No device is assigned.
			padTypes[pad] = TP_PT_NONE;
			continue;
		}

		switch (pads[pad]->type()) {
			case IoManager::IOT_NONE:
			default:
				padTypes[pad] = TP_PT_NONE;
				break;

			case IoManager::IOT_3BTN:
				padTypes[pad] = TP_PT_3BTN;
				ctrlIndexTbl[i++] = dtBase + 0;
				ctrlIndexTbl[i++] = dtBase + 1;
				break;

			case IoManager::IOT_6BTN:
				padTypes[pad] = TP_PT_6BTN;
				ctrlIndexTbl[i++] = dtBase + 0;
				ctrlIndexTbl[i++] = dtBase + 1;
				ctrlIndexTbl[i++] = dtBase + 2;
				break;

			case IoManager::IOT_MEGA_MOUSE:
				padTypes[pad] = TP_PT_MOUSE;
				ctrlIndexTbl[i++] = dtBase + 0;
				ctrlIndexTbl[i++] = dtBase + 1;
				ctrlIndexTbl[i++] = dtBase + 2;
				ctrlIndexTbl[i++] = dtBase + 3;
				ctrlIndexTbl[i++] = dtBase + 4;
				ctrlIndexTbl[i++] = dtBase + 5;
				break;
		}
	}

	// Set the rest of the controller data indexes to DT_MAX.
	for (int x = i; x < ARRAY_SIZE(ctrlIndexTbl); x++) {
		ctrlIndexTbl[x] = TP_DT_MAX;
	}
}

/**
 * Set a sub-device.
 * Used for multitaps.
 * @param virtPort Virtual port number. (0-3)
 * @param ioDevice I/O device.
 * @return 0 on success; non-zero on error.
 */
int IoTeamPlayer::setSubDevice(int virtPort, Device *ioDevice)
{
	assert(virtPort >= 0 && virtPort < ARRAY_SIZE(pads));
	if (virtPort < 0 || virtPort >= ARRAY_SIZE(pads))
		return -1;

	// Set the pad and rebuild the index table.
	// TODO: Also add a "set all sub devices" function
	// so we don't have to rebuild the index table
	// multiple times?
	// TODO: Verify device type.
	// TODO: update()?
	pads[virtPort] = ioDevice;
	rebuildCtrlIndexTable();
	return 0;
}

} }
