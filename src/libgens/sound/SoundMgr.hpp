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

// Blip_Buffer.
#include "Blip_Buffer/Blip_Buffer.h"

// Audio ICs.
#include "../sound/Psg.hpp"
#include "../sound/PsgHq.hpp"
#include "../sound/Ym2612.hpp"

// ZOMG save structs.
#include "libzomg/zomg_psg.h"
#include "libzomg/zomg_ym2612.h"

namespace LibGens
{

class SoundMgr
{
	public:
		static void Init(void);
		static void End(void);
		
		static void ReInit(int rate, bool isPal, bool preserveState = false);
		static inline void SetRate(int rate, bool preserveState = true)
		{
			ReInit(rate, ms_IsPal, preserveState);
		}
		static inline void SetRegion(bool isPal, bool preserveState = true)
		{
			ReInit(ms_Rate, isPal, preserveState);
		}
		
		static inline int GetSegLength(void) { return ms_SegLength; }
		
		// TODO: Bounds checking.
		static inline int GetWritePos(int line) { return ms_Extrapol[line][0]; }
		static inline int GetWriteLen(int line) { return ms_Extrapol[line][1]; }
		
		// Maximum sampling rate and segment size.
		static const int MAX_SAMPLING_RATE = 48000;
		static const int MAX_SEGMENT_SIZE = 960;	// ceil(MAX_SAMPLING_RATE / 50)
		
		// Segment buffer.
		// Stores up to MAX_SEGMENT_SIZE 16-bit stereo samples.
		// (Samples are actually 32-bit in order to handle oversaturation properly.)
		// TODO: Call the write functions from SoundMgr so this doesn't need to be public.
		// TODO: Convert to interleaved stereo.
		static int32_t ms_SegBufL[MAX_SEGMENT_SIZE];
		static int32_t ms_SegBufR[MAX_SEGMENT_SIZE];
		
		// Use Blip_Buffer?
		static inline bool IsUsingBlipBuffer(void)
			{ return ms_UseBlipBuffer; }
		static void SetUsingBlipBuffer(bool newUsingBlipBuffer);
		
		/**
		 * ResetPtrsAndLens(): Reset buffer pointers and lengths.
		 */
		static void ResetPtrsAndLens(void);
		
		/**
		 * SpecialUpdate(): Run the specialUpdate() functions.
		 */
		static void SpecialUpdate(void);
		
		// IC wrapper functions.
		static void ResetPSG(void);
		static void UpdatePSG(int writeLen, int cycles);
		static void WritePSG(uint8_t data);
		static void ZomgRestorePSG(const Zomg_PsgSave_t *psg_save);
		static void ZomgSavePSG(Zomg_PsgSave_t *psg_save);
		
		static void ResetYM2612(void);
		static void UpdateYM2612(int32_t *bufL, int32_t *bufR, int writeLen);
		static void WriteYM2612(uint32_t address, uint8_t data);
		static uint8_t ReadYM2612(void);
		static void ZomgRestoreYM2612(const Zomg_Ym2612Save_t *ym2612_save);
		static void ZomgSaveYM2612(Zomg_Ym2612Save_t *ym2612_save);
	
	protected:
		// Segment length.
		static int CalcSegLength(int rate, bool isPal);
		static int ms_SegLength;
		
		static int ms_Rate;
		static bool ms_IsPal;
		
		// Line extrapolation values. [312 + extra room to prevent overflows]
		// Index 0 == start; Index 1 == length
		static unsigned int ms_Extrapol[312+8][2];
		
		// Audio ICs.
		static Psg *ms_Psg;
		static Ym2612 *ms_Ym2612;
		
		// Blip_Buffer and high-quality audio ICs.
		static void SetUsingBlipBuffer_int(bool newUsingBlipBuffer);
		static bool ms_UseBlipBuffer;
		static Blip_Buffer *ms_BlipBuffer;
		static PsgHq *ms_PsgHq;
	
	private:
		SoundMgr() { }
		~SoundMgr() { }
};


// TODO: Make Psg and PsgHq inherit from a common base class?


inline void SoundMgr::SetUsingBlipBuffer(bool newUsingBlipBuffer)
{
	if (ms_UseBlipBuffer == newUsingBlipBuffer)
		return;
	
	SetUsingBlipBuffer_int(newUsingBlipBuffer);
}


/** PSG wrapper functions. **/


/**
 * ResetPSG(): Reset the PSG.
 */
inline void SoundMgr::ResetPSG(void)
{
	if (!ms_UseBlipBuffer)
		ms_Psg->reset();
	else
		ms_PsgHq->reset();
}

/**
 * UpdatePSG(): Update the PSG.
 * @param writeLen Write length. (classic only)
 * @param cycles PSG cycles. (Blip_Buffer only)
 */
inline void SoundMgr::UpdatePSG(int writeLen, int cycles)
{
	if (!ms_UseBlipBuffer)
		ms_Psg->addWriteLen(writeLen);
	else
		ms_PsgHq->runCycles(cycles);
}

inline void SoundMgr::WritePSG(uint8_t data)
{
	if (!ms_UseBlipBuffer)
		ms_Psg->write(data);
	else
		ms_PsgHq->write(data);
}

inline void SoundMgr::ZomgRestorePSG(const Zomg_PsgSave_t *psg_save)
{
	if (!ms_UseBlipBuffer)
		ms_Psg->zomgRestore(psg_save);
	else
		ms_PsgHq->zomgRestore(psg_save);
}

inline void SoundMgr::ZomgSavePSG(Zomg_PsgSave_t *psg_save)
{
	if (!ms_UseBlipBuffer)
		ms_Psg->zomgSave(psg_save);
	else
		ms_PsgHq->zomgSave(psg_save);
}


/** YM2612 wrapper functions. **/


/**
 * ResetYM2612(): Reset the YM2612.
 */
inline void SoundMgr::ResetYM2612(void)
{
	if (!ms_UseBlipBuffer)
		ms_Ym2612->reset();
	// TODO: Blip_Buffer YM2612.
}

/**
 * UpdateYM2612(): Update the YM2612.
 * @param bufL Left buffer. (classic only)
 * @param bufR Right buffer. (classic only)
 * @param writeLen Write length. (classic only)
 */
inline void SoundMgr::UpdateYM2612(int32_t *bufL, int32_t *bufR, int writeLen)
{
	if (!ms_UseBlipBuffer)
	{
		ms_Ym2612->updateDacAndTimers(bufL, bufR, writeLen);
		ms_Ym2612->addWriteLen(writeLen);
	}
	// TODO: Blip_Buffer YM2612.
}

inline void SoundMgr::WriteYM2612(uint32_t address, uint8_t data)
{
	if (!ms_UseBlipBuffer)
		ms_Ym2612->write(address, data);
	// TODO: Blip_Buffer YM2612.
}

inline uint8_t SoundMgr::ReadYM2612(void)
{
	if (!ms_UseBlipBuffer)
		return ms_Ym2612->read();
	else
		return 0xFF;
	// TODO: Blip_Buffer YM2612.
}

inline void SoundMgr::ZomgRestoreYM2612(const Zomg_Ym2612Save_t *ym2612_save)
{
	if (!ms_UseBlipBuffer)
		ms_Ym2612->zomgRestore(ym2612_save);
	// TODO: Blip_Buffer YM2612.
}

inline void SoundMgr::ZomgSaveYM2612(Zomg_Ym2612Save_t *ym2612_save)
{
	if (!ms_UseBlipBuffer)
		ms_Ym2612->zomgSave(ym2612_save);
}

}

#endif /* __LIBGENS_SOUND_SOUNDMGR_HPP__ */
