/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SoundMgr.hpp: Sound manager.                                            *
 * Manages general sound stuff, e.g. line extrapolation.                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifndef __LIBGENS_SOUND_SOUNDMGR_HPP__
#define __LIBGENS_SOUND_SOUNDMGR_HPP__

// C includes.
#include <stdint.h>

namespace LibGens
{

class SoundMgr
{
	public:
		static void Init(void);
		static void End(void);
		
		static void ReInit(int rate, bool isPal);
		
		static int GetSegLength(void) { return ms_SegLength; }
		
		// TODO: Bounds checking.
		static int GetWritePos(int line) { return ms_Extrapol[line][0]; }
		static int GetWriteLen(int line) { return ms_Extrapol[line][1]; }
		
		// Segment buffer.
		// Stores up to 882 32-bit stereo samples.
		// (32-bit instead of 16-bit to handle oversaturation properly.)
		// TODO: Call the write functions from SoundMgr so this doesn't need to be public.
		// TODO: Convert to interleaved stereo.
		static int32_t ms_SegBufL[882];
		static int32_t ms_SegBufR[882];
	
	protected:
		// Segment length.
		static int CalcSegLength(int rate, bool isPal);
		static int ms_SegLength;
		
		// Line extrapolation values. [312 + extra room to prevent overflows]
		// Index 0 == start; Index 1 == length
		static unsigned int ms_Extrapol[312+8][2];
	
	private:
		SoundMgr() { }
		~SoundMgr() { }
};

}

#endif /* __LIBGENS_SOUND_SOUNDMGR_HPP__ */
