/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Rom.hpp: ROM loader.                                                    *
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

#ifndef __LIBGENS_ROM_HPP__
#define __LIBGENS_ROM_HPP__

#include <stdio.h>
#include <stdint.h>

// C++ includes.
#include <string>

/**
 * ROM_HEADER_SIZE: Number of bytes used for ROM type detection.
 */
#define ROM_HEADER_SIZE 65536

namespace LibGens
{

class Rom
{
	public:
		// TODO: Use MDP headers for system identifiers.
		enum MDP_SYSTEM_ID
		{
			/*! BEGIN: MDP v1.0 system IDs. !*/
			MDP_SYSTEM_UNKNOWN = 0,
			MDP_SYSTEM_MD      = 1,
			MDP_SYSTEM_MCD     = 2,
			MDP_SYSTEM_32X     = 3,
			MDP_SYSTEM_MCD32X  = 4,
			MDP_SYSTEM_SMS     = 5,
			MDP_SYSTEM_GG      = 6,
			MDP_SYSTEM_SG1000  = 7,
			MDP_SYSTEM_PICO    = 8,
			/*! END: MDP v1.0 system IDs. !*/
			
			MDP_SYSTEM_MAX
		};
		
		enum RomFormat
		{
			RFMT_UNKNOWN = 0,
			
			RFMT_BINARY,		// Plain binary ROM image.
			RFMT_SMD,		// Interleaved ROM image from Super Magic Drive.
			RFMT_SMD_SPLIT,		// Multi-part SMD image. (Probably won't be supported.)
			RFMT_MGD,		// Interleaved ROM image from Multi-Game-Doctor.
			
			// TODO: CD-ROM image handling.
			RFMT_CD_ISO,		// CD-ROM image, ISO-9660 format. (2048-byte sectors)
			RFMT_CD_BIN,		// CD-ROM image, BIN format. (2352-byte sectors)
			RFMT_CD_CUE,		// CD-ROM image, CUE sheet.
		};
		
		Rom(FILE *f, MDP_SYSTEM_ID sysOverride = MDP_SYSTEM_UNKNOWN, RomFormat fmtOverride = RFMT_UNKNOWN);
		~Rom();
		
		bool isOpen(void) { return (m_file != NULL); }
		void close(void) { fclose(m_file); m_file = NULL; }
	
	protected:
		MDP_SYSTEM_ID m_sys;
		RomFormat m_fmt;
		FILE *m_file;
		
		static RomFormat detectFormat(uint8_t header[ROM_HEADER_SIZE], size_t header_size);
		static MDP_SYSTEM_ID detectSystem(uint8_t header[ROM_HEADER_SIZE], size_t header_size, RomFormat fmt);
};

}

#endif /* __LIBGENS_ROM_HPP__ */
