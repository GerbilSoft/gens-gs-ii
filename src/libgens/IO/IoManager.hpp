/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.hpp: I/O manager.                                             *
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

#ifndef __LIBGENS_IO_IOMANAGER_HPP__
#define __LIBGENS_IO_IOMANAGER_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// ZOMG savestate structs.
#include "libzomg/zomg_md_io.h"

namespace LibGens
{

class IoManagerPrivate;

class IoManager
{
	public:
		IoManager();
		~IoManager();

	private:
		friend class IoManagerPrivate;
		IoManagerPrivate *const d;
		
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoManager(const IoManager &);
		IoManager &operator=(const IoManager &);

	public:
		/**
		 * Reset all devices.
		 */
		void reset(void);

		/**
		 * @name Physical port numbers.
		 */
		enum PhysPort {
			PHYSPORT_1	= 0,	// Player 1
			PHYSPORT_2	= 1,	// Player 2
			PHYSPORT_EXT	= 2,	// Extension port

			PHYSPORT_MAX
		};

		/**
		 * MD I/O functions.
		 * NOTE: port must be either 0, 1, or 2.
		 * These correspond to physical ports.
		 */
		uint8_t readDataMD(int physPort) const;
		void writeDataMD(int physPort, uint8_t data);

		uint8_t readCtrlMD(int physPort) const;
		void writeCtrlMD(int physPort, uint8_t ctrl);

		uint8_t readSerCtrl(int physPort) const;
		void writeSerCtrl(int physPort, uint8_t serCtrl);

		uint8_t readSerTx(int physPort) const;
		void writeSerTx(int physPort, uint8_t serTx);

		uint8_t readSerRx(int physPort) const;

		/**
		 * SMS I/O functions.
		 * These are mapped to MD registers internally.
		 */
		void writeCtrlSMS(uint8_t ctrl);
		uint8_t readDataSMS_DC(void) const;
		uint8_t readDataSMS_DD(void) const;
		bool isPauseSMS(void) const;
		uint8_t readStartGG(void) const;

		/**
		 * Update an I/O device.
		 * @param virtPort Virtual port.
		 * @param buttons New button state.
		 */
		void update(int virtPort, uint32_t buttons);

		/**
		 * Update the scanline counter for all controllers.
		 * This is used by the 6-button controller,
		 * which resets its internal counter after
		 * around 25 scanlines of no TH rising edges.
		 */
		void doScanline(void);

		/**
		 * @name Virtual port numbers.
		 */
		enum VirtPort_t {
			VIRTPORT_1	= 0,	// Player 1
			VIRTPORT_2	= 1,	// Player 2
			VIRTPORT_EXT	= 2,	// Extension port

			// Team Player 1
			VIRTPORT_TP1A	= 3,	// TP-1A
			VIRTPORT_TP1B	= 4,	// TP-1B
			VIRTPORT_TP1C	= 5,	// TP-1C
			VIRTPORT_TP1D	= 6,	// TP-1D

			// Team Player 2
			VIRTPORT_TP2A	= 7,	// TP-1A
			VIRTPORT_TP2B	= 8,	// TP-1B
			VIRTPORT_TP2C	= 9,	// TP-1C
			VIRTPORT_TP2D	= 10,	// TP-1D

			// 4-Way Play
			VIRTPORT_4WPA	= 11,	// 4WP-A
			VIRTPORT_4WPB	= 12,	// 4WP-B
			VIRTPORT_4WPC	= 13,	// 4WP-C
			VIRTPORT_4WPD	= 14,	// 4WP-D

			// J-Cart (TODO)
			VIRTPORT_JCART1	= 15,
			VIRTPORT_JCART2	= 16,

			VIRTPORT_MAX
		};

		enum IoType_t {
			IOT_NONE	= 0,
			IOT_3BTN,
			IOT_6BTN,
			IOT_2BTN,

			// Miscellaneous Master System peripherals.
			IOT_PADDLE,
			IOT_SPORTS_PAD,

			// Miscellaneous Mega Drive peripherals.
			IOT_MEGA_MOUSE,
			IOT_XE_1AP,
			IOT_ACTIVATOR,

			// Light guns.
			IOT_PHASER,
			IOT_MENACER,
			IOT_JUSTIFIER,

			// Multitaps.
			IOT_TEAMPLAYER,
			IOT_4WP_MASTER,
			IOT_4WP_SLAVE,

			IOT_MAX
		};

		// Controller configuration.

		/** General device type functions. **/

		/**
		 * Get the number of buttons present on a specific type of device.
		 * @param ioType Device type.
		 * @return Number of buttons.
		 */
		static int NumDevButtons(IoType_t ioType);

		/**
		 * Is a given device type usable?
		 * @param ioType Device type.
		 * @return True if usable; false if not.
		 */
		static bool IsDevTypeUsable(IoType_t ioType);

		/**
		 * Get the FourCC for a given device type.
		 * @param ioType Device type.
		 * @return FourCC for the device type.
		 */
		static uint32_t IoTypeToFourCC(IoType_t ioType);

		/**
		 * Get the device type for a given FourCC.
		 * @param ioType Device type.
		 * @return Device type, or IOT_MAX if the FourCC doesn't match any device type.
		 */
		static IoManager::IoType_t FourCCToIoType(uint32_t fourCC);

		/**
		 * Convert a string to a FourCC.
		 * @param str String.
		 * @return FourCC, or 0 if the string was not four characters long.
		 */
		static uint32_t StringToFourCC(const std::string& str);

		/**
		 * Convert a FourCC to a string.
		 * @param fourCC FourCC.
		 * @return FourCC as a string.
		 */
		static std::string FourCCToString(uint32_t fourCC);

		/**
		 * Get the FourCC as a string for a given device type.
		 * @param ioType Device type.
		 * @return FourCC as a string.
		 */
		static std::string IoTypeToString(IoType_t ioType);

		/**
		 * Get the device type for a given FourCC string.
		 * @param str String.
		 * @return FourCC, or IOT_MAX if the FourCC doesn't match any device type.
		 */
		static IoManager::IoType_t StringToIoType(const std::string& str);

		/** Get/set device types. **/

		/**
		 * Get the device type for a given virtual port.
		 * @param virtPort Virtual port.
		 */
		IoType_t devType(VirtPort_t virtPort) const;

		/**
		 * Set the device type for a given virtual port.
		 * @param virtPort Virtual port.
		 * @param ioType New device type.
		 */
		void setDevType(VirtPort_t virtPort, IoType_t ioType);

		/**
		 * Logical button names.
		 * These are used for button name trnaslation in the UI.
		 */
		enum ButtonName_t {
			BTNNAME_UNKNOWN	= -1,

			// Standard controller buttons.
			BTNNAME_UP,
			BTNNAME_DOWN,
			BTNNAME_LEFT,
			BTNNAME_RIGHT,
			BTNNAME_B,
			BTNNAME_C,
			BTNNAME_A,
			BTNNAME_START,
			BTNNAME_Z,
			BTNNAME_Y,
			BTNNAME_X,
			BTNNAME_MODE,

			// SMS/GG buttons.
			BTNNAME_1,
			BTNNAME_2,
			BTNNAME_STARTPAUSE,

			// Sega Mega Mouse buttons.
			BTNNAME_MOUSE_LEFT,
			BTNNAME_MOUSE_RIGHT,
			BTNNAME_MOUSE_MIDDLE,
			BTNNAME_MOUSE_START,

			// XE-1 AP buttons.
			BTNNAME_SELECT,
			BTNNAME_E2,
			BTNNAME_E1,
			BTNNAME_D,

			BTNNAME_MAX
		};

		// Button index values.
		enum ButtonIndex_t {
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
			BTNI_STARTPAUSE	= 6,

			// SMS: Paddle controller.
			BTNI_PADDLE_1	= 0,
			BTNI_PADDLE_2	= 1,

			// SMS: Sports pad.
			BTNI_SPAD_1	= 0,
			BTNI_SPAD_2	= 1,

			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTNI_MOUSE_LEFT		= 0,
			BTNI_MOUSE_RIGHT	= 1,
			BTNI_MOUSE_MIDDLE	= 2,
			BTNI_MOUSE_START	= 3,	// Start

			// XE-1AP
			BTNI_XE1AP_SELECT	= 0,
			BTNI_XE1AP_START	= 1,
			BTNI_XE1AP_E2		= 2,
			BTNI_XE1AP_E1		= 3,
			BTNI_XE1AP_D		= 4,
			BTNI_XE1AP_C		= 5,
			BTNI_XE1AP_B		= 6,
			BTNI_XE1AP_A		= 7,

			BTNI_MAX	= 12
		};

		// Get button names.
		static ButtonName_t ButtonName(IoType_t ioType, int btnIdx);
		static int FirstLogicalButton(IoType_t ioType);
		static int NextLogicalButton(IoType_t ioType, int btnIdx);

		/** ZOMG savestate functions. **/
		void zomgSaveMD(Zomg_MD_IoSave_t *state) const;
		void zomgRestoreMD(const Zomg_MD_IoSave_t *state);
};

/**
 * Get the FourCC as a string for a given device type.
 * @param ioType Device type.
 * @return FourCC as a string.
 */
inline std::string IoManager::IoTypeToString(IoType_t ioType)
	{ return FourCCToString(IoTypeToFourCC(ioType)); }

/**
 * Get the device type for a given FourCC string.
 * @param str String.
 * @return FourCC, or IOT_MAX if the FourCC doesn't match any device type.
 */
inline IoManager::IoType_t IoManager::StringToIoType(const std::string& str)
	{ return FourCCToIoType(StringToFourCC(str)); }

}

#endif /* __LIBGENS_IOMANAGER_HPP__ */
