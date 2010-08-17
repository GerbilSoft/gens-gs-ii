/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SoundMgr.cpp: Sound manager.                                            *
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

#include "SoundMgr.hpp"

// C includes.
#include <math.h>
#include <string.h>

// M68K.hpp has CLOCK_NTSC and CLOCK_PAL #defines.
// TODO: Convert to static const ints and move elsewhere.
#include "cpu/M68K.hpp"

namespace LibGens
{

// Static variable initialization.
int SoundMgr::ms_SegLength = 0;

// Line extrapolation values. [312 + extra room to prevent overflows]
// Index 0 == start; Index 1 == length
unsigned int SoundMgr::ms_Extrapol[312+8][2];

// Segment buffer.
// Stores up to 882 32-bit stereo samples.
// (32-bit instead of 16-bit to handle oversaturation properly.)
// TODO: Convert to interleaved stereo.
int32_t SoundMgr::ms_SegBufL[882];
int32_t SoundMgr::ms_SegBufR[882];

// Audio ICs.
Psg SoundMgr::ms_Psg;
Ym2612 SoundMgr::ms_Ym2612;

// Audio settings.
int SoundMgr::ms_Rate = 44100;
bool SoundMgr::ms_IsPal = false;

void SoundMgr::Init(void)
{
	// TODO
}

void SoundMgr::End(void)
{
	// TODO
}


/**
 * ReInit(): Reinitialize the Sound Manager.
 * @param rate Sound rate, in Hz.
 * @param isPal If true, system is PAL.
 */
void SoundMgr::ReInit(int rate, bool isPal)
{
	ms_Rate = rate;
	ms_IsPal = isPal;
	
	// Calculate the segment length.
	ms_SegLength = CalcSegLength(rate, isPal);
	
	// Build the sound extrapolation table.
	const int lines = (isPal ? 312 : 262);
	for (int i = 0; i < lines; i++)
	{
		ms_Extrapol[i][0] = ((ms_SegLength * i) / lines);
		ms_Extrapol[i][1] = (((ms_SegLength * (i+1)) / lines) - ms_Extrapol[i][0]);
	}
	
	// Clear the segment buffers.
	memset(ms_SegBufL, 0x00, sizeof(ms_SegBufL));
	memset(ms_SegBufR, 0x00, sizeof(ms_SegBufR));
	
	// Initialize the PSG and YM2612.
	if (isPal)
	{
		ms_Psg.reinit((int)((double)CLOCK_PAL / 15.0), rate);
		ms_Ym2612.reinit((int)((double)CLOCK_PAL / 7.0), rate);
	}
	else
	{
		ms_Psg.reinit((int)((double)CLOCK_NTSC / 15.0), rate);
		ms_Ym2612.reinit((int)((double)CLOCK_NTSC / 7.0), rate);
	}
}


/**
 * CalcSegLength(): Calculate the segment length.
 * @param rate Sound rate, in Hz.
 * @param isPal If true, system is PAL.
 * @return Segment length.
 */
int SoundMgr::CalcSegLength(int rate, bool isPal)
{
	if (rate > 44100)
	{
		// TODO: Support higher rates than 44.1 kHz.
		rate = 44100;
	}
	
	switch (rate)
	{
		case 11025:
			return (isPal ? 220 : 184);
		case 22050:
			return (isPal ? 441 : 368);
		case 44100:
			return (isPal ? 882 : 735);
		default:
			return rint((double)rate / (isPal ? 50 : 60));
	}
}

}
