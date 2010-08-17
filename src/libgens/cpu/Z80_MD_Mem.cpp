/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Z80_MD_Mem.cpp: Z80 memory handler. (Mega Drive mode)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "Z80_MD_Mem.hpp"

// Unused parameter macro.
// TODO: Move somewhere else?
#define UNUSED(x) ((void)x)

// LibGens includes.
#include "M68K_Mem.hpp"
#include "MD/VdpIo.hpp"

// Gens includes.
// TODO
#if 0
#include "mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"
#endif

// TODO: mdZ80 accesses Ram_Z80 directly.
// Move Ram_Z80 back to Z80_MD_Mem once mdZ80 is updated.
uint8_t Ram_Z80[8 * 1024];

namespace LibGens
{

// Static class variables.
int Z80_MD_Mem::Bank_Z80;


void Z80_MD_Mem::Init(void)
{
	// TODO
}


void Z80_MD_Mem::End(void)
{
	// TODO
}


/** Z80 General Read/Write functions. **/


/**
 * Z80_ReadB(): Read a byte from the Z80 address space.
 * @param address Address to read from.
 * @return Byte from the Z80 address space.
 */
uint8_t Z80_MD_Mem::Z80_ReadB(uint32_t address)
{
	unsigned int fn = (address & 0xF000) >> 12;
	return Z80_ReadB_Table[fn](address & 0x7FFF);
}


/**
 * Z80_WriteB(): Write a byte to the Z80 address space.
 * @param address Address to write to.
 * @param data Byte to write to the Z80 address space.
 */
void Z80_MD_Mem::Z80_WriteB(uint32_t address, uint8_t data)
{
	unsigned int fn = (address & 0xF000) >> 12;
	Z80_WriteB_Table[fn](address & 0x7FFF, data);
}


/** Z80 Read Byte functions. **/


/**
 * Z80_ReadB_Bad(): Read a byte from an invalid location in the Z80 address space.
 * @param address Address to read from.
 * @return 0.
 */
uint8_t FASTCALL Z80_MD_Mem::Z80_ReadB_Bad(uint32_t address)
{
	UNUSED(address);
	
	// TODO: Invalid address. This should do something other than return 0.
	return 0;
}


/**
 * Z80_ReadB_Ram(): Read a byte from Z80 RAM.
 * @param address Address to read from.
 * @return Byte from Z80 RAM.
 */
uint8_t FASTCALL Z80_MD_Mem::Z80_ReadB_Ram(uint32_t address)
{
	return (Ram_Z80[address & 0x1FFF]);
}


/**
 * Z80_ReadB_Bank(): Read the Z80's 68K ROM banking register. (INVALID)
 * @param address Address to read from.
 * @return
 */
uint8_t FASTCALL Z80_MD_Mem::Z80_ReadB_Bank(uint32_t address)
{
	UNUSED(address);
	
	// TODO: Invalid address. This should do something other than return 0.
	return 0;
}


/**
 * Z80_ReadB_YM2612(): Read a byte from the YM2612.
 * @param address Address to read from.
 * @return YM2612 register.
 */
uint8_t FASTCALL Z80_MD_Mem::Z80_ReadB_YM2612(uint32_t address)
{
	// According to the Genesis Software Manual, all four addresses return
	// the same value for YM2612_Read().
	UNUSED(address);
	
	// The YM2612's RESET line is tied to the Z80's RESET line.
	// TODO: Determine the correct return value.
	if (M68K_Mem::Z80_State & Z80_STATE_RESET)
		return 0xFF;
	
	// Return the YM2612 status register.
	return M68K_Mem::m_Ym2612.read();
}


/**
 * Z80_ReadB_PSG(): Read a byte from the PSG or VDP.
 * @param address Address to read from.
 * @return PSG register.
 */
uint8_t FASTCALL Z80_MD_Mem::Z80_ReadB_PSG(uint32_t address)
{
	if (address < 0x7F04 || address > 0x7F09)
	{
		// TODO: Invalid address. This should do something other than return 0.
		return 0;
	}
	
	if (address < 0x7F08)
	{
		// VDP status.
		int rval = VdpIo::Read_Status();
		if (address & 1)
			return (rval & 0xFF);
		else
			return ((rval >> 8) & 0xFF);
	}
	else //if (address >= 0x7F08 && address <= 0x7F09)
	{
		// VDP counter.
		if (address & 1)
			return VdpIo::Read_H_Counter();
		else
			return VdpIo::Read_V_Counter();
	}
}


/**
 * Z80_ReadB_68K_Ram(): Read a byte from MC68000 RAM.
 * @param address Address to read from.
 * @return Byte from MC68000 RAM.
 */
uint8_t FASTCALL Z80_MD_Mem::Z80_ReadB_68K_Ram(uint32_t address)
{
	return M68K_Mem::M68K_RB(Bank_Z80 + (address & 0x7FFF));
}


/** Z80 Write Byte functions. **/


/**
 * Z80_WriteB_Bad(): Write a byte to an invalid location in the Z80 address space.
 * @param address Address to write to.
 * @param data Byte to write.
 */
void FASTCALL Z80_MD_Mem::Z80_WriteB_Bad(uint32_t address, uint8_t data)
{
	UNUSED(address);
	UNUSED(data);
	
	// TODO: Invalid address. This should do something.
	return;
}


/**
 * Z80_WriteB_Ram(): Write a byte to Z80 RAM.
 * @param address Address to write to.
 * @param data Byte to write.
 */
void FASTCALL Z80_MD_Mem::Z80_WriteB_Ram(uint32_t address, uint8_t data)
{
	Ram_Z80[address & 0x1FFF] = data;
}


/**
 * Z80_WriteB_Bank(): Shift a bit into the Z80's 68K ROM banking register.
 * @param address Address to write to.
 * @param data Byte to write.
 */
void FASTCALL Z80_MD_Mem::Z80_WriteB_Bank(uint32_t address, uint8_t data)
{
	if (address > 0x60FF)
	{
		// TODO: Invalid address. This should do something.
		return;
	}
	
	uint32_t bank_address = (Bank_Z80 & 0xFF0000) >> 1;
	uint32_t bank_num = (data & 1) << 23;
	bank_address += bank_num;
	
	Bank_Z80 = bank_address;
}


/**
 * Z80_WriteB_YM2612(): Write a byte to the YM2612.
 * @param address Address to write to.
 * @param data Byte to write.
 */
void FASTCALL Z80_MD_Mem::Z80_WriteB_YM2612(uint32_t address, uint8_t data)
{
	// The YM2612's RESET line is tied to the Z80's RESET line.
	if (M68K_Mem::Z80_State & Z80_STATE_RESET)
		return;
	
	// Write to the YM2612.
	M68K_Mem::m_Ym2612.write(address & 0x03, data);
}


/**
 * Z80_WriteB_PSG(): Write a byte to the PSG or VDP.
 * @param address Address to write to.
 * @param data Byte to write.
 */
void FASTCALL Z80_MD_Mem::Z80_WriteB_PSG(uint32_t address, uint8_t data)
{
	// TODO: All VDP registers should be writable here...
	// Z80 [0x7F00, 0x7F1F] maps to 68000 [0xC00000, 0xC0001F].
	
	if (address == 0x7F11)
	{
		// PSG register.
		M68K_Mem::m_Psg.write(data);
		return;
	}
	
	if (address > 0x7F03)
	{
		// TODO: Invalid address. This should do something.
		return;
	}
	
	// VDP register. (TODO: WTF?)
	VdpIo::Write_Data_Byte(data);
}


/**
 * Z80_WriteB_68K_Ram(): Write a byte to MC68000 RAM.
 * @param address Address to write to.
 * @param data Byte to write.
 */
void FASTCALL Z80_MD_Mem::Z80_WriteB_68K_Ram(uint32_t address, uint8_t data)
{
	address &= 0x7FFF;
	address += Bank_Z80;
	
	M68K_Mem::M68K_WB(address, data);
}


/** Default function tables. **/


/**
 * Z80_ReadB_Table[]: Read Byte function table.
 * 4 KB pages; 16 entries.
 */
const Z80_MD_Mem::Z80_ReadB_fn Z80_MD_Mem::Z80_ReadB_Table[16] =
{
	Z80_ReadB_Ram,		// 0x0000 - 0x0FFF
	Z80_ReadB_Ram,		// 0x1000 - 0x1FFF
	Z80_ReadB_Ram,		// 0x2000 - 0x2FFF
	Z80_ReadB_Ram,		// 0x3000 - 0x3FFF
	Z80_ReadB_YM2612,	// 0x4000 - 0x4FFF
	Z80_ReadB_YM2612,	// 0x5000 - 0x5FFF
	Z80_ReadB_Bank,		// 0x6000 - 0x6FFF
	Z80_ReadB_PSG,		// 0x7000 - 0x7FFF
	Z80_ReadB_68K_Ram,	// 0x8000 - 0x8FFF
	Z80_ReadB_68K_Ram,	// 0x9000 - 0x9FFF
	Z80_ReadB_68K_Ram,	// 0xA000 - 0xAFFF
	Z80_ReadB_68K_Ram,	// 0xB000 - 0xBFFF
	Z80_ReadB_68K_Ram,	// 0xC000 - 0xCFFF
	Z80_ReadB_68K_Ram,	// 0xD000 - 0xDFFF
	Z80_ReadB_68K_Ram,	// 0xE000 - 0xEFFF
	Z80_ReadB_68K_Ram,	// 0xF000 - 0xFFFF
};


/**
 * Z80_WriteB_Table[]: Write Byte function table.
 * 4 KB pages; 16 entries.
 */
const Z80_MD_Mem::Z80_WriteB_fn Z80_MD_Mem::Z80_WriteB_Table[16] =
{
	Z80_WriteB_Ram,		// 0x0000 - 0x0FFF
	Z80_WriteB_Ram,		// 0x1000 - 0x1FFF
	Z80_WriteB_Ram,		// 0x2000 - 0x2FFF
	Z80_WriteB_Ram,		// 0x3000 - 0x3FFF
	Z80_WriteB_YM2612,	// 0x4000 - 0x4FFF
	Z80_WriteB_YM2612,	// 0x5000 - 0x5FFF
	Z80_WriteB_Bank,	// 0x6000 - 0x6FFF
	Z80_WriteB_PSG,		// 0x7000 - 0x7FFF
	Z80_WriteB_68K_Ram,	// 0x8000 - 0x8FFF
	Z80_WriteB_68K_Ram,	// 0x9000 - 0x9FFF
	Z80_WriteB_68K_Ram,	// 0xA000 - 0xAFFF
	Z80_WriteB_68K_Ram,	// 0xB000 - 0xBFFF
	Z80_WriteB_68K_Ram,	// 0xC000 - 0xCFFF
	Z80_WriteB_68K_Ram,	// 0xD000 - 0xDFFF
	Z80_WriteB_68K_Ram,	// 0xE000 - 0xEFFF
	Z80_WriteB_68K_Ram,	// 0xF000 - 0xFFFF
};

}
