/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.hpp: I/O manager. (Private Class)                             *
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

#ifndef __LIBGENS_IO_IOMANAGER_P_HPP__
#define __LIBGENS_IO_IOMANAGER_P_HPP__

// Private class.
// Only allow inclusion in IoManager.
#if !defined(__LIBGENS_IN_IOMANAGER_CLASS__)
#error IoManager_p.hpp is NOT a public header. Do NOT include it in your code!
#endif

#include "IoManager.hpp"
#include "Device.hpp"

// C includes. (C++ namespace)
#include <cstring>

// ARRAY_SIZE(x)
#include "../macros/common.h"

namespace LibGens {

/**
 * IoManager private class.
 */
class IoManagerPrivate
{
	public:
		IoManagerPrivate(IoManager *q);
		~IoManagerPrivate();

	private:
		IoManager *const q;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoManagerPrivate(const IoManagerPrivate &);
		IoManagerPrivate &operator=(const IoManagerPrivate &);

	public:
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
		 * I/O devices.
		 * NOTE: Ports 1, 2, and EXT must have valid
		 * devices. If those ports are empty, they should
		 * have a generic Device object. Other ports may
		 * be nullptr for empty ports.
		 */
		IO::Device *ioDevices[IoManager::VIRTPORT_MAX];

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
		 * Sega Pico page register.
		 * Valid page values:
		 * [00, 01, 03, 07, 0F, 1F, 3F]
		 * - 00 = book closed
		 * - Opening a page causes another sensor to be uncovered.
		 * - 6 pages is the maximum.
		 */
		uint8_t pico_page;
};

}

#endif /* __LIBGENS_IO_IOMANAGER_P_HPP__ */
