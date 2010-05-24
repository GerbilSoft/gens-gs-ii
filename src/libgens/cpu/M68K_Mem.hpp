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
#include "M68K.hpp"

namespace LibGens
{

class M68K_Mem
{
	public:
		union Ram_68k_t
		{
			uint8_t  u8[64*1024];
			uint16_t u16[(64*1024)>>1];
			uint32_t u32[(64*1024)>>2];
		};
		static Ram_68k_t Ram_68k;
		
		union Rom_Data_t
		{
			uint8_t  u8[6*1024*1024];
			uint16_t u16[(6*1024*1024)>>1];
			uint32_t u32[(6*1024*1024)>>2];
		};
		static Rom_Data_t Rom_Data;
		
		// Genesis TMSS ROM.
		static uint8_t MD_TMSS_Rom[2 * 1024];
		
		// SRam.
		static uint8_t SRam[64 * 1024];
		static uint32_t SRam_Start;
		static uint32_t SRam_End;
		
		struct SRam_State_t
		{
			uint32_t on      : 1;
			uint32_t write   : 1;
			uint32_t custom  : 1;
			uint32_t enabled : 1;
		};
		static SRam_State_t SRam_State;
		
		static unsigned int Rom_Size;
		
		// Z80/M68K cycle table.
		static int Z80_M68K_Cycle_Tab[512];
		
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
		static void Init(M68K::SysID system);
		static uint8_t M68K_RB(uint32_t address);
		static uint16_t M68K_RW(uint32_t address);
		static void M68K_WB(uint32_t address, uint8_t data);
		static void M68K_WW(uint32_t address, uint16_t data);
	
	protected:
		/** Read Byte functions. **/
		static uint8_t M68K_Read_Byte_Default(uint32_t address);
		
		template<uint8_t bank>
		static uint8_t T_M68K_Read_Byte_RomX(uint32_t address);
		
		static uint8_t M68K_Read_Byte_Rom4(uint32_t address);
		static uint8_t M68K_Read_Byte_Ram(uint32_t address);
		static uint8_t M68K_Read_Byte_Misc(uint32_t address);
		static uint8_t M68K_Read_Byte_VDP(uint32_t address);
		
		/** Read Word functions. **/
		static uint16_t M68K_Read_Word_Default(uint32_t address);
		
		template<uint8_t bank>
		static uint16_t T_M68K_Read_Word_RomX(uint32_t address);
		
		static uint16_t M68K_Read_Word_Rom4(uint32_t address);
		static uint16_t M68K_Read_Word_Ram(uint32_t address);
		static uint16_t M68K_Read_Word_Misc(uint32_t address);
		static uint16_t M68K_Read_Word_VDP(uint32_t address);
		
		/** Write Byte functions. **/
		static void M68K_Write_Byte_Default(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_SRam(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Ram(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Misc(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_VDP(uint32_t address, uint8_t data);
		
		/** Main MC68000 read/write functions. **/
		typedef uint8_t  (*M68K_Read_Byte_fn)(uint32_t address);
		typedef uint16_t (*M68K_Read_Word_fn)(uint32_t address);
		typedef void     (*M68K_Write_Byte_fn)(uint32_t address, uint8_t data);
		typedef void     (*M68K_Write_Word_fn)(uint32_t address, uint8_t data);
		
		/** Main M68K function tables. (512 KB pages; 0x20 entries.) **/
		static M68K_Read_Byte_fn M68K_Read_Byte_Table[0x20];
		static M68K_Read_Word_fn M68K_Read_Word_Table[0x20];
		static M68K_Write_Byte_fn M68K_Write_Byte_Table[0x20];
		static M68K_Write_Word_fn M68K_Write_Word_Table[0x20];
		
		/** Default M68K function tables for MD. **/
		static const M68K_Read_Byte_fn MD_M68K_Read_Byte_Table[0x20];
		static const M68K_Read_Word_fn MD_M68K_Read_Word_Table[0x20];
		static const M68K_Write_Byte_fn MD_M68K_Write_Byte_Table[0x20];
		static const M68K_Write_Word_fn MD_M68K_Write_Word_Table[0x20];
};

}

#endif /* __LIBGENS_CPU_M68K_MEM_HPP__ */
