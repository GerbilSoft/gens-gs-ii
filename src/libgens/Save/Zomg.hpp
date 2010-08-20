/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Zomg.hpp: Zipped Original Memory from Genesis savestate handler.        *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#ifndef __LIBGENS_SAVE_ZOMG_HPP__
#define __LIBGENS_SAVE_ZOMG_HPP__

#include "../../extlib/minizip/unzip.h"

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

namespace LibGens
{

class Zomg
{
	public:
		Zomg(const char *filename);
		~Zomg();
		
		bool isOpen(void) const { return (m_zFile != NULL); }
		void close(void);
		
		int load(void);
	
	protected:
		unzFile m_zFile;
		
		int loadFromZomg(const char *filename, void *buf, int len);
		
		// Savestate buffers.
		struct ZomgCommon_t
		{
			union ZomgVdpReg_t
			{
				uint8_t tms9918[8];	// TMS9918: 0x00 - 0x07
				uint8_t sms[11];	// SMS/GG: 0x00 - 0x0A
				uint8_t md[24];		// MD: 0x00 - 0x17
			};
			ZomgVdpReg_t VdpReg;
			
			union ZomgVRam_t
			{
				uint8_t sms[16384];	// TMS9918/SMS/GG
				uint16_t md[32768];	// MD
			};
			ZomgVRam_t VRam;
			
			union ZomgCRam_t
			{
				uint8_t sms[32];	// SMS only
				uint16_t gg[32];	// GG (little-endian)
				uint16_t md[64];	// MD (big-endian)
			};
			ZomgCRam_t CRam;
		};
		ZomgCommon_t m_common;
		
		struct ZomgMd_t
		{
			uint8_t VSRam[80];
		};
		ZomgMd_t m_md;
};

}

#endif /* __LIBGENS_SAVE_ZOMG_HPP__ */
