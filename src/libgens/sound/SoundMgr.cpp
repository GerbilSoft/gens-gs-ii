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
int i  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
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
int32_t SoundMgr::ms_SegBufL[MAX_SEGMENT_SIZE];
int32_t SoundMgr::ms_SegBufR[MAX_SEGMENT_SIZE];

// Audio ICs.
Psg *SoundMgr::ms_Psg = NULL;
Ym2612 *SoundMgr::ms_Ym2612 = NULL;

// Blip_Buffer and high-quality Audio ICs.
bool SoundMgr::ms_UseBlipBuffer = false;
Blip_Buffer *SoundMgr::ms_BlipBuffer = NULL;
PsgHq *SoundMgr::ms_PsgHq = NULL;

// Audio settings.
int SoundMgr::ms_Rate = 44100;
bool SoundMgr::ms_IsPal = false;

void SoundMgr::Init(void)
{
	// Don't use Blip_Buffer by default.
	SetUsingBlipBuffer_int(false);
}

void SoundMgr::End(void)
{
	// Delete "classic" audio objects.
	delete ms_Psg;		ms_Psg = NULL;
	delete ms_Ym2612;	ms_Ym2612 = NULL;
	
	// Delete Blip_Buffer audio objects.
	delete ms_PsgHq;	ms_PsgHq = NULL;
	delete ms_BlipBuffer;	ms_BlipBuffer = NULL;
}


void SoundMgr::SetUsingBlipBuffer_int(bool newUsingBlipBuffer)
{
	ms_UseBlipBuffer = newUsingBlipBuffer;
	
	if (ms_UseBlipBuffer)
	{
		// Enable Blip_Buffer.
		ms_BlipBuffer = new Blip_Buffer();
		ms_PsgHq = new PsgHq(ms_BlipBuffer);
		
		// Disable "classic" audio subsystem.
		delete ms_Psg;		ms_Psg = NULL;
		delete ms_Ym2612;	ms_Ym2612 = NULL;
	}
	else
	{
		// Enable "classic" audio subsystem.
		ms_Ym2612 = new Ym2612();
		ms_Psg = new Psg();
		
		// Disable Blip_Buffer.
		delete ms_PsgHq;	ms_PsgHq = NULL;
		delete ms_BlipBuffer;	ms_BlipBuffer = NULL;
	}
	
	// TODO: Reinitialize the Sound Manager.
	// TODO: Save/restore PSG and YM2612 settings.
}


/**
 * ReInit(): Reinitialize the Sound Manager.
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
	for (int i = 0; i < lines; i++)
	{
		ms_Extrapol[i][0] = ((ms_SegLength * i) / lines);
		ms_Extrapol[i][1] = (((ms_SegLength * (i+1)) / lines) - ms_Extrapol[i][0]);
	}
	
	// Clear the segment buffers.
	memset(ms_SegBufL, 0x00, sizeof(ms_SegBufL));
	memset(ms_SegBufR, 0x00, sizeof(ms_SegBufR));
	
	// If requested, save the PSG/YM state.
	// NOTE: Blip_Buffer versions don't handle sample rate themselves,
	// so they don't need the state saved/restored.
	Zomg_PsgSave_t psgState;
	Zomg_Ym2612Save_t ym2612State;
	if (preserveState && !ms_BlipBuffer)
	{
		ms_Psg->zomgSave(&psgState);
		ms_Ym2612->zomgSave(&ym2612State);
	}
	
	// Initialize the PSG and YM2612.
	const int master_clock = (isPal ? CLOCK_PAL : CLOCK_NTSC);
	if (!ms_BlipBuffer)
	{
		ms_Psg->reInit((int)((double)master_clock / 15.0), rate);
		ms_Ym2612->reInit((int)((double)master_clock / 7.0), rate);
	}
	else
	{
		// TODO: Use the master clock for the Blip_Buffer?
		// Currently using PSG clock only.
		ms_BlipBuffer->clock_rate((double)master_clock / 7.0);
		ms_BlipBuffer->sample_rate(rate);
		ms_BlipBuffer->clear();
	}
	
	// If requested, restore the PSG/YM state.
	if (preserveState && !ms_BlipBuffer)
	{
		ms_Psg->zomgRestore(&psgState);
		ms_Ym2612->zomgRestore(&ym2612State);
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
	if (rate > MAX_SAMPLING_RATE)
	{
		// TODO: Support higher rates than 48 kHz.
		rate = MAX_SAMPLING_RATE;
	}
	
	switch (rate)
	{
		case 11025:	return (isPal ? 220 : 184);
		case 16000:	return (isPal ? 320 : 267);
		case 22050:	return (isPal ? 441 : 368);
		case 32000:	return (isPal ? 640 : 534);
		case 44100:	return (isPal ? 882 : 735);
		case 48000:	return (isPal ? 960 : 800);
		default:
			// Segment size is ceil(rate / framesPerSecond).
			return ceil((double)rate / (isPal ? 50.0 : 60.0));
	}
}


/**
 * ResetPtrsAndLens(): Reset buffer pointers and lengths.
 * Classic mode only!
 */
void SoundMgr::ResetPtrsAndLens(void)
{
	if (ms_BlipBuffer)
		return;
	
	ms_Ym2612->resetBufferPtrs();
	ms_Ym2612->clearWriteLen();
	ms_Psg->resetBufferPtrs();
	ms_Psg->clearWriteLen();
}


/**
 * SpecialUpdate(): Run the specialUpdate() functions.
 */
void SoundMgr::SpecialUpdate(void)
{
	if (!ms_BlipBuffer)
	{
		// "Classic" audio subsystem.
		ms_Psg->specialUpdate();
		ms_Ym2612->specialUpdate();
	}
	else
	{
		// Blip_Buffer audio subsystem.
		// TODO: Make a separate function for Blip_Buffer.
		ms_BlipBuffer->end_frame(ms_PsgHq->cycles());
		ms_PsgHq->clearCycles();
		
		// TODO: Read the samples somewhere.
		ms_BlipBuffer->clear();
	}
}

}
