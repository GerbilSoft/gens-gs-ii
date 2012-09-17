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
		 * @param virtPort Virtual port number.
		 */
		void updateDevice(int virtPort);

		void updateDevice_3BTN(int virtPort);
		void updateDevice_6BTN(int virtPort, bool oldSelect);
		void updateDevice_2BTN(int virtPort);
		void updateDevice_TP(int physPort, bool oldSelect, bool oldTr);
		void updateDevice_4WP_Master(int physPort);
		void updateDevice_4WP_Slave(int physPort);

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

		/**
		 * Teamplayer.
		 */

		/**
		 * Rebuild the controller index table.
		 * @param physPort Physical controller port. (0 or 1)
		 */
		void rebuildCtrlIndexTable(int physPort);

		/**
		 * @name TP_PadType
		 * Indicates the type of controller connected to
		 * a given port on the Sega Teamplayer adapter.
		 */
		enum TP_PadType {
			TP_PT_NONE	= 0xF,
			TP_PT_3BTN	= 0x0,
			TP_PT_6BTN	= 0x1
		};

		/**
		 * @name TP_DataType
		 * Indicates the type of data that should be returned
		 * in the current cycle.
		 */
		enum TP_DataType {
			// Initialization
			TP_DT_INIT		= 0,	// 0: Initial state.
			TP_DT_START		= 1,	// 1: Start request.
			TP_DT_ACK1		= 2,	// 2: Acknowledgement.
			TP_DT_ACK2		= 3,	// 3: Acknowledgement.
			TP_DT_PADTYPE_A		= 4,	// 4: Pad type for Controller A
			TP_DT_PADTYPE_B		= 5,	// 5: Pad type for Controller A
			TP_DT_PADTYPE_C		= 6,	// 6: Pad type for Controller A
			TP_DT_PADTYPE_D		= 7,	// 7: Pad type for Controller A

			// Controller A
			TP_DT_PADA_RLDU		= 8,	// D-pad
			TP_DT_PADA_SACB		= 9,	// Start, A, C, B
			TP_DT_PADA_MXYZ		= 10,	// Mode, X, Y, Z (6-button only)

			// Controller B
			TP_DT_PADB_RLDU		= 11,	// D-pad
			TP_DT_PADB_SACB		= 12,	// Start, A, C, B
			TP_DT_PADB_MXYZ		= 13,	// Mode, X, Y, Z (6-button only)

			// Controller C
			TP_DT_PADC_RLDU		= 14,	// D-pad
			TP_DT_PADC_SACB		= 15,	// Start, A, C, B
			TP_DT_PADC_MXYZ		= 16,	// Mode, X, Y, Z (6-button only)

			// Controller D
			TP_DT_PADD_RLDU		= 17,	// D-pad
			TP_DT_PADD_SACB		= 18,	// Start, A, C, B
			TP_DT_PADD_MXYZ		= 19,	// Mode, X, Y, Z (6-button only)

			TP_DT_MAX
		};

		struct IoDevice
		{
			IoDevice()
				: type(IoManager::IOT_3BTN)
			{
				reset();
			}

			void reset(void) {
				counter = 0;
				scanlines = 0;
				ctrl = 0;
				mdData = 0xFF;
				deviceData = 0xFF;
				th_line = false;	// SELECT
				tr_line = false;	// TR
				buttons = ~0;
				serCtrl = 0;
				serLastTx = 0xFF;
			}

			IoManager::IoType_t type;	// Device type.
			int counter;			// Internal counter.
			int scanlines;			// Scanline counter.

			uint8_t ctrl;			// Tristate control.
			uint8_t mdData;			// Data written from the MD.
			uint8_t deviceData;		// Device data.
			bool th_line;			// TH line state. (SELECT)
			bool tr_line;			// TR line state.

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
				th_line = (!(ctrl & IOPIN_TH) ||
					    (mdData & IOPIN_TH));
			}

			inline bool isSelect(void) const
				{ return th_line; }

			/**
			 * Determine the TR line state.
			 */
			inline void updateTrLine(void) {
				// TODO: Apply the device data.
				tr_line = (!(ctrl & IOPIN_TR) ||
					    (mdData & IOPIN_TR));
			}

			inline bool isTrLine(void) const
				{ return tr_line; }

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
			// TODO: Use next-highest power-of-two?
			GensKey_t keyMap[IoManager::BTNI_MAX];

			/**
			 * Teamplayer data.
			 */
			uint8_t tp_padTypes[4];
			uint8_t tp_ctrlIndexTbl[TP_DT_MAX - TP_DT_PADA_RLDU];
		};

		IoDevice ioDevices[IoManager::VIRTPORT_MAX];

		/**
		 * Number of buttons in each device type.
		 */
		static const uint8_t devBtnCount[IoManager::IOT_MAX];

		/**
		 * 4-Way Play: Current player.
		 */
		int ea4wp_curPlayer;
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

	// Rebuild Teamplayer controller index tables for TP devices.
	for (int i = IoManager::VIRTPORT_1;
	     i <= IoManager::VIRTPORT_2; i++) {
		if (ioDevices[i].type == IoManager::IOT_TEAMPLAYER)
			rebuildCtrlIndexTable(i);
	}

	// EA 4-Way Play.
	ea4wp_curPlayer = 7;
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
	IoDevice *const dev = &ioDevices[virtPort];
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
	const IoDevice *const dev = &ioDevices[virtPort];
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

/**
 * I/O device update function.
 */
void IoManagerPrivate::update(void)
{
	// Update keyboard input for all ports.
	for (int i = 0; i < NUM_ELEMENTS(ioDevices); i++) {
		int btnCount = devBtnCount[ioDevices[i].type];
		uint32_t buttons = 0;
		for (int j = (btnCount - 1); j >= 0; j--) {
			buttons <<= 1;
			buttons |= DevManager::IsKeyPressed(ioDevices[i].keyMap[j]);
		}

		// Buttons typically use active-low logic.
		ioDevices[i].buttons = ~buttons;
	}

	// Update all ports.
	for (int i = IoManager::PHYSPORT_1; i < IoManager::PHYSPORT_MAX; i++)
		updateDevice(i);
}

/**
 * Update an I/O device's state based on ctrl/data lines.
 * @param virtPort Virtual port number.
 */
void IoManagerPrivate::updateDevice(int virtPort)
{
	assert(virtPort >= IoManager::VIRTPORT_1 && virtPort < IoManager::VIRTPORT_MAX);

	IoDevice *const dev = &ioDevices[virtPort];
	const bool oldSelect = dev->isSelect();
	dev->updateSelectLine();

	// Handle devices that require updating when certain lines change.
	switch (dev->type) {
		case IoManager::IOT_3BTN: updateDevice_3BTN(virtPort); break;
		case IoManager::IOT_6BTN: updateDevice_6BTN(virtPort, oldSelect); break;
		case IoManager::IOT_2BTN: updateDevice_2BTN(virtPort); break;
		case IoManager::IOT_4WP_MASTER: updateDevice_4WP_Master(virtPort); break;
		case IoManager::IOT_4WP_SLAVE: updateDevice_4WP_Slave(virtPort); break;

		case IoManager::IOT_TEAMPLAYER: {
			const bool oldTr = dev->isTrLine();
			dev->updateTrLine();
			updateDevice_TP(virtPort, oldSelect, oldTr);
			break;
		}

		// TODO: Implement 4WP and Mega Mouse.
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
	
	IoDevice *const dev = &ioDevices[virtPort];
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
	IoDevice *const dev = &ioDevices[virtPort];
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
	IoDevice *const dev = &ioDevices[virtPort];
	dev->deviceData = (0xC0 | (ioDevices[virtPort].buttons & 0x3F));
}

/**
 * Update a Teamplayer device.
 * @param physPort Physical controller port.
 * @param oldSelect Previous TH (SELECT) line state.
 * @param oldTr Previous TR line state.
 */
void IoManagerPrivate::updateDevice_TP(int physPort, bool oldSelect, bool oldTr)
{
	assert(physPort >= IoManager::PHYSPORT_1 && physPort <= IoManager::PHYSPORT_2);

	IoDevice *const dev = &ioDevices[physPort];

	// Check if either TH or TR has changed.
	if ((dev->isSelect() != oldSelect) ||
	    (dev->isTrLine() != oldTr)) {
		// Check if TH is high.
		if (dev->isSelect()) {
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
	switch (dev->counter)
	{
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
			data = dev->tp_padTypes[dev->counter - TP_DT_PADTYPE_A];
			break;

		default:
			// Check the controller data index table.
			int adj_counter = (dev->counter - TP_DT_PADA_RLDU);
			if ((adj_counter >= (int)(NUM_ELEMENTS(dev->tp_ctrlIndexTbl))) ||
			    (dev->tp_ctrlIndexTbl[adj_counter] >= TP_DT_MAX))
			{
				// Invalid counter state.
				// TODO: What value should be returned?
				data = 0x0F;
				break;
			}

			// Determine the virtual port base.
			const int virtPortBase = (physPort == 0
						? IoManager::VIRTPORT_TP1A
						: IoManager::VIRTPORT_TP2A);

			// Controller data.
			const int virtPort = virtPortBase + (adj_counter / 3);
			const int shift = (adj_counter % 3) * 4;

			data = (ioDevices[virtPort].buttons >> shift) & 0xF;
			break;
	}

	// TL should match TR.
	// (from Genesis Plus GX)
	// NOTE: TR is always an MD output line.
	if (dev->isTrLine())
		data |= IOPIN_TL;
	else
		data &= ~IOPIN_TL;

	dev->deviceData = data;
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
	ea4wp_curPlayer = (dev->applyTristate(0xFF) >> 4) & 0x07;

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


/** IoManagerPrivate::IoDevice **/


/**
 * Rebuild the controller index table.
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

	int i = 0;	// tp_ctrlIndexTbl index
	for (int pad = 0; pad < 4; pad++) {
		const int dtBase = (TP_DT_PADA_RLDU + (pad * 3));
		const int virtPort = virtPortBase + pad;

		switch (ioDevices[virtPort].type) {
			case IoManager::IOT_NONE:
			default:
				dev->tp_padTypes[pad] = TP_PT_NONE;
				break;

			case IoManager::IOT_3BTN:
				dev->tp_padTypes[pad] = TP_PT_3BTN;
				dev->tp_ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 0);
				dev->tp_ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 1);
				break;

			case IoManager::IOT_6BTN:
				dev->tp_padTypes[pad] = TP_PT_6BTN;
				dev->tp_ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 0);
				dev->tp_ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 1);
				dev->tp_ctrlIndexTbl[i++] = (TP_DataType)(dtBase + 2);
				break;
		}
	}

	// Set the rest of the controller data indexes to DT_MAX.
	for (int x = i; x < NUM_ELEMENTS(dev->tp_ctrlIndexTbl); x++)
		dev->tp_ctrlIndexTbl[x] = TP_DT_MAX;
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

/**
 * Get the number of buttons present on a specific type of device.
 * @param ioType Device type.
 * @return Number of buttons.
 */
int IoManager::NumDevButtons(IoType_t ioType)
{
	if (ioType < 0 || ioType >= IOT_MAX)
		return 0;

	return IoManagerPrivate::devBtnCount[ioType];
}

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

	IoManagerPrivate::IoDevice *const dev = &d->ioDevices[virtPort];
	dev->type = ioType;
	dev->reset();

	// Rebuild Teamplayer controller index tables for TP devices.
	if (virtPort >= VIRTPORT_1 && virtPort <= VIRTPORT_2) {
		if (dev->type == IoManager::IOT_TEAMPLAYER)
			d->rebuildCtrlIndexTable(virtPort);
	} else if (virtPort >= VIRTPORT_TP1A && virtPort <= VIRTPORT_TP1D) {
		if (d->ioDevices[VIRTPORT_1].type == IoManager::IOT_TEAMPLAYER)
			d->rebuildCtrlIndexTable(VIRTPORT_1);
	} else if (virtPort >= VIRTPORT_TP2A && virtPort <= VIRTPORT_TP2D) {
		if (d->ioDevices[VIRTPORT_2].type == IoManager::IOT_TEAMPLAYER)
			d->rebuildCtrlIndexTable(VIRTPORT_2);
	}
}

IoManager::ButtonName_t IoManager::ButtonName(IoType_t ioType, int btnIdx)
{
	assert(ioType >= IOT_3BTN && ioType < IOT_MAX);
	assert(btnIdx >= 0 && btnIdx < BTNI_MAX);

	switch (ioType) {
		case IOT_3BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNNAME_UP;
				case BTNI_DOWN:		return BTNNAME_DOWN;
				case BTNI_LEFT:		return BTNNAME_LEFT;
				case BTNI_RIGHT:	return BTNNAME_RIGHT;
				case BTNI_B:		return BTNNAME_B;
				case BTNI_C:		return BTNNAME_C;
				case BTNI_A:		return BTNNAME_A;
				case BTNI_START:	return BTNNAME_START;
				default:		return BTNNAME_UNKNOWN;
			}
			break;

		case IOT_6BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNNAME_UP;
				case BTNI_DOWN:		return BTNNAME_DOWN;
				case BTNI_LEFT:		return BTNNAME_LEFT;
				case BTNI_RIGHT:	return BTNNAME_RIGHT;
				case BTNI_B:		return BTNNAME_B;
				case BTNI_C:		return BTNNAME_C;
				case BTNI_A:		return BTNNAME_A;
				case BTNI_START:	return BTNNAME_START;
				case BTNI_Z:		return BTNNAME_Z;
				case BTNI_Y:		return BTNNAME_Y;
				case BTNI_X:		return BTNNAME_X;
				case BTNI_MODE:		return BTNNAME_MODE;
				default:		return BTNNAME_UNKNOWN;
			}
			break;

		case IOT_2BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNNAME_UP;
				case BTNI_DOWN:		return BTNNAME_DOWN;
				case BTNI_LEFT:		return BTNNAME_LEFT;
				case BTNI_RIGHT:	return BTNNAME_RIGHT;
				case BTNI_1:		return BTNNAME_1;
				case BTNI_2:		return BTNNAME_2;
				default:		return BTNNAME_UNKNOWN;
			}
			break;

		case IOT_MEGA_MOUSE:
			switch (btnIdx) {
				case BTNI_MOUSE_LEFT:		return BTNNAME_MOUSE_LEFT;
				case BTNI_MOUSE_RIGHT:		return BTNNAME_MOUSE_RIGHT;
				case BTNI_MOUSE_MIDDLE:	return BTNNAME_MOUSE_MIDDLE;
				case BTNI_MOUSE_START:		return BTNNAME_MOUSE_START;
				default:			return BTNNAME_UNKNOWN;
			}
			break;

		default:
			break;
	}

	return BTNNAME_UNKNOWN;
}

int IoManager::NextLogicalButton(IoType_t ioType, int btnIdx)
{
	assert(ioType >= IOT_3BTN && ioType < IOT_MAX);
	assert(btnIdx >= 0 && btnIdx < BTNI_MAX);

	switch (ioType) {
		case IOT_3BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNI_DOWN;
				case BTNI_DOWN:		return BTNI_LEFT;
				case BTNI_LEFT:		return BTNI_RIGHT;
				case BTNI_RIGHT:	return BTNI_START;
				case BTNI_START:	return BTNI_A;
				case BTNI_A:		return BTNI_B;
				case BTNI_B:		return BTNI_C;
				case BTNI_C:
				default:		return BTNI_UNKNOWN;
			}
			break;

		case IOT_6BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNI_DOWN;
				case BTNI_DOWN:		return BTNI_LEFT;
				case BTNI_LEFT:		return BTNI_RIGHT;
				case BTNI_RIGHT:	return BTNI_START;
				case BTNI_START:	return BTNI_A;
				case BTNI_A:		return BTNI_B;
				case BTNI_B:		return BTNI_C;
				case BTNI_C:		return BTNI_MODE;
				case BTNI_MODE:		return BTNI_X;
				case BTNI_X:		return BTNI_Y;
				case BTNI_Y:		return BTNI_Z;
				case BTNI_Z:
				default:		return BTNI_UNKNOWN;
			}
			break;

		case IOT_2BTN:
			switch (btnIdx) {
				case BTNI_UP:		return BTNI_DOWN;
				case BTNI_DOWN:		return BTNI_LEFT;
				case BTNI_LEFT:		return BTNI_RIGHT;
				case BTNI_RIGHT:	return BTNI_1;
				case BTNI_1:		return BTNI_2;
				case BTNI_2:
				default:		return BTNI_UNKNOWN;
			}
			break;

		case IOT_MEGA_MOUSE:
			switch (btnIdx) {
				case BTNI_MOUSE_LEFT:		return BTNI_MOUSE_MIDDLE;
				case BTNI_MOUSE_MIDDLE:	return BTNI_MOUSE_RIGHT;
				case BTNI_MOUSE_RIGHT:		return BTNI_MOUSE_START;
				case BTNI_MOUSE_START:
				default:			return BTNI_UNKNOWN;
			}
			break;

		default:
			break;
	}

	return BTNI_UNKNOWN;
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


/**
 * I/O device update function.
 */
void IoManager::update(void)
{
	d->update();
}

}
