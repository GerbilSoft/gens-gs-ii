/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.cpp: I/O manager. (Private Class) (Update Functions)          *
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

// C includes. (C++ namespace)
#include <cassert>

namespace LibGens {

/**
 * Update an I/O device.
 * @param virtPort Virtual port.
 * @param buttons New button state.
 */
void IoManagerPrivate::update(int virtPort, uint32_t buttons)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	// Set the new button state.
	ioDevices[virtPort].buttons = buttons;

	// Update the port state.
	updateDevice(virtPort);
}

/**
 * Update an I/O device's state based on ctrl/data lines.
 * @param virtPort Virtual port number.
 */
void IoManagerPrivate::updateDevice(int virtPort)
{
	IoDevice *const dev = &ioDevices[virtPort];
	const uint8_t oldTristateInput = dev->mdData_tris;
	dev->updateTristateInputCache();

	// Handle devices that require updating when certain lines change.
	switch (dev->type) {
		case IoManager::IOT_3BTN: updateDevice_3BTN(virtPort); break;
		case IoManager::IOT_6BTN: updateDevice_6BTN(virtPort, oldTristateInput); break;
		case IoManager::IOT_2BTN: updateDevice_2BTN(virtPort); break;

		case IoManager::IOT_MEGA_MOUSE: {
			updateDevice_Mouse(virtPort, oldTristateInput);
			break;
		}

		case IoManager::IOT_4WP_MASTER: updateDevice_4WP_Master(virtPort); break;
		case IoManager::IOT_4WP_SLAVE: updateDevice_4WP_Slave(virtPort); break;

		case IoManager::IOT_TEAMPLAYER: {
			updateDevice_TP(virtPort, oldTristateInput);
			break;
		}

		case IoManager::IOT_NONE:
		default:
			// No device, or unknown device.
			// Assume device data is 0xFF.
			dev->deviceData = 0xFF;
			break;
	}
}

/**
 * Update a 3-button controller.
 * @param virtPort Virtual port number.
 */
void IoManagerPrivate::updateDevice_3BTN(int virtPort)
{
	/**
	 * Data formats: (D == last written MSB)
	 * TH=1: D1CBRLDU
	 * TH=0: D0SA00DU
	 */
	
	IoDevice *const dev = &ioDevices[virtPort];
	uint8_t data;
	if (dev->checkInputLine(IOPIN_TH)) {
		// TH=1.
		data = (dev->buttons & 0x3F) | 0x40;
	} else {
		// TH=0.
		data = (dev->buttons & 0xC0) >> 2;
		data |= (dev->buttons & 0x03);
	}
	
	dev->deviceData = data;
}

/**
 * Update a 6-button controller.
 * @param virtPort Virtual port number.
 * @param oldTristateInput Previous MD data, adjusted for tristate control.
 */
void IoManagerPrivate::updateDevice_6BTN(int virtPort, uint8_t oldTristateInput)
{
	IoDevice *const dev = &ioDevices[virtPort];
	uint8_t data;

	if (!(oldTristateInput & IOPIN_TH) && dev->checkInputLine(IOPIN_TH)) {
		// IOPIN_TH rising edge.
		// Increment the counter.
		dev->counter = ((dev->counter + 2) & 0x06);
		
		// Reset the scanline counter.
		dev->scanlines = 0;
	}
	
	// Use the TH counter to determine the controller state.
	// TODO: There should be a 2-NOP delay between TH change and reaction...
	const int idx = (dev->counter | (dev->checkInputLine(IOPIN_TH) ? 0 : 1));
	switch (idx) {
		case 0:
		case 2:
		case 4:
			// TH=1: First/Second/Third
			// Format: D1CBRLDU
			// (Same as 3-button.)
			data = (dev->buttons & 0x3F) | 0x40;
			break;
		
		case 1:
		case 3:
			// TH=0: First/Second
			// Format: D0SA00DU
			// (Same as 6-button.)
			data = (dev->buttons & 0xC0) >> 2;
			data |= (dev->buttons & 0x03);
			break;
		
		case 5:
			// TH=0: Third
			// Format: D0SA0000
			data = (dev->buttons & 0xC0) >> 2;
			break;
		
		case 6:
			// TH=1: Fourth
			// Format: D1CBMXYZ
			data = (dev->buttons & 0x30) | 0x40;
			data |= ((dev->buttons & 0xF00) >> 8);
			break;
		
		case 7:
			// TH=0: Fourth
			// Format: D0SA1111
			data = (dev->buttons & 0xC0) >> 2;
			data |= 0x0F;
			break;
		
		default:
			data = 0xFF;
			break;
	}
	
	dev->deviceData = data;
}

/**
 * Update a 2-button controller.
 * @param virtPort Virtual port number.
 */
void IoManagerPrivate::updateDevice_2BTN(int virtPort)
{
	/**
	 * Data format: (x == tristate value)
	 * - xxCBRLDU
	 * B == button 1
	 * C == button 2
	 */
	IoDevice *const dev = &ioDevices[virtPort];
	dev->deviceData = (0xC0 | (ioDevices[virtPort].buttons & 0x3F));
}

/**
 * Update a Mega Mouse controller.
 * @param virtPort Virtual port number.
 * @param oldTristateInput Previous MD data, adjusted for tristate control.
 */
void IoManagerPrivate::updateDevice_Mouse(int virtPort, uint8_t oldTristateInput)
{
	/**
	 * Sega Mega Mouse protocol documentation by Charles MacDonald:
	 * http://gendev.spritesmind.net/forum/viewtopic.php?t=579
	 */

	IoDevice *const dev = &ioDevices[virtPort];

	// Check for RESET.
	if (dev->checkInputLine(IOPIN_TH)) {
		// TH line is high. Reset the counter.
		dev->counter = 0;
	} else {
		// Check if the device data needs to be updated.
		if (dev->counter == 0) {
			// Wait for TH falling edge.
			if ((oldTristateInput & IOPIN_TH) && !dev->checkInputLine(IOPIN_TH)) {
				// TH falling edge.
				dev->counter++;
				latchMegaMouse(virtPort);
			}
		} else {
			// Check for TR transition.
			// NOTE: checkInputLine returns 0 or IOPIN_TR.
			if ((oldTristateInput & IOPIN_TR) != dev->checkInputLine(IOPIN_TR)) {
				// IOPIN_TR has changed.
				dev->counter++;
			}
		}
	}

	// Determine the new device data.
	uint8_t ret;
	switch (dev->counter) {
		case 0:
			// ID #0: $0 (+BF)
			ret = 0x10;
			break;
		case 1:
			// ID #1: $B (+BF)
			ret = 0x1B;
			break;
		case 2:
			// ID #2: $F (-BF)
			ret = 0x0F;
			break;
		case 3:
			// ID #3: $F (+BF)
			ret = 0x1F;
			break;
		case 4:
			// Axis sign and overflow. (-BF)
			// Format: [YOVER XOVER YSIGN XSIGN]
			// OVER == overflow occurred
			// SIGN == 0 for positive, 1 for negative
			ret = dev->data.mouse.latch.signOver;
			break;
		case 5:
			// Mouse buttons. (+BF)
			// Format: [START MIDDLE RIGHT LEFT]

			// NOTE: Mega Mouse buttons are Active High.
			// m_buttons is active low, so it needs to be inverted.
			ret = 0x10 | (~dev->buttons & 0x0F);
			break;
		case 6:
			// X axis MSN. (-BF)
			ret = (dev->data.mouse.latch.relX >> 4) & 0xF;
			break;
		case 7:
			// X axis LSN. (+BF)
			ret = 0x10 | (dev->data.mouse.latch.relX & 0xF);
			break;
		case 8:
			// Y axis MSN. (-BF)
			ret = (dev->data.mouse.latch.relY >> 4) & 0xF;
			break;
		case 9:
		default:
			// Y axis LSN. (+BF)
			// Also returned if the mouse is polled
			// more than 10 times.
			ret = 0x10 | (dev->data.mouse.latch.relY & 0xF);
			break;
	}

	// Save the device data.
	dev->deviceData = ret;
}

/**
 * Latch relX, relY, and signOver for a Sega Mega Mouse.
 * @param virtPort Virtual port number.
 */
void IoManagerPrivate::latchMegaMouse(int virtPort)
{
	IoDevice *const dev = &ioDevices[virtPort];

	// TODO: Optimize this function!

	// Format: [YOVER XOVER YSIGN XSIGN]
	// OVER == overflow occurred
	// SIGN == 0 for positive, 1 for negative

	dev->data.mouse.latch.signOver = 0;

	// X axis.
	if (dev->data.mouse.relX > 255) {
		dev->data.mouse.latch.signOver = 0x4;
		dev->data.mouse.latch.relX = 255;
	} else if (dev->data.mouse.relX >= 0) {
		dev->data.mouse.latch.signOver = 0x0;
		dev->data.mouse.latch.relX = (dev->data.mouse.relX & 0xFF);
	} else if (dev->data.mouse.relX < -255) {
		dev->data.mouse.latch.signOver = 0x5;
		dev->data.mouse.latch.relX = 255;
	} else /*if (dev->data.mouse.relX < 0)*/ {
		dev->data.mouse.latch.signOver = 0x1;
		dev->data.mouse.latch.relX = (dev->data.mouse.relX & 0xFF);
	}

	// Y axis.
	if (dev->data.mouse.relY > 255) {
		dev->data.mouse.latch.signOver |= 0x8;
		dev->data.mouse.latch.relY = 255;
	} else if (dev->data.mouse.relY >= 0) {
		dev->data.mouse.latch.relY = (dev->data.mouse.relY & 0xFF);
	} else if (dev->data.mouse.relY < -255) {
		dev->data.mouse.latch.signOver |= 0xA;
		dev->data.mouse.latch.relY = 255;
	} else /*if (dev->data.mouse.relY < 0)*/ {
		dev->data.mouse.latch.signOver |= 0x2;
		dev->data.mouse.latch.relY = (dev->data.mouse.relY & 0xFF);
	}

	// Clear the accumulators.
	dev->data.mouse.relX = 0;
	dev->data.mouse.relY = 0;
}

/**
 * Update a Team Player device.
 * @param physPort Physical controller port.
 * @param oldTristateInput Previous MD data, adjusted for tristate control.
 */
void IoManagerPrivate::updateDevice_TP(int physPort, uint8_t oldTristateInput)
{
	assert(physPort >= IoManager::PHYSPORT_1 && physPort <= IoManager::PHYSPORT_2);

	IoDevice *const dev = &ioDevices[physPort];

	// Check if either TH or TR has changed.
	// NOTE: checkInputLine(line) returns either 0 or line.
	// TODO: Optimize this, maybe?
	if ((oldTristateInput & IOPIN_TH) != dev->checkInputLine(IOPIN_TH) ||
	    (oldTristateInput & IOPIN_TR) != dev->checkInputLine(IOPIN_TR))
	{
		// Check if TH is high.
		if (dev->checkInputLine(IOPIN_TH)) {
			// TH high. Reset the counter.
			dev->counter = TP_DT_INIT;
		} else {
			// Increment the counter.
			dev->counter++;
		}
	}

	if (dev->counter >= TP_DT_MAX) {
		// Counter has overflowed.
		dev->counter = TP_DT_MAX;
		dev->deviceData = 0xFF;
		return;
	}

	// Check the controller data index table.
	uint8_t data = 0;
	switch (dev->counter) {
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
			data = dev->data.tp.padTypes[dev->counter - TP_DT_PADTYPE_A];
			break;

		default:
			// Check the controller data index table.
			// TODO: What value should be returned for errors?
			int adj_counter = (dev->counter - TP_DT_PADA_RLDU);
			if (adj_counter >= ARRAY_SIZE(dev->data.tp.ctrlIndexTbl)) {
				// Counter is out of bounds.
				data = 0x0F;
				break;
			}

			// Look up the data type from the controller index table.
			TP_DataType dataType = (TP_DataType)dev->data.tp.ctrlIndexTbl[adj_counter];
			if (dataType < TP_DT_PADA_RLDU || dataType >= TP_DT_MAX) {
				// Invalid counter state.
				data = 0x0F;
				break;
			}

			// Readjust the counter for the correct data type.
			adj_counter = dataType - TP_DT_PADA_RLDU;

			// Determine the virtual port base.
			const int virtPortBase = (physPort == 0
						? IoManager::VIRTPORT_TP1A
						: IoManager::VIRTPORT_TP2A);

			// Controller data.
			// TODO: Move dtPerPortMax to a class constant?
			static const int dtPerPortMax = (TP_DT_PADA_MXYZ - TP_DT_PADA_RLDU) + 1;
			const int virtPort = virtPortBase + (adj_counter / dtPerPortMax);
			const int shift = (adj_counter % dtPerPortMax) * 4;

			data = (ioDevices[virtPort].buttons >> shift) & 0xF;
			break;
	}

	// TL should match TR.
	// (from Genesis Plus GX)
	// NOTE: TR is always an MD output line.
	if (dev->checkInputLine(IOPIN_TR))
		data |= IOPIN_TL;
	else
		data &= ~IOPIN_TL;

	dev->deviceData = data;
}

/**
 * Team Player: Rebuild the controller index table.
 * @param physPort Physical controller port.
 */
void IoManagerPrivate::rebuildCtrlIndexTable(int physPort)
{
	// Check controller types.
	assert(physPort >= IoManager::PHYSPORT_1 && physPort <= IoManager::PHYSPORT_2);

	IoDevice *const dev = &ioDevices[physPort];
	assert(dev->type == IoManager::IOT_TEAMPLAYER);

	// Determine the virtual port base.
	const int virtPortBase = (physPort == 0
				? IoManager::VIRTPORT_TP1A
				: IoManager::VIRTPORT_TP2A);

	int i = 0;	// data.tp.ctrlIndexTbl index
	for (int pad = 0; pad < 4; pad++) {
		// TODO: Move dtPerPortMax to a class constant?
		static const int dtPerPortMax = (TP_DT_PADA_MXYZ - TP_DT_PADA_RLDU) + 1;
		const int dtBase = (TP_DT_PADA_RLDU + (pad * dtPerPortMax));
		const int virtPort = virtPortBase + pad;

		switch (ioDevices[virtPort].type) {
			case IoManager::IOT_NONE:
			default:
				dev->data.tp.padTypes[pad] = TP_PT_NONE;
				break;

			case IoManager::IOT_3BTN:
				dev->data.tp.padTypes[pad] = TP_PT_3BTN;
				dev->data.tp.ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 0);
				dev->data.tp.ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 1);
				break;

			case IoManager::IOT_6BTN:
				dev->data.tp.padTypes[pad] = TP_PT_6BTN;
				dev->data.tp.ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 0);
				dev->data.tp.ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 1);
				dev->data.tp.ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 2);
				break;
		}
	}

	// Set the rest of the controller data indexes to DT_MAX.
	for (int x = i; x < ARRAY_SIZE(dev->data.tp.ctrlIndexTbl); x++)
		dev->data.tp.ctrlIndexTbl[x] = TP_DT_MAX;
}

/**
 * Update the 4WP Master device.
 * @param physPort Physical controller port. (MUST be PHYSPORT_2!)
 */
void IoManagerPrivate::updateDevice_4WP_Master(int physPort)
{
	assert(physPort == IoManager::PHYSPORT_2);
	IoDevice *const dev = &ioDevices[physPort];

	// Update the slave port number.
	ea4wp_curPlayer = (dev->mdData_tris >> 4) & 0x07;

	// Update the slave device.
	assert(ioDevices[IoManager::PHYSPORT_1].type == IoManager::IOT_4WP_SLAVE);
	updateDevice_4WP_Slave(IoManager::PHYSPORT_1);

	// Device data is always 0x7F.
	dev->deviceData = 0x7F;
}

/**
 * Update the 4WP Slave device.
 * @param physPort Physical controller port. (MUST be PHYSPORT_1!)
 */
void IoManagerPrivate::updateDevice_4WP_Slave(int physPort)
{
	assert(physPort == IoManager::PHYSPORT_1);
	IoDevice *const dev = &ioDevices[physPort];

	if (ea4wp_curPlayer < 0 || ea4wp_curPlayer >= 4) {
		// Multitap detection.
		dev->deviceData = 0x70;
		return;
	}

	// Update the current virtual gamepad.
	const int virtPort = IoManager::VIRTPORT_4WPA + ea4wp_curPlayer;
	IoDevice *const virtDev = &ioDevices[virtPort];
	assert(virtDev->type >= IoManager::IOT_NONE && virtDev->type <= IoManager::IOT_6BTN);

	virtDev->ctrl = dev->ctrl;
	virtDev->mdData = dev->mdData;
	updateDevice(virtPort);
	dev->deviceData = virtDev->deviceData;
}

}
