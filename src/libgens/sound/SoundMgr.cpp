/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SoundMgr.cpp: Sound manager.                                            *
 * Manages general sound stuff, e.g. line extrapolation.                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2015 by David Korth                                  *
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

// ZOMG
#include "libzomg/zomg_psg.h"
#include "libzomg/zomg_ym2612.h"

namespace LibGens {

// Static variable initialization.
int SoundMgr::ms_SegLength = 0;

// Line extrapolation values. [312 + extra room to prevent overflows]
// Index 0 == start; Index 1 == length
unsigned int SoundMgr::ms_Extrapol[312+8][2];

// Segment buffer.
// Stores up to 882 32-bit stereo samples.
// (32-bit instead of 16-bit to handle oversaturation properly.)
// TODO: Convert to interleaved stereo.
int32_t SoundMgr::ms_SegBufL[MAX_SEGMENT_SIZE];
int32_t SoundMgr::ms_SegBufR[MAX_SEGMENT_SIZE];

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
 * Reinitialize the Sound Manager.
 * @param rate Sound rate, in Hz.
 * @param isPal If true, system is PAL.
 * @param preserveState If true, save the PSG/YM state before reinitializing them.
 */
void SoundMgr::ReInit(int rate, bool isPal, bool preserveState)
{
	ms_Rate = rate;
	ms_IsPal = isPal;

	// Calculate the segment length.
	ms_SegLength = CalcSegLength(rate, isPal);

	// Build the sound extrapolation table.
	const int lines = (isPal ? 312 : 262);
	for (int i = 0; i < lines; i++) {
		ms_Extrapol[i][0] = ((ms_SegLength * i) / lines);
		ms_Extrapol[i][1] = (((ms_SegLength * (i+1)) / lines) - ms_Extrapol[i][0]);
	}

	// Clear the segment buffers.
	memset(ms_SegBufL, 0x00, sizeof(ms_SegBufL));
	memset(ms_SegBufR, 0x00, sizeof(ms_SegBufR));

	// If requested, save the PSG/YM state.
	Zomg_PsgSave_t psgState;
	Zomg_Ym2612Save_t ym2612State;
	if (preserveState) {
		ms_Psg.zomgSave(&psgState);
		ms_Ym2612.zomgSave(&ym2612State);
	}

	// Initialize the PSG and YM2612.
	if (isPal) {
		ms_Psg.reInit((int)((double)CLOCK_PAL / 15.0), rate);
		ms_Ym2612.reInit((int)((double)CLOCK_PAL / 7.0), rate);
	} else {
		ms_Psg.reInit((int)((double)CLOCK_NTSC / 15.0), rate);
		ms_Ym2612.reInit((int)((double)CLOCK_NTSC / 7.0), rate);
	}

	// If requested, restore the PSG/YM state.
	if (preserveState) {
		ms_Psg.zomgRestore(&psgState);
		ms_Ym2612.zomgRestore(&ym2612State);
	}
}

/**
 * Calculate the segment length.
 * @param rate Sound rate, in Hz.
 * @param isPal If true, system is PAL.
 * @return Segment length.
 */
int SoundMgr::CalcSegLength(int rate, bool isPal)
{
	if (rate > MAX_SAMPLING_RATE) {
		// TODO: Support higher rates than 48 kHz.
		rate = MAX_SAMPLING_RATE;
	}

	switch (rate) {
		case 11025:	return (isPal ? 220 : 184);
		case 16000:	return (isPal ? 320 : 267);
		case 22050:	return (isPal ? 441 : 368);
		case 32000:	return (isPal ? 640 : 534);
		case 44100:	return (isPal ? 882 : 735);
		case 48000:	return (isPal ? 960 : 800);
		default:
			// Segment size is ceil(rate / framesPerSecond).
			return (int)ceil((double)rate / (isPal ? 50.0 : 60.0));
	}
}

}
