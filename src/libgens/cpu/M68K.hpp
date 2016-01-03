/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K.hpp: Main 68000 CPU wrapper class.                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_CPU_M68K_HPP__
#define __LIBGENS_CPU_M68K_HPP__

#include <libgens/config.libgens.h>

#include "star_68k.h"

// ZOMG M68K structs.
#include "libzomg/zomg_m68k.h"

// TODO: Move these elsewhere!
#define CLOCK_NTSC 53693175
#define CLOCK_PAL  53203424

namespace LibGens
{

class M68K
{
	public:
		static void Init(void);
		static void End(void);
		
		/**
		 * @name System IDs
		 * TODO: Use MDP system IDs?
		 */
		enum SysID {
			SYSID_NONE	= 0,

			SYSID_MD,
			SYSID_MCD,
			SYSID_32X,

			// TODO: Not implemented...
			SYSID_PICO,

			SYSID_MAX
		};
		
		static void InitSys(SysID system);
		static void EndSys(void);
		static void UpdateSysBanking(void);
		
		/** ZOMG savestate functions. **/
		static void ZomgSaveReg(Zomg_M68KRegSave_t *state);
		static void ZomgRestoreReg(const Zomg_M68KRegSave_t *state);
		
		/** BEGIN: Starscream wrapper functions. **/
		static inline void Reset(void);
		static inline int Interrupt(int level, int vector);
		static inline unsigned int ReadOdometer(void);
		static inline void ReleaseCycles(int cycles);
		static inline void AddCycles(int cycles);
		static inline unsigned int Exec(int n);
		static inline unsigned int TripOdometer(void);
		/** END: Starscream wrapper functions. **/
	
	protected:
		static S68000CONTEXT ms_Context;
		
		static STARSCREAM_PROGRAMREGION M68K_Fetch[];
		static STARSCREAM_DATAREGION M68K_Read_Byte[4];
		static STARSCREAM_DATAREGION M68K_Read_Word[4];
		static STARSCREAM_DATAREGION M68K_Write_Byte[3];
		static STARSCREAM_DATAREGION M68K_Write_Word[3];
		
		// TODO: What does the Reset Handler function do?
		static void M68K_Reset_Handler(void);
	
	private:
		M68K() { }
		~M68K() { }

		static SysID ms_LastSysID;
};

/** BEGIN: Starscream wrapper functions. **/

#ifdef GENS_ENABLE_EMULATION
/**
 * Reset the emulated CPU.
 */
inline void M68K::Reset(void)
{
	main68k_reset();
}

/**
 * Trigger an interrupt.
 * @param level Interrupt level.
 * @param vector Interrupt vector. (???)
 * @return ???
 */
inline int M68K::Interrupt(int level, int vector)
{
	return main68k_interrupt(level, vector);
}

/**
 * Read the M68K odometer.
 * @return M68K odometer.
 */
inline unsigned int M68K::ReadOdometer(void)
{
	return main68k_readOdometer();
}

/**
* Release cycles.
* @param cycles Cycles to release.
*/
inline void M68K::ReleaseCycles(int cycles)
{
	main68k_releaseCycles(cycles);
}

/**
 * Add cycles to the M68K odometer.
 * @param cycles Number of cycles to add.
 */
inline void M68K::AddCycles(int cycles)
{
	main68k_addCycles(cycles);
}

/**
 * Execute instructions for a given number of cycles.
 * @param n Number of cycles to execute.
 * @return ???
 */
inline unsigned int M68K::Exec(int n)
{
	return main68k_exec(n);
}

/**
* Clear the M68K odometer.
* @return ???
*/
inline unsigned int M68K::TripOdometer(void)
{
	return main68k_tripOdometer();
}

#else /* !GENS_ENABLE_EMULATION */

inline void M68K::Reset(void) { }
inline int M68K::Interrupt(int level, int vector) { ((void)level); ((void)vector); return -1; }
inline unsigned int M68K::ReadOdometer(void) { return 0; }
inline void M68K::ReleaseCycles(int cycles) { ((void)cycles); }
inline void M68K::AddCycles(int cycles) { ((void)cycles); }
inline unsigned int M68K::Exec(int n) { ((void)n); return 0; }
inline unsigned int M68K::TripOdometer(void) { return 0; }

#endif /* GENS_ENABLE_EMULATION */

/** END: Starscream wrapper functions. **/

}

#endif /* __LIBGENS_CPU_M68K_HPP__ */
