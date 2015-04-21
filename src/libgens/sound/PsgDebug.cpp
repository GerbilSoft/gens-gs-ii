/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Psg.cpp: TI SN76489 (PSG) emulator: Debugging functions.                *
 * For use by MDP plugins and test suites.                                 *
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

#include "Psg.hpp"
#include "Psg_p.hpp"

namespace LibGens {

// TODO: Use MDP error codes.

/**
 * Get a PSG register value.
 * @param reg_num Register number.
 * @param out Buffer for register value.
 * @return MDP error code.
 */
int Psg::dbg_getReg(int reg_num, uint16_t *out) const
{
	if (reg_num < 0 || reg_num >= 8)
		return -1;
	// Tone registers are 10-bit.
	// Volume registers are 4-bit.
	*out = (uint16_t)d->reg[reg_num];
	return 0;
}

/**
 * Set a PSG register value.
 * @param reg_num Register number.
 * @param val Register value.
 * @return MDP error code.
 */
int Psg::dbg_setReg(int reg_num, uint16_t val)
{
	if (reg_num < 0 || reg_num >= 8)
		return -1;
	// Tone registers are 10-bit.
	// Volume registers are 4-bit.
	// TODO: Mask the registers and process the value.
	return -1;
}

/**
 * Get the internal register number latch.
 * @param out Buffer for the register number latch.
 * @return MDP error code.
 */
int Psg::dbg_getRegNumLatch(uint8_t *out) const
{
	*out = (uint8_t)d->curReg;
	return 0;
}

/**
 * Set the internal register number latch.
 * @param val Register number latch.
 * @return MDP error code.
 */
int Psg::dbg_setRegNumLatch(uint8_t val)
{
	d->curReg = val;
	return 0;
}

}
