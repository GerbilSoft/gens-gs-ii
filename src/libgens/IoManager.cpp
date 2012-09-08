/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.cpp: I/O manager.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#include "IoManager.hpp"

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

namespace LibGens
{

IoManager::IoManager()
{
	// Set the default controller types for IOPORT_1 and IOPORT_2.
	m_ioDevices[VIRTPORT_1].type = IOT_6BTN;
	m_ioDevices[VIRTPORT_2].type = IOT_3BTN;
	
	// Reset all devices.
	reset();
}


/**
 * Reset all controllers.
 */
void IoManager::reset(void)
{
	for (int i = 0; i < NUM_ELEMENTS(m_ioDevices); i++)
		m_ioDevices[i].reset();
}


/**
 * Update the scanline counter for all controllers.
 * This is used by the 6-button controller,
 * which resets its internal counter after
 * around 25 scanlines of no TH rising edges.
 */
void IoManager::doScanline(void)
{
	for (int i = VIRTPORT_1; i < VIRTPORT_MAX; i++) {
		IoDevice *dev = &m_ioDevices[i];
		if (dev->type != IOT_6BTN)
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


/** MD-side controller functions. **/

/**
 * Read data from an MD controller port.
 * @param physPort Physical port number.
 * @return Data.
 */
uint8_t IoManager::readDataMD(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	// Mask the data according to the tristate control.
	// Tristate is 0 for input and 1 for output.
	// Note that tristate bit 7 is used for TH interrupt.
	// All input bits should read the device data.
	// All output bits should read the MD data.
	return m_ioDevices[physPort].readData();
}

/**
 * Write data to an MD controller port.
 * @param physPort Physical port number.
 * @param data Data.
 */
void IoManager::writeDataMD(int physPort, uint8_t data)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	m_ioDevices[physPort].mdData = data;
	updateDevice(physPort);
	// TODO: Update the device.
	// TODO: 4WP needs to copy this to the active device.
}


/**
 * Read the tristate register from an MD controller port.
 * @param physPort Physical port number.
 * @return Tristate register.
 */
uint8_t IoManager::readCtrlMD(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	return m_ioDevices[physPort].ctrl;
}

/**
 * Write tristate register data to an MD controller port.
 * @param physPort Physical port number.
 * @param ctrl Tristate register data.
 */
void IoManager::writeCtrlMD(int physPort, uint8_t ctrl)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	m_ioDevices[physPort].ctrl = ctrl;
	updateDevice(physPort);
	// TODO: 4WP needs to copy this to the active device.
}


/** Serial I/O virtual functions. **/

// TODO: Baud rate delay handling, TL/TR handling.
// TODO: Trigger IRQ 2 on data receive if interrupt is enabled.
// NOTE: Serial mode used is 8n1: 1 start, 8 data, 1 stop = 10 baud per byte.
uint8_t IoManager::readSerCtrl(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	return (m_ioDevices[physPort].serCtrl & 0xF8);
}
void IoManager::writeSerCtrl(int physPort, uint8_t serCtrl)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	m_ioDevices[physPort].serCtrl = serCtrl;
}
uint8_t IoManager::readSerTx(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	return m_ioDevices[physPort].serLastTx;
}
void IoManager::writeSerTx(int physPort, uint8_t data)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	m_ioDevices[physPort].serLastTx = data;
}
uint8_t IoManager::readSerRx(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	// TODO
	return 0xFF;
}


/**
 * I/O device update function.
 */
void IoManager::update(void)
{
	// TODO: Update keyboard input for al ports.

	// Update all physical ports.
	for (int i = PHYSPORT_1; i < PHYSPORT_MAX; i++)
		updateDevice(i);
}


/**
 * Update an I/O device's state based on ctrl/data lines.
 * @param physPort Physical port number.
 */
void IoManager::updateDevice(int physPort)
{
	assert(physPort >= 0 && physPort <= 2);

	IoDevice *dev = &m_ioDevices[physPort];
	const bool oldSelect = dev->isSelect();
	dev->updateSelectLine();

	// Handle devices that require updating when certain lines change.
	switch (dev->type) {
		case IOT_3BTN: updateDevice_3BTN(physPort); break;
		case IOT_6BTN: updateDevice_6BTN(physPort, oldSelect); break;
		case IOT_2BTN: updateDevice_2BTN(physPort); break;
		
		// TODO: Implement Team Player, 4WP, and Mega Mouse.
		default:
			break;
	}
}

void IoManager::updateDevice_3BTN(int port)
{
	/**
	 * Data formats: (D == last written MSB)
	 * TH=1: D1CBRLDU
	 * TH=0: D0SA00DU
	 */
	
	IoDevice *dev = &m_ioDevices[port];
	uint8_t data;
	if (dev->isSelect()) {
		// TH=1.
		data = (dev->buttons & 0x3F) | 0x40;
	} else {
		// TH=0.
		data = (dev->buttons & 0xC0) >> 2;
		data |= (dev->buttons & 0x03);
	}
	
	dev->deviceData = data;
}

void IoManager::updateDevice_6BTN(int virtPort, bool oldSelect)
{
	IoDevice *dev = &m_ioDevices[virtPort];
	uint8_t data;

	if (!oldSelect && dev->isSelect()) {
		// IOPIN_TH rising edge.
		// Increment the counter.
		dev->counter = ((dev->counter + 2) & 0x06);
		
		// Reset the scanline counter.
		dev->scanlines = 0;
	}
	
	// Use the TH counter to determine the controller state.
	// TODO: Should be a 2-NOP delay between TH change and reaction...
	switch (dev->counter | !dev->isSelect()) {
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
			data |= (dev->buttons & 0x03);
			data |= 0x0F;
			break;
		
		default:
			data = 0xFF;
			break;
	}
	
	dev->deviceData = data;
}

void IoManager::updateDevice_2BTN(int virtPort)
{
	/**
	 * Data format: (x == tristate value)
	 * - xxCBRLDU
	 * B == button 1
	 * C == button 2
	 */
	IoDevice *dev = &m_ioDevices[virtPort];
	dev->mdData = (0xC0 | (m_ioDevices[virtPort].buttons & 0x3F));
}

}
