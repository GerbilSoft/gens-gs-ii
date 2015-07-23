/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoTeamPlayer.hpp: Sega Mega Drive Team Player adapter.                  *
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

#ifndef __LIBGENS_IO_IOTEAMPLAYER_HPP__
#define __LIBGENS_IO_IOTEAMPLAYER_HPP__

#include "Device.hpp"

namespace LibGens { namespace IO {

class IoTeamPlayer : public Device
{
	public:
		IoTeamPlayer();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		IoTeamPlayer(const IoTeamPlayer &);
		IoTeamPlayer &operator=(const IoTeamPlayer &);

	public:
		/**
		 * Reset Device data that only affects the device
		 * and not the emulation-side registers.
		 *
		 * Should be overridden by subclasses that have
		 * device-specific data.
		 */
		virtual void resetDev(void) final;

		/**
		 * Update the I/O device.
		 * Runs the internal device update.
		 */
		virtual void update(void) final;

		/**
		 * Set a sub-device.
		 * Used for multitaps.
		 * @param virtPort Virtual port number. (0-3)
		 * @param ioDevice I/O device.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int setSubDevice(int virtPort, Device *ioDevice) final;

	protected:
		/**
		 * @name TP_PadType
		 * Indicates the type of controller connected to
		 * a given port on the Sega Team Player adapter.
		 */
		enum TP_PadType {
			TP_PT_NONE	= 0xF,
			TP_PT_3BTN	= 0x0,
			TP_PT_6BTN	= 0x1,
			TP_PT_MOUSE	= 0x2,
		};

		/**
		 * @name TP_DataType
		 * Indicates the type of data that should be returned
		 * in the current cycle.
		 */
		enum TP_DataType {
			// Initialization
			TP_DT_INIT			= 0,	// Initial state.
			TP_DT_START			= 1,	// Start request.
			TP_DT_ACK1			= 2,	// Acknowledgement.
			TP_DT_ACK2			= 3,	// Acknowledgement.
			TP_DT_PADTYPE_A			= 4,	// Pad type for Controller A
			TP_DT_PADTYPE_B			= 5,	// Pad type for Controller A
			TP_DT_PADTYPE_C			= 6,	// Pad type for Controller A
			TP_DT_PADTYPE_D			= 7,	// Pad type for Controller A

			// Controller A
			TP_DT_PADA_RLDU			= 8,	// D-pad
			TP_DT_PADA_SACB			= 9,	// Start, A, C, B
			TP_DT_PADA_MXYZ			= 10,	// Mode, X, Y, Z (6-button only)

			// Controller A (Mega Mouse)
			TP_DT_PADA_MOUSE_SIGNOVER	= 8,
			TP_DT_PADA_MOUSE_BUTTONS	= 9,
			TP_DT_PADA_MOUSE_X_MSN		= 10,
			TP_DT_PADA_MOUSE_X_LSN		= 11,
			TP_DT_PADA_MOUSE_Y_MSN		= 12,
			TP_DT_PADA_MOUSE_Y_LSN		= 13,

			// Controller B
			TP_DT_PADB_RLDU			= 14,	// D-pad
			TP_DT_PADB_SACB			= 15,	// Start, A, C, B
			TP_DT_PADB_MXYZ			= 16,	// Mode, X, Y, Z (6-button only)

			// Controller B (Mega Mouse)
			TP_DT_PADB_MOUSE_SIGNOVER	= 14,
			TP_DT_PADB_MOUSE_BUTTONS	= 15,
			TP_DT_PADB_MOUSE_X_MSN		= 16,
			TP_DT_PADB_MOUSE_X_LSN		= 17,
			TP_DT_PADB_MOUSE_Y_MSN		= 18,
			TP_DT_PADB_MOUSE_Y_LSN		= 19,

			// Controller C
			TP_DT_PADC_RLDU			= 20,	// D-pad
			TP_DT_PADC_SACB			= 21,	// Start, A, C, B
			TP_DT_PADC_MXYZ			= 22,	// Mode, X, Y, Z (6-button only)

			// Controller C (Mega Mouse)
			TP_DT_PADC_MOUSE_SIGNOVER	= 20,
			TP_DT_PADC_MOUSE_BUTTONS	= 21,
			TP_DT_PADC_MOUSE_X_MSN		= 22,
			TP_DT_PADC_MOUSE_X_LSN		= 23,
			TP_DT_PADC_MOUSE_Y_MSN		= 24,
			TP_DT_PADC_MOUSE_Y_LSN		= 25,

			// Controller D
			TP_DT_PADD_RLDU			= 26,	// D-pad
			TP_DT_PADD_SACB			= 27,	// Start, A, C, B
			TP_DT_PADD_MXYZ			= 28,	// Mode, X, Y, Z (6-button only)

			// Controller D (Mega Mouse)
			TP_DT_PADD_MOUSE_SIGNOVER	= 26,
			TP_DT_PADD_MOUSE_BUTTONS	= 27,
			TP_DT_PADD_MOUSE_X_MSN		= 28,
			TP_DT_PADD_MOUSE_X_LSN		= 29,
			TP_DT_PADD_MOUSE_Y_MSN		= 30,
			TP_DT_PADD_MOUSE_Y_LSN		= 31,

			TP_DT_MAX
		};

		/**
		 * Connected controllers.
		 * NOTE: This object does NOT own these IoDevices.
		 */
		Device *pads[4];

		// Team Player data.
		uint8_t padTypes[4];					// TP_PadType
		uint8_t ctrlIndexTbl[TP_DT_MAX - TP_DT_PADA_RLDU];	// TP_DataType

		/**
		 * Rebuild the controller index table.
		 */
		void rebuildCtrlIndexTable(void);
};

} }

#endif /* __LIBGENS_IO_IOTEAMPLAYER_HPP__ */
