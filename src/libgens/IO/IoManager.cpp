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

// I/O devices.
#include "Device.hpp"
#include "Io3BTN.hpp"
#include "Io6BTN.hpp"
#include "Io2BTN.hpp"
#include "IoMegaMouse.hpp"

// Multitaps.
#include "IoTeamPlayer.hpp"
#include "Io4WPM.hpp"
#include "Io4WPS.hpp"
#include "IoMasterTap.hpp"

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
	for (int i = 0; i < ARRAY_SIZE(d->ioDevices); i++) {
		IO::Device *const dev = d->ioDevices[i];
		if (dev != nullptr) {
			dev->reset();
		}
	}
}

/**
 * Update the scanline counter for all controllers.
 * This is used by the 6-button controller,
 * which resets its internal counter after
 * around 25 scanlines of no TH rising edges.
 */
void IoManager::doScanline(void)
{
	for (int i = 0; i < ARRAY_SIZE(d->ioDevices); i++) {
		IO::Device *const dev = d->ioDevices[i];
		if (dev != nullptr) {
			dev->update_onScanline();
		}
	}
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

	const IO::Device *const dev = d->ioDevices[virtPort];
	// Physical ports must be allocated.
	assert(virtPort > VIRTPORT_EXT || dev != nullptr);

	if (!dev) {
		return IOT_NONE;
	}
	return d->ioDevices[virtPort]->type();
}

/**
 * Set the device type for a given virtual port.
 * @param virtPort Virtual port.
 * @param ioType New device type.
 * TODO: Return an error code?
 */
void IoManager::setDevType(VirtPort_t virtPort, IoType_t ioType)
{
	// TODO: This will require creating a new I/O device.
	assert(virtPort >= VIRTPORT_1 && virtPort < VIRTPORT_MAX);
	assert(ioType >= IOT_NONE && ioType < IOT_MAX);

	// Does the port actually need to be changed?
	IO::Device *const old_dev = d->ioDevices[virtPort];
	if (ioType == IOT_NONE) {
		if (!old_dev || old_dev->type() == IOT_NONE) {
			// Device does not need to be changed.
			return;
		}
	} else {
		if (old_dev && old_dev->type() == ioType) {
			// Device does not need to be changed.
			return;
		}
	}

	// TODO: Use bitfields to speed this up?
	if (virtPort > VIRTPORT_EXT) {
		// Team Player supports 3BTN, 6BTN, and MOUS.
		// Other ports only support 3BTN and 6BTN.
		if (virtPort >= VIRTPORT_TP1A && virtPort <= VIRTPORT_TP2D) {
			// Team Player virtual ports support 3BTN, 6BTN, and MOUS.
			assert((ioType >= IOT_NONE && ioType <= IOT_6BTN) || ioType == IOT_MEGA_MOUSE);
			if (ioType < IOT_NONE || (ioType > IOT_6BTN && ioType != IOT_MEGA_MOUSE))
				return;
		} else if (virtPort >= VIRTPORT_MTAP1A && virtPort <= VIRTPORT_MTAP2D) {
			// Master Tap virtual ports only support 2BTN.
			assert(ioType == IOT_NONE || ioType == IOT_2BTN);
			if (ioType != IOT_NONE && ioType != IOT_2BTN)
				return;
		} else {
			// Other virtual ports only support 3BTN and 6BTN.
			assert(ioType >= IOT_NONE && ioType <= IOT_6BTN);
			if (ioType < IOT_NONE || ioType > IOT_6BTN)
				return;
		}
	}

	// Create a new device.
	// TODO: Copy MD-side data from old device using a pseudo-copy constructor...
	// TODO: Don't create TP/4WP sub-devices if the main devices are missing?
	IO::Device *dev = nullptr;
	switch (ioType) {
		case IOT_NONE:
			if (virtPort <= VIRTPORT_EXT) {
				// Must have a generic Device.
				dev = new IO::Device();
			}
			break;
		case IOT_3BTN:
			dev = new IO::Io3BTN();
			break;
		case IOT_6BTN:
			dev = new IO::Io6BTN();
			break;
		case IOT_2BTN:
			dev = new IO::Io2BTN();
			break;
		case IOT_MEGA_MOUSE:
			dev = new IO::IoMegaMouse();
			break;
		case IOT_TEAMPLAYER:
			dev = new IO::IoTeamPlayer();
			break;
		case IOT_4WP_MASTER:
			if (virtPort != VIRTPORT_2) {
				// EA 4-Way Play Master device must be on port 2.
				return;
			}
			dev = new IO::Io4WPM();
			break;
		case IOT_4WP_SLAVE:
			if (virtPort != VIRTPORT_1) {
				// EA 4-Way Play Slave device must be on port 1.
				return;
			}
			dev = new IO::Io4WPS();
			break;
		case IOT_MASTERTAP:
			dev = new IO::IoMasterTap();
			break;
		default:
			// TODO: Handle Team Player correctly.
			return;
	}

	if (dev && old_dev) {
		// Copy data from the old device.
		dev->ctrl = old_dev->ctrl;
		dev->mdData = old_dev->mdData;
		dev->mdData_tris = old_dev->mdData_tris;
		dev->deviceData = old_dev->deviceData;
		// TODO: Copy over sub-devices for multitaps.

		// Replace the old device.
		d->ioDevices[virtPort] = dev;
		delete old_dev;
	} else if (!dev) {
		// New device is nullptr.
		// Delete the old device.
		d->ioDevices[virtPort] = nullptr;
		delete old_dev;
	} else /*if (dev)*/ {
		// New device is valid; old device is nullptr.
		d->ioDevices[virtPort] = dev;
	}

	// Update multitaps.
	switch (virtPort) {
		// System controller ports.
		case VIRTPORT_1: {
			const IoType_t type = dev->type();
			if (type == IoManager::IOT_TEAMPLAYER) {
				// Team Player.
				for (int i = VIRTPORT_TP1A; i <= VIRTPORT_TP1D; i++) {
					dev->setSubDevice(i - VIRTPORT_TP1A, d->ioDevices[i]);
				}
			} else if (type == IoManager::IOT_4WP_SLAVE) {
				// EA 4-Way Play Slave device.
				for (int i = VIRTPORT_4WPA; i <= VIRTPORT_4WPD; i++) {
					dev->setSubDevice(i - VIRTPORT_4WPA, d->ioDevices[i]);
				}
			}
			break;
		}

		case VIRTPORT_2:
			if (dev->type() == IoManager::IOT_TEAMPLAYER) {
				// Team Player.
				for (int i = VIRTPORT_TP2A; i <= VIRTPORT_TP2D; i++) {
					dev->setSubDevice(i - VIRTPORT_TP2A, d->ioDevices[i]);
				}
			}
			break;

		// Team Player, Port 1.
		case VIRTPORT_TP1A:
		case VIRTPORT_TP1B:
		case VIRTPORT_TP1C:
		case VIRTPORT_TP1D: {
			IO::Device *const dev1 = d->ioDevices[VIRTPORT_1];
			if (dev1->type() == IoManager::IOT_TEAMPLAYER) {
				dev1->setSubDevice(virtPort - VIRTPORT_TP1A, dev);
			}
			break;
		}

		// Team Player, Port 1.
		case VIRTPORT_TP2A:
		case VIRTPORT_TP2B:
		case VIRTPORT_TP2C:
		case VIRTPORT_TP2D: {
			IO::Device *const dev2 = d->ioDevices[VIRTPORT_2];
			if (dev2->type() == IoManager::IOT_TEAMPLAYER) {
				dev2->setSubDevice(virtPort - VIRTPORT_TP2A, dev);
			}
			break;
		}

		// EA 4-Way Play.
		case VIRTPORT_4WPA:
		case VIRTPORT_4WPB:
		case VIRTPORT_4WPC:
		case VIRTPORT_4WPD: {
			IO::Device *const dev1 = d->ioDevices[VIRTPORT_1];
			if (dev1->type() == IoManager::IOT_4WP_SLAVE) {
				dev1->setSubDevice(virtPort - VIRTPORT_4WPA, dev);
			}
			break;
		}

		// Master Tap, Port 1.
		case VIRTPORT_MTAP1A:
		case VIRTPORT_MTAP1B:
		case VIRTPORT_MTAP1C:
		case VIRTPORT_MTAP1D: {
			IO::Device *const dev1 = d->ioDevices[VIRTPORT_1];
			if (dev1->type() == IoManager::IOT_MASTERTAP) {
				dev1->setSubDevice(virtPort - VIRTPORT_MTAP1A, dev);
			}
			break;
		}

		// Master Tap, Port 2.
		case VIRTPORT_MTAP2A:
		case VIRTPORT_MTAP2B:
		case VIRTPORT_MTAP2C:
		case VIRTPORT_MTAP2D: {
			IO::Device *const dev2 = d->ioDevices[VIRTPORT_2];
			if (dev2->type() == IoManager::IOT_MASTERTAP) {
				dev2->setSubDevice(virtPort - VIRTPORT_MTAP2A, dev);
			}
			break;
		}

		default:
			break;
	}

	// Update the device.
	if (dev) {
		dev->update();
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

	// TODO: 4WP (and maybe TP?) needs to receive updates from the virtual potrs.
	IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	uint8_t data = dev->readData();
	dev->update_onRead();	// TODO: Check for XE-1AP first?
	return data;
}

/**
 * Write data to an MD controller port.
 * @param physPort Physical port number.
 * @param data Data.
 */
void IoManager::writeDataMD(int physPort, uint8_t data)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	dev->mdData = data;
	dev->update();	// TODO: updateWithData()?
}


/**
 * Read the tristate register from an MD controller port.
 * @param physPort Physical port number.
 * @return Tristate register.
 */
uint8_t IoManager::readCtrlMD(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	const IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	return dev->ctrl;
}

/**
 * Write tristate register data to an MD controller port.
 * @param physPort Physical port number.
 * @param ctrl Tristate register data.
 */
void IoManager::writeCtrlMD(int physPort, uint8_t ctrl)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	dev->ctrl = ctrl;
	dev->update();	// TODO: updateWithCtrl()?
	// TODO: 4WP needs to copy this to the active device.
}

/** Serial I/O virtual functions. **/

// TODO: Baud rate delay handling, TL/TR handling.
// TODO: Trigger IRQ 2 on data receive if interrupt is enabled.
// NOTE: Serial mode used is 8n1: 1 start, 8 data, 1 stop = 10 baud per byte.
uint8_t IoManager::readSerCtrl(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	const IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	return (dev->serCtrl & 0xF8);
}

void IoManager::writeSerCtrl(int physPort, uint8_t serCtrl)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	dev->serCtrl = serCtrl;
}

uint8_t IoManager::readSerTx(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	const IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	return dev->serLastTx;
}

void IoManager::writeSerTx(int physPort, uint8_t data)
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

	dev->serLastTx = data;
}

uint8_t IoManager::readSerRx(int physPort) const
{
	assert(physPort >= PHYSPORT_1 && physPort < PHYSPORT_MAX);

	const IO::Device *const dev = d->ioDevices[physPort];
	assert(dev != nullptr);	// Physical ports must be allocated.

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
	// TODO: Is this port readable?

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
	 * [This is the OPPOSITE of MD!]
	 * System default is 0x00.
	 */

	// Calculate MD control lines.
	// MD ctrl: [0 TH TR TL R L D U]
	const uint8_t ctrl1 = (((ctrl & 0x3) << 5) ^ 0x60);
	const uint8_t ctrl2 = (((ctrl & 0xC) << 3) ^ 0x60);

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
	data |= 0x20; // CONT pin (TODO: Set to 0x00 for MD with PBC.)
	data |= (data1 & IO::Device::IOPIN_TH);
	data |= ((data2 & IO::Device::IOPIN_TH) << 1);
	return data;
}

/**
 * Is the SMS Pause button pressed?
 * (Also Game Gear Start button.)
 * @return True if pressed; false if not.
 */
bool IoManager::isPauseSMS(void) const
{
	// TODO: Untested!

	// Check all controllers.
	// TODO: Optimize this so we don't have to check all controllers?
	for (int i = 0; i < ARRAY_SIZE(d->ioDevices); i++) {
		const IO::Device *dev = d->ioDevices[i];
		if (dev->type() == IOT_2BTN) {
			const IO::Io2BTN *dev2BTN = (const IO::Io2BTN*)dev;
			if (dev2BTN->pause) {
				// Pause is pressed.
				return true;
			}
		}
	}

	// Pause is not pressed.
	// TODO: Should be a single-shot...
	return false;
}

/**
 * Read the Game Gear Start button register.
 * @return Game Gear Start button register.
 */
uint8_t IoManager::readStartGG(void) const
{
	// TODO: Untested!

	/**
	 * Start button: Register $00
	 * Bit 7 represents the state, active low. (0 == pressed)
	 *
	 * Reference: http://www.smspower.org/Development/StartButton
	 */

	bool start = isPauseSMS();
	if (start) {
		// Start is pressed.
		return 0x7F;
	}

	// Start is not pressed.
	return 0xFF;
}

/**
 * Update an I/O device.
 * @param virtPort Virtual port.
 * @param buttons New button state.
 */
void IoManager::update(int virtPort, uint32_t buttons)
{
	assert(virtPort >= VIRTPORT_1 && virtPort < VIRTPORT_MAX);
	IO::Device *const dev = d->ioDevices[virtPort];
	if (dev != nullptr) {
		// Update the device.
		dev->update(buttons);
	}
}

/** ZOMG savestate functions. **/

/**
 * Save the I/O state. (MD)
 * @param state Zomg_MD_IoSave_t struct to save to.
 */
void IoManager::zomgSaveMD(Zomg_MD_IoSave_t *state) const
{
	const IO::Device *const dev1 = d->ioDevices[PHYSPORT_1];
	const IO::Device *const dev2 = d->ioDevices[PHYSPORT_2];
	const IO::Device *const devE = d->ioDevices[PHYSPORT_EXT];

	// Physical ports must be allocated.
	assert(dev1 != nullptr);
	assert(dev2 != nullptr);
	assert(devE != nullptr);

	// Port 1
	state->port1_data	= dev1->mdData;
	state->port1_ctrl	= dev1->ctrl;
	state->port1_ser_tx	= dev1->serLastTx;
	state->port1_ser_rx	= 0xFF; // TODO
	state->port1_ser_ctrl	= dev1->serCtrl;

	// Port 2
	state->port2_data	= dev2->mdData;
	state->port2_ctrl	= dev2->ctrl;
	state->port2_ser_tx	= dev2->serLastTx;
	state->port2_ser_rx	= 0xFF; // TODO
	state->port2_ser_ctrl	= dev2->serCtrl;

	// Port 3 (EXT)
	state->port3_data	= devE->mdData;
	state->port3_ctrl	= devE->ctrl;
	state->port3_ser_tx	= devE->serLastTx;
	state->port3_ser_rx	= 0xFF; // TODO
	state->port3_ser_ctrl	= devE->serCtrl;
}

/**
 * Restore the I/O state. (MD)
 * @param state Zomg_MD_IoSave_t struct to restore from.
 */
void IoManager::zomgRestoreMD(const Zomg_MD_IoSave_t *state)
{
	IO::Device *const dev1 = d->ioDevices[PHYSPORT_1];
	IO::Device *const dev2 = d->ioDevices[PHYSPORT_2];
	IO::Device *const devE = d->ioDevices[PHYSPORT_EXT];

	// Physical ports must be allocated.
	assert(dev1 != nullptr);
	assert(dev2 != nullptr);
	assert(devE != nullptr);

	// Port 1
	dev1->mdData	= state->port1_data;
	dev1->ctrl	= state->port1_ctrl;
	dev1->serLastTx	= state->port1_ser_tx;
	//dev1->serLastRx	= state->port1_ser_rx;	// TODO
	dev1->serCtrl	= state->port1_ser_ctrl;

	// Port 2
	dev2->mdData	= state->port2_data;
	dev2->ctrl	= state->port2_ctrl;
	dev2->serLastTx	= state->port2_ser_tx;
	//dev2->serLastRx	= state->port2_ser_rx;	// TODO
	dev2->serCtrl	= state->port2_ser_ctrl;

	// Port 3 (EXT)
	devE->mdData	= state->port3_data;
	devE->ctrl	= state->port3_ctrl;
	devE->serLastTx	= state->port3_ser_tx;
	//devE->serLastRx	= state->port3_ser_rx;	// TODO
	devE->serCtrl	= state->port3_ser_ctrl;

	// Make sure pin58 is set to 2. (Pin 5 = +5V, Pin 8 = GND)
	// NOTE: setPin58() may call update().
	dev1->setPin58(2);
	dev2->setPin58(2);
	devE->setPin58(2);

	// Update the ports.
	dev1->update();
	dev2->update();
	devE->update();
}

}
