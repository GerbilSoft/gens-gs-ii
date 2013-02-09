/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.hpp: I/O manager. (Private Class)                             *
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

#ifndef __LIBGENS_IO_IOMANAGER_P_HPP__
#define __LIBGENS_IO_IOMANAGER_P_HPP__

// Private class.
// Only allow inclusion in IoManager.
#if !defined(__LIBGENS_IN_IOMANAGER_CLASS__)
#error IoManager_p.hpp is NOT a public header. Do NOT include it in your code!
#endif

#include "IoManager.hpp"

// C includes. (C++ namespace)
#include <cstring>

// ARRAY_SIZE(x)
#include "../macros/common.h"

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
		void updateDevice_Mouse(int virtPort, bool oldSelect, bool oldTr);
		void updateDevice_TP(int physPort, bool oldSelect, bool oldTr);
		void updateDevice_4WP_Master(int physPort);
		void updateDevice_4WP_Slave(int physPort);

		/**
		 * Latch relX, relY, and signOver for a Sega Mega Mouse.
		 * @param virtPort Virtual port number.
		 */
		void latchMegaMouse(int virtPort);

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
			BTN_C		= 0x10,
			BTN_B		= 0x20,
			BTN_START	= 0x40,
			BTN_A		= 0x80,
			BTN_Z		= 0x100,
			BTN_Y		= 0x200,
			BTN_X		= 0x400,
			BTN_MODE	= 0x800,

			// SMS/GG buttons.
			BTN_2		= 0x10,
			BTN_1		= 0x20,

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
		 * Team Player.
		 */

		/**
		 * Rebuild the controller index table.
		 * @param physPort Physical controller port. (0 or 1)
		 */
		void rebuildCtrlIndexTable(int physPort);

		/**
		 * @name TP_PadType
		 * Indicates the type of controller connected to
		 * a given port on the Sega Team Player adapter.
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
				: type(IoManager::IOT_NONE)
			{ reset(); }

			/**
			 * Reset everything in the IoDevice.
			 * WARNING: Do NOT use this while emulation is running!
			 */
			void reset(void) {
				ctrl = 0;
				mdData = 0xFF;
				serCtrl = 0;
				serLastTx = 0xFF;
				resetDev();
			}

			/**
			 * Reset IoDevice data that only affects the device
			 * and not the emulation-side registers.
			 */
			void resetDev(void) {
				counter = 0;
				scanlines = 0;
				buttons = ~0;
				deviceData = 0xFF;
				updateSelectLine();
				updateTrLine();

				// Clear device-specific data.
				memset(&data, 0x00, sizeof(data));
			}

			IoManager::IoType_t type;	// Device type.

			// Device-side variables.
			int counter;			// Internal counter.
			int scanlines;			// Scanline counter.
			uint8_t deviceData;		// Data written from the device.

			// System-side variables.
			uint8_t ctrl;			// Tristate control.
			uint8_t mdData;			// Data written from the MD.

			// Cache variables.
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
			 * Device-specific data.
			 */
			union {
				struct {
					// Team Player data.
					uint8_t padTypes[4];
					uint8_t ctrlIndexTbl[TP_DT_MAX - TP_DT_PADA_RLDU];
				} tp;
				struct {
					// Mouse data.
					// TODO: Make use of this - currently, only buttons are supported.
					int relX;
					int relY;
					struct {
						uint8_t signOver;
						uint8_t relX;
						uint8_t relY;
					} latch;
				} mouse;
			} data;
		};

		IoDevice ioDevices[IoManager::VIRTPORT_MAX];

		/**
		 * Device information.
		 */
		struct IoDevInfo {
			uint32_t fourCC;	// FourCC.
			uint8_t btnCount;	// Number of buttons.

			// Is this device type usable?
			// If false, disabled in Release builds.
			bool isUsable;
			// TODO: UPDATE function pointer?
		};

		static const IoDevInfo ioDevInfo[IoManager::IOT_MAX];

		/**
		 * 4-Way Play: Current player.
		 */
		int ea4wp_curPlayer;
};

}

#endif /* __LIBGENS_IO_IOMANAGER_P_HPP__ */
