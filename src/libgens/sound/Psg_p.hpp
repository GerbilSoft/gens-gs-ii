/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Psg_p.hpp: TI SN76489 (PSG) emulator. (Private class)                   *
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

#ifndef __LIBGENS_SOUND_PSG_P_HPP__
#define __LIBGENS_SOUND_PSG_P_HPP__

// C includes.
#include <stdint.h>

// ZOMG
#include "libzomg/zomg_psg.h"

namespace LibGens {

// TODO: Needs more optimization.
class PsgPrivate
{
	public:
		PsgPrivate(Psg *q);

	protected:
		friend class Psg;
		Psg *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		PsgPrivate(const PsgPrivate &);
		PsgPrivate &operator=(const PsgPrivate &);

	public:
		void update(int32_t *bufL, int32_t *bufR, int length);

		// Initial PSG state.
		static const Zomg_PsgSave_t psgStateInit;

		// Internal state.
		int curChan;	// Current channel.
		int curReg;	// Current register.

		unsigned int reg[8];		// Registers.
		unsigned int counter[4];	// Counters for the four channels.
		unsigned int cntStep[4];	// Counter steps when using interpolation.

		unsigned int volume[4];		// Volume for each channel.

		/* Noise channel variables. */
		unsigned int lfsrMask;		// Linear Feedback Shift Register mask.
		unsigned int lfsr;		// Linear Feedback Shift Register contents.

		/* Lookup tables. */
		unsigned int stepTable[1024];
		unsigned int volumeTable[16];
		unsigned int noiseStepTable[4];

		/* LFSR constants. */
		static const unsigned int LFSR_MASK_WHITE	= 0x0009;
		static const unsigned int LFSR_MASK_PERIODIC	= 0x0001;
		static const unsigned int LFSR_INIT		= 0x8000;

		/**
		 * parity16() and LFSR16_Shift() from:
		 * http://www.smspower.org/dev/docs/wiki/Sound/PSG#noisegeneration
		 */

		/**
		* Get the parity of a 16-bit value.
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
		* Shift the Linear Feedback Shift Register. (16-bit LFSR)
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

		// PSG write length. (for audio output)
		int writeLen;
		bool enabled;

		// PSG buffer pointers.
		// TODO: Figure out how to get rid of these!
		int32_t *bufPtrL;
		int32_t *bufPtrR;
};

}

#endif /* __LIBGENS_SOUND_PSG_P_HPP__ */
