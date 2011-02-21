/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * PsgHq.hpp: TI SN76489 (PSG) emulator. (High Quality)                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2011 by David Korth                                  *
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

#ifndef __LIBGENS_SOUND_PSGHQ_HPP__
#define __LIBGENS_SOUND_PSGHQ_HPP__

// C includes.
#include <stdint.h>
#include <stdio.h>

// LibGens includes.
#include "../macros/common.h"

// ZOMG PSG struct.
#include "libzomg/zomg_psg.h"

// Blip Buffer.
#include "Blip_Buffer/Blip_Buffer.h"

namespace LibGens
{

class PsgHq
{
	public:
		PsgHq();
		PsgHq(int clock, int rate);
		
		void reInit(int clock, int rate);
		
		/**
		 * reset(): Reset the PSG state.
		 */
		inline void reset(void) { zomgRestore(&ms_psgStateInit); }
		
		/* PSG manipulation functions. */
		void write(uint8_t data);
		void runCycles(int cycles);
		void render(int32_t *bufL, int32_t *bufR);
		
		/** ZOMG savestate functions. **/
		void zomgSave(Zomg_PsgSave_t *state);
		void zomgRestore(const Zomg_PsgSave_t *state);
		
		/** Gens-specific code. */
		int getReg(int regID);
	
	protected:
		int m_curChan;	// Current channel.
		int m_curReg;	// Current register.
		
		// User-accessible registers.
		// Contains attenuation and reset counters for tones.
		unsigned int m_reg[8];		// Registers.
		
		// Internal registers.
		int8_t m_volume[4];	// Volume for each channel.
		int m_counter[4];	// Tone counters for the four channels.
		bool m_isOutput[4];	// True if a channel output is currently HI.
		
		/* Noise channel variables. */
		unsigned int m_lfsrMask;	// Linear Feedback Shift Register mask.
		unsigned int m_lfsr;		// Linear Feedback Shift Register contents.
		
		// Lookup tables.
		static const int8_t ms_VolTable[16];
		
		/* LFSR constants. */
		static const unsigned int LFSR_MASK_WHITE	= 0x0009;
		static const unsigned int LFSR_MASK_PERIODIC	= 0x0001;
		static const unsigned int LFSR_INIT		= 0x8000;
		
		/** parity16() and LFSR16_Shift() from http://www.smspower.org/dev/docs/wiki/Sound/PSG#noisegeneration **/
		
		/**
		* parity16(): Get the parity of a 16-bit value.
		* @param n Value to check.
		* @return Parity.
		*/
		static inline unsigned int FUNC_PURE parity16(unsigned int n)
		{
			n ^= n >> 8;
			n ^= n >> 4;
			n ^= n >> 2;
			n ^= n >> 1;
			return (n & 1);
		}
		
		/**
		* LFSR16_Shift(): Shift the Linear Feedback Shift Register. (16-bit LFSR)
		* @param LFSR Current LFSR contents.
		* @param LFSR_Mask LFSR mask.
		* @return Shifted LFSR value.
		*/
		static inline unsigned int FUNC_PURE LFSR16_Shift(unsigned int LFSR, unsigned int LFSR_Mask)
		{
			return (LFSR >> 1) |
				(((LFSR_Mask > 1)
					? parity16(LFSR & LFSR_Mask)
					: (LFSR & LFSR_Mask)) << 15);
		}
		
		/* Maximum output. (default = 0x7FFF) */
		static const unsigned int MAX_OUTPUT = 0x4FFF;
		
		/* Initial PSG state. */
		static const Zomg_PsgSave_t ms_psgStateInit;
		
	private:
		// Blip Buffer objects.
		Blip_Buffer m_output;					// Output buffer.
		Blip_Synth<blip_good_quality, 64*2> m_synthTone[3];	// Tone generators.
		Blip_Synth<blip_low_quality, 64*2>  m_synthNoise;	// Noise generator.
		
		int m_cycles;		// Current cycle.
};

}

#endif /* __LIBGENS_SOUND_PSGHQ_HPP__ */
