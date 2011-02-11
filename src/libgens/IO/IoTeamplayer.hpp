/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoTeamplayer.hpp: Sega Teamplayer device.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "IoBase.hpp"

namespace LibGens
{

class IoTeamplayer : public IoBase
{
	public:
		IoTeamplayer();
		IoTeamplayer(const IoBase *other);
		virtual ~IoTeamplayer() { }
		
		/**
		 * reset(): Reset function.
		 * Called when the system is reset.
		 */
		void reset(void);
		
		void writeCtrl(uint8_t ctrl);
		
		void writeData(uint8_t data);
		uint8_t readData(void);
		
		// I/O device update function.
		void update(void);
		
		// Controller configuration.
		const char *devName(void) const { return "Sega Teamplayer"; }
		IoType devType(void) const { return IOT_TEAMPLAYER; }
		int numButtons(void) const { return 12; }
		int nextLogicalButton(int button) const;
		const char *buttonName(int button) const;
	
	protected:
		/**
		 * @name PadType
		 * Indicates the type of controller connected to
		 * a given port on the Sega Teamplayer adapter.
		 */
		enum PadType
		{
			PT_NONE		= 0xF,
			PT_3BTN		= 0x0,
			PT_6BTN		= 0x1
		};
		
		/**
		 * @name DataType
		 * Indicates the type of data that should be returned
		 * in the current cycle.
		 */
		enum DataType
		{
			// Initialization
			DT_INIT		= 0,	// 0: Initial state.
			DT_START	= 1,	// 1: Start request.
			DT_ACK1		= 2,	// 2: Acknowledgement.
			DT_ACK2		= 3,	// 3: Acknowledgement.
			DT_PADTYPE_A	= 4,	// 4: Pad type for Controller A
			DT_PADTYPE_B	= 5,	// 5: Pad type for Controller A
			DT_PADTYPE_C	= 6,	// 6: Pad type for Controller A
			DT_PADTYPE_D	= 7,	// 7: Pad type for Controller A
			
			// Controller A
			DT_PADA_RLDU	= 8,	// D-pad
			DT_PADA_SACB	= 9,	// Start, A, C, B
			DT_PADA_MXYZ	= 10,	// Mode, X, Y, Z (6-button only)
			
			// Controller B
			DT_PADB_RLDU	= 11,	// D-pad
			DT_PADB_SACB	= 12,	// Start, A, C, B
			DT_PADB_MXYZ	= 13,	// Mode, X, Y, Z (6-button only)
			
			// Controller C
			DT_PADC_RLDU	= 14,	// D-pad
			DT_PADC_SACB	= 15,	// Start, A, C, B
			DT_PADC_MXYZ	= 16,	// Mode, X, Y, Z (6-button only)
			
			// Controller D
			DT_PADD_RLDU	= 17,	// D-pad
			DT_PADD_SACB	= 18,	// Start, A, C, B
			DT_PADD_MXYZ	= 19,	// Mode, X, Y, Z (6-button only)
			
			DT_MAX
		};
		
		/**
		 * m_counter: Data Type counter.
		 */
		int m_counter;
		
		/**
		 * m_ctrlType[]: Controller types.
		 */
		PadType m_ctrlType[4];
		
		/**
		 * m_ctrlData[]: Controller data.
		 * Indexes are (DataType - DT_PADA_RLDU).
		 * Format: RLDU, SACD, MXYZ
		 */
		uint8_t m_ctrlData[12];
		
		/**
		 * m_ctrlIndex[]: Controller data index table.
		 * Determines what data is returned on what cycle.
		 */
		DataType m_ctrlIndex[DT_MAX - DT_PADA_RLDU];
		void rebuildCtrlIndexTable(void);
};

}

#endif /* __LIBGENS_IO_IOTEAMPLAYER_HPP__ */
