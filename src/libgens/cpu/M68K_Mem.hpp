/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K_Mem.hpp: Main 68000 memory handler.                                *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __LIBGENS_CPU_M68K_MEM_HPP__
#define __LIBGENS_CPU_M68K_MEM_HPP__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// TODO: Starscream accesses Ram_68k directly.
// Move Ram_68k back to M68K once Starscream is updated.
typedef union
{
	uint8_t  u8[64*1024];
	uint16_t u16[(64*1024)>>1];
	uint32_t u32[(64*1024)>>2];
} Ram_68k_t;
extern Ram_68k_t Ram_68k;
#ifdef __cplusplus
}
#endif

#include "M68K.hpp"
#include "../MD/SysVersion.hpp"

// ZOMG TIME_reg structs.
#include "libzomg/zomg_md_time_reg.h"

namespace LibGens
{

class M68K_Mem
{
	public:
		static void Init(void);
		static void End(void);
		
#if 0
		// TODO: Starscream accesses Ram_68k directly.
		// Move Ram_68k back here once Starscream is updated.
		union Ram_68k_t
		{
			uint8_t  u8[64*1024];
			uint16_t u16[(64*1024)>>1];
			uint32_t u32[(64*1024)>>2];
		};
		static Ram_68k_t Ram_68k;
#endif
		
		union Rom_Data_t
		{
			uint8_t  u8[6*1024*1024];
			uint16_t u16[(6*1024*1024)>>1];
			uint32_t u32[(6*1024*1024)>>2];
		};
		static Rom_Data_t Rom_Data;
		static unsigned int Rom_Size;
		
		// Genesis TMSS ROM.
		static uint8_t MD_TMSS_Rom[2 * 1024];
		
		/** Z80 state. **/
		#define Z80_STATE_ENABLED	(1 << 0)
		#define Z80_STATE_BUSREQ	(1 << 1)
		#define Z80_STATE_RESET		(1 << 2)
		
		static unsigned int Z80_State;
		static int Last_BUS_REQ_Cnt;
		static int Last_BUS_REQ_St;
		static int Bank_M68K; // NOTE: This is for Sega CD, not Z80!
		static int Fake_Fetch;
		
		static int CPL_M68K;
		static int CPL_Z80;
		static int Cycles_M68K;
		static int Cycles_Z80;
		
		// TODO: Move ms_SysVersion somewhere else?
		static SysVersion ms_SysVersion;
		
		/** Public init and read/write functions. **/
		static void InitSys(M68K::SysID system);
		static uint8_t M68K_RB(uint32_t address);
		static uint16_t M68K_RW(uint32_t address);
		static void M68K_WB(uint32_t address, uint8_t data);
		static void M68K_WW(uint32_t address, uint16_t data);
		
		/** ZOMG savestate functions. */
		static void ZomgSaveSSF2BankState(Zomg_MD_TimeReg_t *state);
		static void ZomgRestoreSSF2BankState(const Zomg_MD_TimeReg_t *state);
	
	private:
		/** Z80/M68K cycle table. **/
		static int Z80_M68K_Cycle_Tab[512];
		
		/** Bus acquisition timing. **/
		static const int CYCLE_FOR_TAKE_Z80_BUS_GENESIS = 16;
		
		/**
		 * ms_SSF2_BankState[]: SSF2 bankswitching state.
		 * TODO: Make a helper class for this?
		 * Index 0 == unused. (present for alignment and consistency)
		 * Index 1 == $A130F3 (bank 1)
		 * Index 2 == $A130F5 (bank 2)
		 * Index 3 == $A130F7 (bank 3)
		 * Index 4 == $A130F9 (bank 4)
		 * Index 5 == $A130FB (bank 5)
		 * Index 6 == $A130FD (bank 6)
		 * Index 7 == $A130FF (bank 7)
		 *
		 * Values: 0xFF == no bankswitching; 0-63 == bank number
		 * NOTE: Gens/GS II only implements banks 0-9.
		 */
		static uint8_t ms_SSF2_BankState[8];
		
		/**
		 * ms_RomData_ptrs[]: ROM data pointers.
		 * Used with SSF2 bankswitching.
		 * Each pointer refers to a 512 KB page of ROM.
		 * 
		 * NOTE: Gens/GS II only implements banks 0-9.
		 */
		static uint8_t *ms_RomData_ptrs[10];
		
		/** Read Byte functions. **/
		static uint8_t M68K_Read_Byte_Rom(uint32_t address);
		
		template<uint8_t bank>
		static uint8_t T_M68K_Read_Byte_RomX_SRam(uint32_t address);
		
		static uint8_t M68K_Read_Byte_Ram(uint32_t address);
		static uint8_t M68K_Read_Byte_Misc(uint32_t address);
		static uint8_t M68K_Read_Byte_VDP(uint32_t address);
		
		/** Read Word functions. **/
		static uint16_t M68K_Read_Word_Rom(uint32_t address);
		
		template<uint8_t bank>
		static uint16_t T_M68K_Read_Word_RomX_SRam(uint32_t address);
		
		static uint16_t M68K_Read_Word_Ram(uint32_t address);
		static uint16_t M68K_Read_Word_Misc(uint32_t address);
		static uint16_t M68K_Read_Word_VDP(uint32_t address);
		
		/** Write Byte functions. **/
		static void M68K_Write_Byte_SRam(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Ram(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Misc(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_VDP(uint32_t address, uint8_t data);
		
		/** Write Word functions. **/
		static void M68K_Write_Word_SRam(uint32_t address, uint16_t data);
		static void M68K_Write_Word_Ram(uint32_t address, uint16_t data);
		static void M68K_Write_Word_Misc(uint32_t address, uint16_t data);
		static void M68K_Write_Word_VDP(uint32_t address, uint16_t data);
};

}

#endif /* __LIBGENS_CPU_M68K_MEM_HPP__ */
