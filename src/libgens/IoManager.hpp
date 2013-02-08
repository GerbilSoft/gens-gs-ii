/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.hpp: I/O manager.                                             *
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

#ifndef __LIBGENS_IOMANAGER_HPP__
#define __LIBGENS_IOMANAGER_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

#include "GensInput/GensKey_t.h"

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
		 * I/O device update function.
		 */
		void update(void);

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
			IOT_3BTN	= 1,
			IOT_6BTN	= 2,
			IOT_2BTN	= 3,
			IOT_MEGA_MOUSE	= 4,
			IOT_TEAMPLAYER	= 5,
			IOT_4WP_MASTER	= 6,
			IOT_4WP_SLAVE	= 7,

			IOT_MAX
		};

		// Controller configuration.

		// Set/get keymap.
		int setKeymap(int virtPort, const GensKey_t *keymap, int siz);
		int keymap(int virtPort, GensKey_t *keymap, int siz) const;

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
		 * @return Device type, or IOT_MAX if the FourCC is invalid.
		 */
		static uint32_t FourCCToIoType(uint32_t fourCC);

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
		static std::string FourCCToSTring(uint32_t fourCC);

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
		enum ButtonName_t
		{
			BTNNAME_UNKNOWN	= -1,
			
			// Standard controller buttons.
			BTNNAME_UP	= 0,
			BTNNAME_DOWN	= 1,
			BTNNAME_LEFT	= 2,
			BTNNAME_RIGHT	= 3,
			BTNNAME_C	= 4,
			BTNNAME_B	= 5,
			BTNNAME_START	= 6,
			BTNNAME_A	= 7,
			BTNNAME_Z	= 8,
			BTNNAME_Y	= 9,
			BTNNAME_X	= 10,
			BTNNAME_MODE	= 11,
			
			// SMS/GG buttons.
			BTNNAME_2	= 12,
			BTNNAME_1	= 13,
			
			// Sega Mega Mouse buttons.
			BTNNAME_MOUSE_LEFT	= 14,
			BTNNAME_MOUSE_RIGHT	= 15,
			BTNNAME_MOUSE_MIDDLE	= 16,
			BTNNAME_MOUSE_START	= 17,
			
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
			BTNI_C		= 4,
			BTNI_B		= 5,
			BTNI_START	= 6,
			BTNI_A		= 7,
			BTNI_Z		= 8,
			BTNI_Y		= 9,
			BTNI_X		= 10,
			BTNI_MODE	= 11,

			// SMS/GG buttons.
			BTNI_2		= 4,
			BTNI_1		= 5,

			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTNI_MOUSE_LEFT		= 0,
			BTNI_MOUSE_RIGHT	= 1,
			BTNI_MOUSE_MIDDLE	= 2,
			BTNI_MOUSE_START	= 3,	// Start

			BTNI_MAX	= 12
		};

		// Get button names.
		static ButtonName_t ButtonName(IoType_t ioType, int btnIdx);
		static int NextLogicalButton(IoType_t ioType, int btnIdx);

		/** ZOMG savestate functions. **/
		// TODO: Move this struct to libzomg.
		/*
		struct Zomg_MD_IoSave_int_t
		{
			uint8_t data;
			uint8_t ctrl;
			uint8_t ser_tx;
			uint8_t ser_rx;
			uint8_t ser_ctrl;
		};
		void zomgSaveMD(Zomg_MD_IoSave_int_t *state) const;
		void zomgRestoreMD(const Zomg_MD_IoSave_int_t *state);
		*/
};

}

#endif /* __LIBGENS_IOMANAGER_HPP__ */
