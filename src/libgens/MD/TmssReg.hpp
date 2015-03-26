/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * TmssReg.hpp: MD TMSS registers.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBGENS_MD_TMSSREG_HPP__
#define __LIBGENS_MD_TMSSREG_HPP__

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <climits>

// Starscream.
#include "libgens/cpu/star_68k.h"

namespace LibGens {

class TmssReg
{
	public:
		TmssReg();
		~TmssReg();

		/**
		 * $A14000: TMSS control register.
		 * This must have 'SEGA' in order to use the VDP.
		 * Otherwise, the VDP will lock on when accessed.
		 * NOTE: This is not emulated at the moment.
		 *
		 * NOTE 2: This is stored in 32-bit host-endian.
		 * TODO: Add read/write functions.
		 */
		union {
			uint32_t d;
			uint16_t w[2];
			uint8_t  b[4];
		} a14000;

		/**
		 * $A14101: TMSS ROM mapping register. (!CART_CE)
		 * Bit 0 indicates if the cartridge or TMSS ROM is mapped.
		 * - 0: TMSS ROM is mapped.
		 * - 1: Cartridge is mapped.
		 */
		uint8_t n_cart_ce;

		/**
		 * Check if TMSS is mapped.
		 * @return True if mapped; false if not.
		 */
		bool isTmssMapped(void) const;

		/**
		 * Reset the TMSS registers.
		 */
		void reset(void);

		/**
		 * Load the TMSS ROM and enable TMSS.
		 * M68K memory map must be updated afterwards.
		 * @return 0 on success; non-zero on error.
		 */
		int loadTmssRom(void);

		/**
		 * Clear the TMSS ROM and disable TMSS.
		 * M68K memory map must be updated afterwards.
		 */
		void clearTmssRom(void);

		/**
		 * Is TMSS enabled and loaded?
		 * @return True if TMSS is enabled and loaded; false if not.
		 */
		bool isTmssEnabled(void) const;

		/**
		 * Read a byte from the TMSS ROM.
		 * @param address Address.
		 * @return Byte from the TMSS ROM.
		 */
		uint8_t readByte(uint32_t address) const;

		/**
		 * Read a word from the TMSS ROM.
		 * @param address Address.
		 * @return Word from the TMSS ROM.
		 */
		uint16_t readWord(uint32_t address) const;

		/**
		 * Update M68K CPU program access structs for bankswitching purposes.
		 * @param M68K_Fetch Pointer to first STARSCREAM_PROGRAMREGION to update.
		 * @param banks Maximum number of banks to update.
		 * @return Number of banks updated.
		 */
		int updateSysBanking(STARSCREAM_PROGRAMREGION *M68K_Fetch, int banks);

	private:
		// TMSS ROM data. (Should be a power of two.)
		uint8_t *m_tmssRom;		// If not nullptr, TMSS is enabled.
		uint32_t m_tmssRom_size_real;	// Actual size of the TMSS ROM.
		uint32_t m_tmssRom_size;	// Allocated size. (TODO: Is this needed?)
		uint32_t m_tmssRom_mask;	// Address mask.

		// Find the next highest power of two. (unsigned integers)
		// http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
		template <class T>
		static inline T next_pow2u(T k)
		{
			if (k == 0)
				return 1;
			k--;
			for (unsigned int i = 1; i < sizeof(T)*CHAR_BIT; i <<= 1)
				k = k | k >> i;
			return k + 1;
		}
};

/**
 * Check if TMSS is mapped.
 * @return True if mapped; false if not.
 */
inline bool TmssReg::isTmssMapped(void) const
	{ return (m_tmssRom != nullptr && !(n_cart_ce & 1)); }

/**
 * Reset the TMSS registers.
 */
inline void TmssReg::reset(void)
{
	a14000.d = 0;
	n_cart_ce = 0;
}

/**
 * Is TMSS enabled and loaded?
 * @return True if TMSS is enabled and loaded; false if not.
 */
inline bool TmssReg::isTmssEnabled(void) const
{
	return (m_tmssRom != nullptr);
}

}

#endif /* __LIBGENS_MD_TMSSREG_HPP__ */
