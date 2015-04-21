/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Psg.hpp: TI SN76489 (PSG) emulator.                                     *
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

#ifndef __LIBGENS_SOUND_PSG_HPP__
#define __LIBGENS_SOUND_PSG_HPP__

// C includes.
#include <stdint.h>

// LibGens includes.
#include "../macros/common.h"

struct _Zomg_PsgSave_t;

namespace LibGens {

class PsgPrivate;
class Psg
{
	public:
		Psg();
		Psg(int clock, int rate);
		~Psg();

	protected:
		friend class PsgPrivate;
		PsgPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Psg(const Psg &);
		Psg &operator=(const Psg &);

	public:
		void reInit(int clock, int rate);

		/**
		 * Reset the PSG state.
		 */
		void reset(void);
		
		/**
		 * Write to the PSG's data port.
		 * @param data Data value.
		 */
		void write(uint8_t data);

		/** ZOMG savestate functions. **/
		void zomgSave(_Zomg_PsgSave_t *state);
		void zomgRestore(const _Zomg_PsgSave_t *state);
		
		/** Gens-specific code. */
		void specialUpdate(void);

		/** FIXME: This sound mixing code needs to be totally redone. **/

		// PSG write length.
		void addWriteLen(int len);
		void clearWriteLen(void);

		// Reset buffer pointers.
		void resetBufferPtrs(void);

	public:
		// Super secret debug stuff!
		// For use by MDP plugins and test suites.
		// Return value is an MDP error code.
		int dbg_getReg(int reg_num, uint16_t *out) const;
		int dbg_setReg(int reg_num, uint16_t val);
		int dbg_getRegNumLatch(uint8_t *out) const;
		int dbg_setRegNumLatch(uint8_t val);
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
