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

// MiniZip
#include "../../../extlib/minizip/zip.h"
#include "../../../extlib/minizip/unzip.h"

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// ZOMG save structs.
#include "zomg_vdp.h"
#include "zomg_psg.h"
#include "zomg_ym2612.h"

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
		Zomg_VdpSave_t m_vdp;
		
		
		struct ZomgMd_t
		{
			uint8_t VSRam[80];
		};
		ZomgMd_t m_md;
};

}

#endif /* __LIBGENS_SAVE_ZOMG_HPP__ */
