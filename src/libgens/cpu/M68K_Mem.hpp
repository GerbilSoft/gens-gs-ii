/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K_Mem.hpp: Main 68000 memory handler.                                *
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
#include "../Save/SRam.hpp"
#include "../Save/EEPRom.hpp"

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
		
		// SRam/EEPRom enable variables.
		// TODO: Accessor/mutator functions.
		static bool SaveDataEnable;
		
		// SRam.
		// TODO: Make this protected!
		// TODO: Add a function e.g. M68K_Mem::Reset() to reset all memory handling.
		static SRam m_SRam;
		
		// EEPRom.
		// TODO: Make this protected!
		// TODO: Add a function e.g. M68K_Mem::Reset() to reset all memory handling.
		static EEPRom m_EEPRom;
		
		/** Z80 state. **/
		#define Z80_STATE_ENABLED	(1 << 0)
		#define Z80_STATE_BUSREQ	(1 << 1)
		#define Z80_STATE_RESET		(1 << 2)
		
		static unsigned int Z80_State;
		static int Last_BUS_REQ_Cnt;
		static int Last_BUS_REQ_St;
		static int Bank_M68K;
		static int Fake_Fetch;
		
		static int CPL_M68K;
		static int CPL_Z80;
		static int Cycles_M68K;
		static int Cycles_Z80;
		
		static int Game_Mode;
		static int CPU_Mode;
		static int Gen_Mode;
		
		/** Public init and read/write functions. **/
		static void InitSys(M68K::SysID system);
		static uint8_t M68K_RB(uint32_t address);
		static uint16_t M68K_RW(uint32_t address);
		static void M68K_WB(uint32_t address, uint8_t data);
		static void M68K_WW(uint32_t address, uint16_t data);
	
	protected:
		/** Z80/M68K cycle table. **/
		static int Z80_M68K_Cycle_Tab[512];
		
		/** Bus acquisition timing. **/
		static const int CYCLE_FOR_TAKE_Z80_BUS_GENESIS = 16;
		
		/** Read Byte functions. **/
		static uint8_t M68K_Read_Byte_Default(uint32_t address);
		
		template<uint8_t bank>
		static uint8_t T_M68K_Read_Byte_RomX(uint32_t address);
		
		template<uint8_t bank>
		static uint8_t T_M68K_Read_Byte_RomX_SRam(uint32_t address);
		
		static uint8_t M68K_Read_Byte_Ram(uint32_t address);
		static uint8_t M68K_Read_Byte_Misc(uint32_t address);
		static uint8_t M68K_Read_Byte_VDP(uint32_t address);
		
		/** Read Word functions. **/
		static uint16_t M68K_Read_Word_Default(uint32_t address);
		
		template<uint8_t bank>
		static uint16_t T_M68K_Read_Word_RomX(uint32_t address);
		
		template<uint8_t bank>
		static uint16_t T_M68K_Read_Word_RomX_SRam(uint32_t address);
		
		static uint16_t M68K_Read_Word_Ram(uint32_t address);
		static uint16_t M68K_Read_Word_Misc(uint32_t address);
		static uint16_t M68K_Read_Word_VDP(uint32_t address);
		
		/** Write Byte functions. **/
		static void M68K_Write_Byte_Default(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_SRam(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Ram(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Misc(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_VDP(uint32_t address, uint8_t data);
		
		/** Write Word functions. **/
		static void M68K_Write_Word_Default(uint32_t address, uint16_t data);
		static void M68K_Write_Word_SRam(uint32_t address, uint16_t data);
		static void M68K_Write_Word_Ram(uint32_t address, uint16_t data);
		static void M68K_Write_Word_Misc(uint32_t address, uint16_t data);
		static void M68K_Write_Word_VDP(uint32_t address, uint16_t data);
		
		/** Main MC68000 read/write functions. **/
		typedef uint8_t  (*M68K_Read_Byte_fn)(uint32_t address);
		typedef uint16_t (*M68K_Read_Word_fn)(uint32_t address);
		typedef void     (*M68K_Write_Byte_fn)(uint32_t address, uint8_t data);
		typedef void     (*M68K_Write_Word_fn)(uint32_t address, uint16_t data);
		
		/** Main M68K function tables. (512 KB pages; 32 entries.) **/
		static M68K_Read_Byte_fn M68K_Read_Byte_Table[32];
		static M68K_Read_Word_fn M68K_Read_Word_Table[32];
		static M68K_Write_Byte_fn M68K_Write_Byte_Table[32];
		static M68K_Write_Word_fn M68K_Write_Word_Table[32];
		
		/** Default M68K function tables for MD. (512 KB pages; 32 entries.) **/
		static const M68K_Read_Byte_fn MD_M68K_Read_Byte_Table[32];
		static const M68K_Read_Word_fn MD_M68K_Read_Word_Table[32];
		static const M68K_Write_Byte_fn MD_M68K_Write_Byte_Table[32];
		static const M68K_Write_Word_fn MD_M68K_Write_Word_Table[32];
};

}

#endif /* __LIBGENS_CPU_M68K_MEM_HPP__ */
