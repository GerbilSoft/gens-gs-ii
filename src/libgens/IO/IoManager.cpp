/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.cpp: I/O manager.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2013 by David Korth.                                 *
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

#define __LIBGENS_IN_IOMANAGER_CLASS__
#include "IoManager_p.hpp"

// C includes. (C++ namespace)
#include <cassert>

// C++ includes.
#include <string>
using std::string;

// ARRAY_SIZE(x)
#include "macros/common.h"

namespace LibGens
{

IoManager::IoManager()
	: d(new IoManagerPrivate(this))
{ }

IoManager::~IoManager()
{
	delete d;
}

/**
 * Reset all devices.
 */
void IoManager::reset(void)
{
	d->reset();
}

/**
 * Update the scanline counter for all controllers.
 * This is used by the 6-button controller,
 * which resets its internal counter after
 * around 25 scanlines of no TH rising edges.
 */
void IoManager::doScanline(void)
{
	d->doScanline();
}

/** General device type functions. **/

/**
 * Get the number of buttons present on a specific type of device.
 * @param ioType Device type.
 * @return Number of buttons.
 */
int IoManager::NumDevButtons(IoType_t ioType)
{
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);
	if (ioType < IOT_NONE || ioType >= IOT_MAX)
		return 0;

	return IoManagerPrivate::ioDevInfo[ioType].btnCount;
}

/**
 * Is a given device type usable?
 * @param ioType Device type.
 * @return True if usable; false if not.
 */
bool IoManager::IsDevTypeUsable(IoType_t ioType)
{
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);
	if (ioType < IOT_NONE || ioType >= IOT_MAX)
		return false;

#ifdef NDEBUG
	// Release build.
	// Return the actual "isUsable" status.
	return IoManagerPrivate::ioDevInfo[ioType].isUsable;
#else
	// Debug build.
	// All controllers are usable.
	// TODO: Mark "unusable" controllers as "partially implemented"?
	return true;
#endif
}

/**
 * Get the FourCC for a given device type.
 * @param ioType Device type.
 * @return FourCC for the device type.
 */
uint32_t IoManager::IoTypeToFourCC(IoType_t ioType)
{
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);
	if (ioType < IOT_NONE || ioType >= IOT_MAX)
		return 0;

	return IoManagerPrivate::ioDevInfo[ioType].fourCC;
}

/**
 * Get the device type for a given FourCC.
 * @param ioType Device type.
 * @return Device type, or IOT_MAX if the FourCC doesn't match any device type.
 */
IoManager::IoType_t IoManager::FourCCToIoType(uint32_t fourCC)
{
	// Verify that multi-character character constants work properly.
	static_assert('NONE' == 0x4E4F4E45, "Multi-character character constant 'NONE' is invalid.");
	static_assert('    ' == 0x20202020, "Multi-character character constant '    ' is invalid.");
	static_assert('\0\0\0\0' == 0x00000000, "Multi-character character constant '\\0\\0\\0\\0' is invalid.");

	// Special case: FourCC == 0 || FourCC == '    '
	// Assume these are invalid in order to force default settings.
	if (fourCC == 0 || fourCC == '    ')
		return IOT_MAX;

	for (int i = 0; i < ARRAY_SIZE(IoManagerPrivate::ioDevInfo); i++) {
		if (IoManagerPrivate::ioDevInfo[i].fourCC == fourCC)
			return (IoType_t)i;
	}

	// FourCC not found.
	return IOT_MAX;
}

/**
 * Convert a string to a FourCC.
 * @param str String.
 * @return FourCC, or 0 if the string was not four characters long.
 */
uint32_t IoManager::StringToFourCC(const std::string& str)
{
	if (str.size() != 4)
		return 0;

	uint32_t fourCC = 0;
	for (int i = 0; i < 4; i++) {
		fourCC <<= 8;
		fourCC |= (uint8_t)str.at(i);
	}
	return fourCC;
}

/**
 * Convert a FourCC to a string.
 * @param fourCC FourCC.
 * @return FourCC as a string.
 */
string IoManager::FourCCToString(uint32_t fourCC)
{
	string str(4, '\0');
	for (int i = 3; i >= 0; i--) {
		str.at(i) = (fourCC & 0xFF);
		fourCC >>= 8;
	}
	return str;
}

/** Get/set device types. **/

/**
 * Get the device type for a given virtual port.
 * @param virtPort Virtual port.
 */
IoManager::IoType_t IoManager::devType(VirtPort_t virtPort) const
{
	assert(virtPort >= VIRTPORT_1 && virtPort < VIRTPORT_MAX);
	return d->ioDevices[virtPort].type;
}

/**
 * Set the device type for a given virtual port.
 * @param virtPort Virtual port.
 * @param ioType New device type.
 */
void IoManager::setDevType(VirtPort_t virtPort, IoType_t ioType)
{
	assert(virtPort >= VIRTPORT_1 && virtPort < VIRTPORT_MAX);
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);

	if (virtPort > VIRTPORT_EXT) {
		// Virtual ports above EXT do not support anything
		// except 3-button and 6-button controllers.
		assert(ioType >= IOT_NONE && ioType <= IOT_6BTN);
		// TODO: Set IOT_NONE if the assertion fails in release?
		if (ioType < IOT_NONE || ioType > IOT_6BTN)
			return;
	}

	IoManagerPrivate::IoDevice *const dev = &d->ioDevices[virtPort];
	dev->type = ioType;
	// Only reset device data so we don't screw up emulation.
	dev->resetDev();

	// Rebuild Team Player controller index tables for TP devices.
	switch (virtPort) {
		// System controller ports.
		case VIRTPORT_1:
		case VIRTPORT_2:
			if (dev->type == IoManager::IOT_TEAMPLAYER)
				d->rebuildCtrlIndexTable(virtPort);
			break;

		// Team Player, Port 1.
		case VIRTPORT_TP1A:
		case VIRTPORT_TP1B:
		case VIRTPORT_TP1C:
		case VIRTPORT_TP1D:
			if (d->ioDevices[VIRTPORT_1].type == IoManager::IOT_TEAMPLAYER)
				d->rebuildCtrlIndexTable(VIRTPORT_1);
			break;

		// Team Player, Port 1.
		case VIRTPORT_TP2A:
		case VIRTPORT_TP2B:
		case VIRTPORT_TP2C:
		case VIRTPORT_TP2D:
			if (d->ioDevices[VIRTPORT_2].type == IoManager::IOT_TEAMPLAYER)
				d->rebuildCtrlIndexTable(VIRTPORT_2);
			break;

		default:
			break;
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
	return d->ioDevices[physPort].readData();
}

/**
 * Write data to an MD controller port.
 * @param physPort Physical port number.
 * @param data Data.
 */
void IoManager::writeDataMD(int physPort, uint8_t data)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	d->ioDevices[physPort].mdData = data;
	d->updateDevice(physPort);
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
	return d->ioDevices[physPort].ctrl;
}

/**
 * Write tristate register data to an MD controller port.
 * @param physPort Physical port number.
 * @param ctrl Tristate register data.
 */
void IoManager::writeCtrlMD(int physPort, uint8_t ctrl)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	d->ioDevices[physPort].ctrl = ctrl;
	d->updateDevice(physPort);
	// TODO: 4WP needs to copy this to the active device.
}


/** Serial I/O virtual functions. **/

// TODO: Baud rate delay handling, TL/TR handling.
// TODO: Trigger IRQ 2 on data receive if interrupt is enabled.
// NOTE: Serial mode used is 8n1: 1 start, 8 data, 1 stop = 10 baud per byte.
uint8_t IoManager::readSerCtrl(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	return (d->ioDevices[physPort].serCtrl & 0xF8);
}
void IoManager::writeSerCtrl(int physPort, uint8_t serCtrl)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	d->ioDevices[physPort].serCtrl = serCtrl;
}
uint8_t IoManager::readSerTx(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	return d->ioDevices[physPort].serLastTx;
}
void IoManager::writeSerTx(int physPort, uint8_t data)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	d->ioDevices[physPort].serLastTx = data;
}
uint8_t IoManager::readSerRx(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);
	// TODO
	return 0xFF;
}

/** SMS-side controller functions. **/

/**
 * Write to the SMS I/O port control register.
 * @param ctrl Control register data.
 */
void IoManager::writeCtrlSMS(uint8_t ctrl)
{
	// TODO: Untested!

	// FIXME: On Japanese hardware, this should be a no-op.
	// ...or, maintain the state and then apply it if
	// the system region is changed back to non-JP.

	/**
	 * SMS I/O port control register layout. ($3F)
	 * Reference: http://www.smspower.org/Development/PeripheralPorts
	 *
	 * 7 == Port B TH pin output level
	 * 6 == Port B TR pin output level
	 * 5 == Port A TH pin output level
	 * 4 == Port A TR pin output level
	 * 3 == Port B TH pin direction
	 * 2 == Port B TR pin direction
	 * 1 == Port A TH pin direction
	 * 0 == Port A TR pin direction
	 *
	 * Direction is 1 for input, 0 for output.
	 * System default is 0x00.
	 */

	// Calculate MD control lines.
	// MD ctrl: [0 TH TR TL R L D U]
	const uint8_t ctrl1 = ((ctrl & 0x3) << 5);
	const uint8_t ctrl2 = ((ctrl & 0xC) << 3);

	// Calculate MD data lines.
	// MD data: [0 TH TR TL R L D U]
	const uint8_t data1 = ((ctrl & 0x30) << 1);
	const uint8_t data2 = ((ctrl & 0xC0) >> 1);

	// Write the control and data values.
	writeCtrlMD(PHYSPORT_1, ctrl1);
	writeDataMD(PHYSPORT_1, data1);
	writeCtrlMD(PHYSPORT_2, ctrl2);
	writeDataMD(PHYSPORT_2, data2);
}

/**
 * Read the SMS $DC register: I/O port A and B.
 * @return SMS $DC register.
 */
uint8_t IoManager::readDataSMS_DC(void) const
{
	// TODO: Untested!

	/**
	 * SMS I/O port A and B register layout. ($DC)
	 * Read-only; all pins are inputs.
	 * Reference: http://www.smspower.org/Development/PeripheralPorts
	 *
	 * 7 == Port B Down
	 * 6 == Port B Up
	 * 5 == Port A TR
	 * 4 == Port A TL
	 * 3 == Port A Right
	 * 2 == Port A Left
	 * 1 == Port A Down
	 * 0 == Port A Up
	 */

	// MD data: [0 TH TR TL R L D U]
	const uint8_t data1 = readDataMD(PHYSPORT_1);
	const uint8_t data2 = readDataMD(PHYSPORT_2);

	// Calculate the SMS data.
	uint8_t data = (data1 & 0x3F);
	data |= ((data2 & 0x3) << 6);
	return data;
}

/**
 * Read the SMS $DD register: I/O port B and miscellaneous.
 * @return SMS $DD register.
 */
uint8_t IoManager::readDataSMS_DD(void) const
{
	// TODO: Untested!
	// TODO: Implement the CONT line and the Reset button.
	// CONT apparently returns 1 on 8-bit and 0 on MD.

	/**
	 * SMS I/O port B and miscellaneous register layout. ($DD)
	 * Read-only; all pins are inputs.
	 * Reference: http://www.smspower.org/Development/PeripheralPorts
	 *
	 * 7 == Port B TH
	 * 6 == Port A TH
	 * 5 == Cartridge slot CONT pin
	 * 4 == Reset button (active low)
	 * 3 == Port B TR
	 * 2 == Port B TL
	 * 1 == Port B Right
	 * 0 == Port B Left
	 */

	// MD data: [0 TH TR TL R L D U]
	const uint8_t data1 = readDataMD(PHYSPORT_1);
	const uint8_t data2 = readDataMD(PHYSPORT_2);

	// Calculate the SMS data.
	uint8_t data = ((data2 & 0x3C) >> 2);
	data |= 0x20; // CONT pin
	data |= (data1 & IoManagerPrivate::IOPIN_TH);
	data |= ((data2 & IoManagerPrivate::IOPIN_TH) << 1);
	return data;
}

/**
 * Update an I/O device.
 * @param virtPort Virtual port.
 * @param buttons New button state.
 */
void IoManager::update(int virtPort, uint32_t buttons)
{
	d->update(virtPort, buttons);
}

/** ZOMG savestate functions. **/

/**
 * Save the I/O state. (MD)
 * @param state Zomg_MD_IoSave_t struct to save to.
 */
void IoManager::zomgSaveMD(Zomg_MD_IoSave_t *state) const
{
	// Port 1
	state->port1_data = d->ioDevices[PHYSPORT_1].mdData;
	state->port1_ctrl = d->ioDevices[PHYSPORT_1].ctrl;
	state->port1_ser_tx = d->ioDevices[PHYSPORT_1].serLastTx;
	state->port1_ser_rx = 0xFF; // TODO
	state->port1_ser_ctrl = d->ioDevices[PHYSPORT_1].serCtrl;

	// Port 2
	state->port2_data = d->ioDevices[PHYSPORT_2].mdData;
	state->port2_ctrl = d->ioDevices[PHYSPORT_2].ctrl;
	state->port2_ser_tx = d->ioDevices[PHYSPORT_2].serLastTx;
	state->port2_ser_rx = 0xFF; // TODO
	state->port2_ser_ctrl = d->ioDevices[PHYSPORT_2].serCtrl;

	// Port 3 (EXT)
	state->port3_data = d->ioDevices[PHYSPORT_EXT].mdData;
	state->port3_ctrl = d->ioDevices[PHYSPORT_EXT].ctrl;
	state->port3_ser_tx = d->ioDevices[PHYSPORT_EXT].serLastTx;
	state->port3_ser_rx = 0xFF; // TODO
	state->port3_ser_ctrl = d->ioDevices[PHYSPORT_EXT].serCtrl;
}

/**
 * Restore the I/O state. (MD)
 * @param state Zomg_MD_IoSave_t struct to restore from.
 */
void IoManager::zomgRestoreMD(const Zomg_MD_IoSave_t *state)
{
	// Port 1
	d->ioDevices[PHYSPORT_1].mdData = state->port1_data;
	d->ioDevices[PHYSPORT_1].ctrl = state->port1_ctrl;
	d->ioDevices[PHYSPORT_1].serLastTx = state->port1_ser_tx;
	//d->ioDevices[PHYSPORT_1].serLastRx = state->port1_ser_rx;	// TODO
	d->ioDevices[PHYSPORT_1].serCtrl = state->port1_ser_ctrl;

	// Port 2
	d->ioDevices[PHYSPORT_2].mdData = state->port2_data;
	d->ioDevices[PHYSPORT_2].ctrl = state->port2_ctrl;
	d->ioDevices[PHYSPORT_2].serLastTx = state->port2_ser_tx;
	//d->ioDevices[PHYSPORT_2].serLastRx = state->port2_ser_rx;	// TODO
	d->ioDevices[PHYSPORT_2].serCtrl = state->port2_ser_ctrl;

	// Port 3 (EXT)
	d->ioDevices[PHYSPORT_EXT].mdData = state->port3_data;
	d->ioDevices[PHYSPORT_EXT].ctrl = state->port3_ctrl;
	d->ioDevices[PHYSPORT_EXT].serLastTx = state->port3_ser_tx;
	//d->ioDevices[PHYSPORT_EXT].serLastRx = state->port3_ser_rx;	// TODO
	d->ioDevices[PHYSPORT_EXT].serCtrl = state->port3_ser_ctrl;

	// Update the ports.
	d->updateDevice(PHYSPORT_1);
	d->updateDevice(PHYSPORT_2);
	d->updateDevice(PHYSPORT_EXT);
}

}
