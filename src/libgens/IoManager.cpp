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
#include "GensInput/DevManager.hpp"

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

// C++ includes.
#include <algorithm>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

namespace LibGens
{

/**
 * IoManager private class.
 */
class IoManagerPrivate
{
	public:
		IoManagerPrivate(IoManager *q);

	private:
		IoManager *const q;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoManagerPrivate(const IoManagerPrivate &);
		IoManagerPrivate &operator=(const IoManagerPrivate &);

	public:
		/**
		 * Reset all devices.
		 */
		void reset(void);

		// Set/get keymap.
		int setKeymap(int virtPort, const GensKey_t *keymap, int siz);
		int keymap(int virtPort, GensKey_t *keymap, int siz) const;

		/**
		 * Update the scanline counter for all controllers.
		 * This is used by the 6-button controller,
		 * which resets its internal counter after
		 * around 25 scanlines of no TH rising edges.
		 */
		void doScanline(void);
		static const int SCANLINE_COUNT_MAX_6BTN = 25;

		/**
		 * I/O device update function.
		 */
		void update(void);

		/**
		 * Update an I/O device's state based on ctrl/data lines.
		 * @param physPort Physical port number.
		 */
		void updateDevice(int physPort);

		void updateDevice_3BTN(int virtPort);
		void updateDevice_6BTN(int virtPort, bool oldSelect);
		void updateDevice_2BTN(int virtPort);
		
		// I/O pin definitions.
		enum IoPinDefs {
			IOPIN_UP	= 0x01,	// D0
			IOPIN_DOWN	= 0x02,	// D1
			IOPIN_LEFT	= 0x04,	// D2
			IOPIN_RIGHT	= 0x08,	// D3
			IOPIN_TL	= 0x10,	// D4
			IOPIN_TR	= 0x20,	// D5
			IOPIN_TH	= 0x40	// D6
		};

		// Button bitfield values.
		enum ButtonBitfield {
			BTN_UP		= 0x01,
			BTN_DOWN	= 0x02,
			BTN_LEFT	= 0x04,
			BTN_RIGHT	= 0x08,
			BTN_B		= 0x10,
			BTN_C		= 0x20,
			BTN_A		= 0x40,
			BTN_START	= 0x80,
			BTN_Z		= 0x100,
			BTN_Y		= 0x200,
			BTN_X		= 0x400,
			BTN_MODE	= 0x800,

			// SMS/GG buttons.
			BTN_1		= 0x10,
			BTN_2		= 0x20,

			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTN_MOUSE_LEFT		= 0x01,
			BTN_MOUSE_RIGHT		= 0x02,
			BTN_MOUSE_MIDDLE	= 0x04,
			BTN_MOUSE_START		= 0x08	// Start
		};

		// Button index values.
		enum ButtonIndex {
			BTNI_UNKNOWN	= -1,

			// Standard controller buttons.
			BTNI_UP		= 0,
			BTNI_DOWN	= 1,
			BTNI_LEFT	= 2,
			BTNI_RIGHT	= 3,
			BTNI_B		= 4,
			BTNI_C		= 5,
			BTNI_A		= 6,
			BTNI_START	= 7,
			BTNI_Z		= 8,
			BTNI_Y		= 9,
			BTNI_X		= 10,
			BTNI_MODE	= 11,

			// SMS/GG buttons.
			BTNI_1		= 4,
			BTNI_2		= 5,

			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTNI_MOUSE_LEFT		= 0,
			BTNI_MOUSE_RIGHT	= 1,
			BTNI_MOUSE_MIDDLE	= 2,
			BTNI_MOUSE_START	= 3,	// Start

			BTNI_MAX	= 12
		};

		/** Serial I/O definitions and variables. **/

		/**
		 * @name Serial I/O control bitfield.
		 */
		enum SerCtrl {
			 SERCTRL_TFUL	= 0x01,		// TxdFull (1 == full)
			 SERCTRL_RRDY	= 0x02,		// RxdReady (1 == ready)
			 SERCTRL_RERR	= 0x04,		// RxdError (1 == error)
			 SERCTRL_RINT	= 0x08,		// Rxd Interrupt (1 == on)
			 SERCTRL_SOUT	= 0x10,		// TL mode. (1 == serial out; 0 == parallel)
			 SERCTRL_SIN	= 0x20,		// TR mode. (1 == serial in; 0 == parallel)
			 SERCTRL_BPS0	= 0x40,
			 SERCTRL_BPS1	= 0x80
		};

		/**
		 * @name Serial I/O baud rate values.
		 */
		enum SerBaud {
			SERBAUD_4800	= 0x00,
			SERBAUD_2400	= 0x01,
			SERBAUD_1200	= 0x02,
			SERBAUD_300	= 0x03
		};

		struct IoDevice
		{
			IoDevice()
				: type(IoManager::IOT_3BTN)
				, counter(0)
				, scanlines(0)
				, ctrl(0)
				, mdData(0xFF)
				, deviceData(0xFF)
				, select(false)
				, buttons(~0)
				, serCtrl(0)
				, serLastTx(0xFF)
			{ }

			void reset(void) {
				counter = 0;
				scanlines = 0;
				ctrl = 0;
				mdData = 0xFF;
				deviceData = 0xFF;
				select = false;
				buttons = ~0;
				serCtrl = 0;
				serLastTx = 0xFF;
			}

			IoManager::IoType type;	// Device type.
			int counter;			// Internal counter.
			int scanlines;			// Scanline counter.

			uint8_t ctrl;			// Tristate control.
			uint8_t mdData;			// Data written from the MD.
			uint8_t deviceData;		// Device data.
			bool select;			// Select line state.

			/**
			 * Controller bitfield.
			 * Format:
			 * - 2-button:          ??CBRLDU
			 * - 3-button:          SACBRLDU
			 * - 6-button: ????MXYZ SACBRLDU
			 * NOTE: ACTIVE LOW! (1 == released; 0 == pressed)
			 */
			unsigned int buttons;

			/**
			 * Determine the SELECT line state.
			 */
			inline void updateSelectLine(void) {
				// TODO: Apply the device data.
				select = (!(ctrl & IOPIN_TH) ||
					    (mdData & IOPIN_TH));
			}

			inline bool isSelect(void) const
				{ return select; }

			/**
			 * Read the last data value, with tristates applied.
			 * @return Data value with tristate settings applied.
			 */
			inline uint8_t readData(void) const {
				return applyTristate(deviceData);
			}

			/**
			 * Apply the Tristate settings to the data value.
			 * @param data Data value.
			 * @return Data value with tristate settings applied.
			 */
			inline uint8_t applyTristate(uint8_t data) const {
				data &= (~ctrl & 0x7F);		// Mask output bits.
				data |= (mdData & (ctrl | 0x80));	// Apply data buffer.
				return data;
			}

			// Serial I/O variables.
			// TODO: Serial data buffer.
			uint8_t serCtrl;	// Serial control.
			uint8_t serLastTx;	// Last transmitted data byte.

			// Button mapping.
			GensKey_t keyMap[BTNI_MAX];
		};

		IoDevice ioDevices[IoManager::VIRTPORT_MAX];

		/**
		 * Number of buttons in each device type.
		 */
		/*
		enum IoType {
			IOT_NONE	= 0,
			IOT_3BTN	= 1,
			IOT_6BTN	= 2,
			IOT_2BTN	= 3,
			IOT_MEGA_MOUSE	= 4,
			IOT_TEAMPLAYER	= 5,
			IOT_4WP_MASTER	= 6,
			IOT_4WP_SLAVE	= 7,
		*/
		static const uint8_t devBtnCount[IoManager::IOT_MAX];
};

/**
 * Number of buttons in each device type.
 */
const uint8_t IoManagerPrivate::devBtnCount[IoManager::IOT_MAX] =
{
	0,	// IOT_NONE
	8,	// IOT_3BTN
	12,	// IOT_6BTN
	6,	// IOT_2BTN (TODO: Start/Pause?)
	4,	// IOT_MEGA_MOUSE
	0,	// IOT_TEAMPLAYER
	0,	// IOT_4WP_MASTER
	0,	// IOT_4WP_SLAVE
};


IoManagerPrivate::IoManagerPrivate(IoManager *q)
	: q(q)
{
	// Set the default controller types for IOPORT_1 and IOPORT_2.
	ioDevices[IoManager::VIRTPORT_1].type = IoManager::IOT_6BTN;
	ioDevices[IoManager::VIRTPORT_2].type = IoManager::IOT_3BTN;
	
	// Reset all devices.
	reset();
}

/**
 * Reset all devices.
 */
void IoManagerPrivate::reset(void)
{
	for (int i = 0; i < NUM_ELEMENTS(ioDevices); i++)
		ioDevices[i].reset();
}

/**
 * Set the device keymap.
 * @param virtPort Virtual port number.
 * @param keymap Array of GensKey_t values.
 * @param siz Size of keymap array.
 * @return Number of keys set, or negative on error.
 */
int IoManagerPrivate::setKeymap(int virtPort, const GensKey_t *keymap, int siz)
{
	IoDevice *dev = &ioDevices[virtPort];
	const int btns = std::min(siz, NUM_ELEMENTS(dev->keyMap));
	for (int i = 0; i < btns; i++)
		dev->keyMap[i] = *keymap++;
	return btns;
}

/**
 * Get the device keymap.
 * @param virtPort Virtual port number.
 * @param keymap Array to store the GensKey_t values in.
 * @param siz Size of keymap array.
 * @return Number of keys returned, or negative on error.
 */
int IoManagerPrivate::keymap(int virtPort, GensKey_t *keymap, int siz) const
{
	const IoDevice *dev = &ioDevices[virtPort];
	const int btns = std::min(siz, NUM_ELEMENTS(dev->keyMap));
	for (int i = 0; i < btns; i++)
		*keymap++ = dev->keyMap[i];
	return btns;
}

/**
 * Update the scanline counter for all controllers.
 * This is used by the 6-button controller,
 * which resets its internal counter after
 * around 25 scanlines of no TH rising edges.
 */
void IoManagerPrivate::doScanline(void)
{
	for (int i = 0; i < NUM_ELEMENTS(ioDevices); i++) {
		IoDevice *dev = &ioDevices[i];
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

/**
 * I/O device update function.
 */
void IoManagerPrivate::update(void)
{
	// Update keyboard input for all ports.
	for (int i = 0; i < NUM_ELEMENTS(ioDevices); i++) {
		int btnCount = devBtnCount[ioDevices[i].type];
		uint32_t buttons = 0;
		for (int j = btnCount; j >= 0; j--) {
			buttons <<= 1;
			buttons |= DevManager::IsKeyPressed(ioDevices[i].keyMap[j]);
		}

		// Buttons typically use active-low logic.
		ioDevices[i].buttons = ~buttons;
	}

	// Update all physical ports.
	for (int i = IoManager::PHYSPORT_1; i < IoManager::PHYSPORT_MAX; i++)
		updateDevice(i);
}

/**
 * Update an I/O device's state based on ctrl/data lines.
 * @param physPort Physical port number.
 */
void IoManagerPrivate::updateDevice(int physPort)
{
	assert(physPort >= 0 && physPort <= 2);

	IoDevice *dev = &ioDevices[physPort];
	const bool oldSelect = dev->isSelect();
	dev->updateSelectLine();

	// Handle devices that require updating when certain lines change.
	switch (dev->type) {
		case IoManager::IOT_3BTN: updateDevice_3BTN(physPort); break;
		case IoManager::IOT_6BTN: updateDevice_6BTN(physPort, oldSelect); break;
		case IoManager::IOT_2BTN: updateDevice_2BTN(physPort); break;
		
		// TODO: Implement Team Player, 4WP, and Mega Mouse.
		default:
			break;
	}
}

void IoManagerPrivate::updateDevice_3BTN(int virtPort)
{
	/**
	 * Data formats: (D == last written MSB)
	 * TH=1: D1CBRLDU
	 * TH=0: D0SA00DU
	 */
	
	IoDevice *dev = &ioDevices[virtPort];
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

void IoManagerPrivate::updateDevice_6BTN(int virtPort, bool oldSelect)
{
	IoDevice *dev = &ioDevices[virtPort];
	uint8_t data;

	if (!oldSelect && dev->isSelect()) {
		// IOPIN_TH rising edge.
		// Increment the counter.
		dev->counter = ((dev->counter + 2) & 0x06);
		
		// Reset the scanline counter.
		dev->scanlines = 0;
	}
	
	// Use the TH counter to determine the controller state.
	// TODO: There should be a 2-NOP delay between TH change and reaction...
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

void IoManagerPrivate::updateDevice_2BTN(int virtPort)
{
	/**
	 * Data format: (x == tristate value)
	 * - xxCBRLDU
	 * B == button 1
	 * C == button 2
	 */
	IoDevice *dev = &ioDevices[virtPort];
	dev->mdData = (0xC0 | (ioDevices[virtPort].buttons & 0x3F));
}


/** IoManager **/


IoManager::IoManager()
	: d(new IoManagerPrivate(this))
{ }


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

/**
 * Set the device keymap.
 * @param virtPort Virtual port number.
 * @param keymap Array of GensKey_t values.
 * @param siz Size of keymap array.
 * @return Number of keys set, or negative on error.
 */
int IoManager::setKeymap(int virtPort, const GensKey_t *keymap, int siz)
{
	return d->setKeymap(virtPort, keymap, siz);
}

/**
 * Get the device keymap.
 * @param virtPort Virtual port number.
 * @param keymap Array to store the GensKey_t values in.
 * @param siz Size of keymap array.
 * @return Number of keys returned, or negative on error.
 */
int IoManager::keymap(int virtPort, GensKey_t *keymap, int siz) const
{
	return d->keymap(virtPort, keymap, siz);
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


/**
 * I/O device update function.
 */
void IoManager::update(void)
{
	d->update();
}

}
