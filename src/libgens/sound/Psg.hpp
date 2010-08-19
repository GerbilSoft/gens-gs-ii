/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Psg.hpp: TI SN76489 (PSG) emulator.                                     *
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

#ifndef __LIBGENS_SOUND_PSG_HPP__
#define __LIBGENS_SOUND_PSG_HPP__

// C includes.
#include <stdint.h>
#include <stdio.h>

// LibGens includes.
#include "../macros/common.h"

namespace LibGens
{

class Psg
{
	public:
		Psg();
		Psg(int clock, int rate);
		
		void reinit(int clock, int rate);
		
		/* PSG manipulation functions. */
		void write(uint8_t data);
		void update(int32_t *bufL, int32_t *bufR, int length);
		
		/* GSX savestate functions. */
		void saveState(uint32_t state[8]);
		void restoreState(const uint32_t state[8]);
		
		/** Gens-specific code. */
		int getReg(int regID);
		void specialUpdate(void);
		
		// PSG write length.
		inline void addWriteLen(int len) { m_writeLen += len; }
		inline void clearWriteLen(void) { m_writeLen = 0; }
		
		// Reset buffer pointers.
		void resetBufferPtrs(void);
		
		
	protected:
		int m_curChan;	// Current channel.
		int m_curReg;	// Current register.
		
		unsigned int m_reg[8];		// Registers.
		unsigned int m_counter[4];	// Counters for the four channels.
		unsigned int m_cntStep[4];	// Counter steps when using interpolation.
		
		unsigned int m_volume[4];	// Volume for each channel.
		
		/* Noise channel variables. */
		unsigned int m_lfsrMask;	// Linear Feedback Shift Register mask.
		unsigned int m_lfsr;		// Linear Feedback Shift Register contents.
		
		/* Lookup tables. */
		unsigned int m_stepTable[1024];
		unsigned int m_volumeTable[16];
		unsigned int m_noiseStepTable[4];
		
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
		static const uint32_t ms_psgStateInit[8];
		
		// PSG write length. (for audio output)
		int m_writeLen;
		bool m_enabled;
		
		// PSG buffer pointers.
		// TODO: Figure out how to get rid of these!
		int32_t *m_bufPtrL;
		int32_t *m_bufPtrR;
};

/* Gens */

#if 0
// Full PSG save/restore functions from Gens Rerecording.
struct _gsx_v7_psg;
void PSG_Save_State_GSX_v7(struct _gsx_v7_psg *save);
void PSG_Restore_State_GSX_v7(struct _gsx_v7_psg *save); 
#endif

/* end */

}

#endif /* __LIBGENS_SOUND_PSG_HPP__ */
