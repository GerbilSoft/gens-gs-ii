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

// NOTE: This will include config.h for the current project, not necessarily libgens!
// TODO: Make a global config.h instead?
#include <config.h>

#include "star_68k.h"

// ZOMG M68K structs.
#include "libzomg/zomg_m68k.h"

// TODO: Move these elsewhere!
#define CLOCK_NTSC 53693175
#define CLOCK_PAL  53203424

namespace LibGens
{

class Zomg;

class M68K
{
	public:
		static void Init(void);
		static void End(void);
		
		/**
		 * @name System IDs
		 * TODO: Use MDP system IDs?
		 */
		enum SysID
		{
			SYSID_MD	= 0,
			SYSID_MCD	= 1,
			SYSID_32X	= 2,
			
			SYSID_MAX
		};
		
		static void InitSys(SysID system);
		
		/** ZOMG savestate functions. **/
		static void ZomgSaveReg(Zomg_M68KRegSave_t *state);
		static void ZomgRestoreReg(const Zomg_M68KRegSave_t *state);
		
		/** BEGIN: Starscream wrapper functions. **/
		
		/**
		 * Reset(): Reset the emulated CPU.
		 */
		static inline void Reset(void)
		{
#ifdef GENS_ENABLE_EMULATION
			main68k_reset();
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * Interrupt(): Trigger an interrupt.
		 * @param level Interrupt level.
		 * @param vector Interrupt vector. (???)
		 * @return ???
		 */
		static inline int Interrupt(int level, int vector)
		{
#ifdef GENS_ENABLE_EMULATION
			return main68k_interrupt(level, vector);
#else
			((void)level); ((void)vector);
			return -1;
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * ReadOdometer(): Read the M68K odometer.
		 * @return M68K odometer.
		 */
		static inline unsigned int ReadOdometer(void)
		{
#ifdef GENS_ENABLE_EMULATION
			return main68k_readOdometer();
#else
			return 0;
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * ReleaseCycles(): Release cycles.
		 * @param cycles Cycles to release.
		 */
		static inline void ReleaseCycles(int cycles)
		{
#ifdef GENS_ENABLE_EMULATION
			main68k_releaseCycles(cycles);
#else
			((void)cycles);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * AddCycles(): Add cycles to the M68K odometer.
		 * @param cycles Number of cycles to add.
		 */
		static inline void AddCycles(int cycles)
		{
#ifdef GENS_ENABLE_EMULATION
			main68k_addCycles(cycles);
#else
			((void)cycles);
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * Exec(): Execute instructions for a given number of cycles.
		 * @param n Number of cycles to execute.
		 * @return ???
		 */
		static inline unsigned int Exec(int n)
		{
#ifdef GENS_ENABLE_EMULATION
			return main68k_exec(n);
#else
			((void)n);
			return 0;
#endif /* GENS_ENABLE_EMULATION */
		}
		
		/**
		 * TripOdometer(): Clear the M68K odometer.
		 * @return ???
		 */
		static inline unsigned int TripOdometer(void)
		{
#ifdef GENS_ENABLE_EMULATION
			return main68k_tripOdometer();
#else
			return 0;
#endif /* GENS_ENABLE_EMULATION */
		}
		
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
};

}

#endif /* __LIBGENS_CPU_M68K_HPP__ */
