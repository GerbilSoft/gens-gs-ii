/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K_Mem.hpp: Main 68000 memory handler.                                *
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

#ifndef __LIBGENS_CPU_M68K_MEM_HPP__
#define __LIBGENS_CPU_M68K_MEM_HPP__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// TODO: Starscream accesses Ram_68k directly.
// Move Ram_68k back to M68K once Starscream is updated.
typedef union {
	uint8_t  u8[64*1024];
	uint16_t u16[(64*1024)>>1];
	uint32_t u32[(64*1024)>>2];
} Ram_68k_t;
extern Ram_68k_t Ram_68k;
#ifdef __cplusplus
}
#endif

#include "M68K.hpp"

// ZOMG TIME_reg structs.
#include "libzomg/zomg_md_time_reg.h"

// TMSS register.
#include "libgens/EmuContext/TmssReg.hpp"

namespace LibGens {

class RomCartridgeMD;

class M68K_Mem
{
	public:
		static void Init(void);
		static void End(void);

#if 0
		// TODO: Starscream accesses Ram_68k directly.
		// Move Ram_68k back here once Starscream is updated.
		union Ram_68k_t {
			uint8_t  u8[64*1024];
			uint16_t u16[(64*1024)>>1];
			uint32_t u32[(64*1024)>>2];
		};
		static Ram_68k_t Ram_68k;
#endif
		// ROM cartridge.
		static RomCartridgeMD *ms_RomCartridge;

		/**
		 * TMSS registers.
		 * NOTE: Only effective if system version != 0.
		 */
		static TmssReg tmss_reg;

		/** Z80 state. **/
		#define Z80_STATE_ENABLED	(1 << 0)
		#define Z80_STATE_BUSREQ	(1 << 1)
		#define Z80_STATE_RESET		(1 << 2)

		static unsigned int Z80_State;
		static int Last_BUS_REQ_Cnt;
		static int Last_BUS_REQ_St;
		static int Bank_M68K; // NOTE: This is for Sega CD, not Z80!
		static int Fake_Fetch;

		// Cycles per line.
		// TODO: Replace with 3420 machine cycles per line.
		static int CPL_M68K;
		static int CPL_Z80;
		static int Cycles_M68K;
		static int Cycles_Z80;

		/** System initialization functions. **/
	public:
		static void UpdateTmssMapping(void);	// FIXME: Needs to be private?
		static void InitSys(M68K::SysID system);

		/**
		 * Update M68K CPU program access structs for bankswitching purposes.
		 * @param M68K_Fetch Pointer to first STARSCREAM_PROGRAMREGION to update.
		 * @param banks Maximum number of banks to update.
		 * @return Number of banks updated.
		 */
		static int UpdateSysBanking(STARSCREAM_PROGRAMREGION *M68K_Fetch, int banks);

		/** Public read/write functions. **/
		static uint8_t M68K_RB(uint32_t address);
		static uint16_t M68K_RW(uint32_t address);
		static void M68K_WB(uint32_t address, uint8_t data);
		static void M68K_WW(uint32_t address, uint16_t data);
		
	private:
		/** Z80/M68K cycle table. **/
		static int Z80_M68K_Cycle_Tab[512];

		/** Bus acquisition timing. **/
		static const int CYCLE_FOR_TAKE_Z80_BUS_GENESIS = 16;

		// Main 68000 bank IDs.
		enum M68KBank_t {
			// ROM cartridge.
			M68K_BANK_CARTRIDGE = 0,	// M68K: $000000 - $9FFFFF

			// I/O area. (MD)
			M68K_BANK_MD_IO,	// M68K: $A00000 - $BFFFFF

			// VDP area.
			M68K_BANK_VDP,		// M68K: $C00000 - $DFFFFF (specialized mirroring)
			
			// RAM area.
			M68K_BANK_RAM,		// M68K: $E00000 - $FFFFFF (64K mirroring)

			// TMSS ROM.
			// Only if system version > 0 and $A14101 == 0.
			M68K_BANK_TMSS_ROM,	// M68K: $000000 - $3FFFFF (mirrored every 2 KB)

			// I/O area. (Pico)
			M68K_BANK_PICO_IO,	// M68K: $800000 - $9FFFFF

			// Unused bank. (Return 0xFF)
			M68K_BANK_UNUSED = 0xFF
		};

		/**
		 * M68K bank type identifiers.
		 * These type identifiers indicate what's mapped to each virtual bank.
		 * Banks are 2 MB each, for a total of 8 banks.
		 */
		static uint8_t ms_M68KBank_Type[8];

		/**
		 * Default M68K bank type IDs for MD.
		 */
		static const uint8_t msc_M68KBank_Def_MD[8];

		/**
		 * Default M68K bank type IDs for Pico.
		 */
		static const uint8_t msc_M68KBank_Def_Pico[8];

		/** Read Byte functions. **/
		static uint8_t M68K_Read_Byte_Ram(uint32_t address);
		static uint8_t M68K_Read_Byte_Misc(uint32_t address);
		static uint8_t M68K_Read_Byte_VDP(uint32_t address);
		static uint8_t M68K_Read_Byte_TMSS_Rom(uint32_t address);
		static uint8_t M68K_Read_Byte_Pico_IO(uint32_t address);

		/** Read Word functions. **/
		static uint16_t M68K_Read_Word_Ram(uint32_t address);
		static uint16_t M68K_Read_Word_Misc(uint32_t address);
		static uint16_t M68K_Read_Word_VDP(uint32_t address);
		static uint16_t M68K_Read_Word_TMSS_Rom(uint32_t address);
		static uint16_t M68K_Read_Word_Pico_IO(uint32_t address);

		/** Write Byte functions. **/
		static void M68K_Write_Byte_Ram(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Misc(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_VDP(uint32_t address, uint8_t data);
		static void M68K_Write_Byte_Pico_IO(uint32_t address, uint8_t data);

		/** Write Word functions. **/
		static void M68K_Write_Word_Ram(uint32_t address, uint16_t data);
		static void M68K_Write_Word_Misc(uint32_t address, uint16_t data);
		static void M68K_Write_Word_VDP(uint32_t address, uint16_t data);
		static void M68K_Write_Word_Pico_IO(uint32_t address, uint16_t data);
};

}

#endif /* __LIBGENS_CPU_M68K_MEM_HPP__ */
