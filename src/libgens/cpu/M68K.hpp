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
		
		/**
		 * Reset(): Reset the emulated CPU.
		 */
		static inline void Reset(void) { main68k_reset(); }
		
		/** ZOMG savestate functions. **/
		static void ZomgSaveReg(Zomg_M68KRegSave_t *state);
		static void ZomgRestoreReg(const Zomg_M68KRegSave_t *state);
	
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
