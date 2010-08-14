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

#include "Z80.hpp"
#include "Z80_MD_Mem.hpp"
#include "M68K_Mem.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

// Static class variables.
Z80_CONTEXT Z80::ms_Z80;


/**
 * Init(): Initialize the Z80 CPU emulator.
 */
void Z80::Init(void)
{
	mdZ80_init(&ms_Z80);
	
	// Set instruction fetch handlers.
	z80_Add_Fetch(&ms_Z80, 0x00, 0x1F, &Ram_Z80[0]);
	z80_Add_Fetch(&ms_Z80, 0x20, 0x3F, &Ram_Z80[0]);
	
	// Set Read Byte handlers.
	z80_Add_ReadB(&ms_Z80, 0x00, 0xFF, Z80_MD_Mem::Z80_ReadB_Bad);
	z80_Add_ReadB(&ms_Z80, 0x00, 0x3F, Z80_MD_Mem::Z80_ReadB_Ram);
	z80_Add_ReadB(&ms_Z80, 0x40, 0x5F, Z80_MD_Mem::Z80_ReadB_YM2612);
	z80_Add_ReadB(&ms_Z80, 0x60, 0x6F, Z80_MD_Mem::Z80_ReadB_Bank);
	z80_Add_ReadB(&ms_Z80, 0x70, 0x7F, Z80_MD_Mem::Z80_ReadB_PSG);
	z80_Add_ReadB(&ms_Z80, 0x80, 0xFF, Z80_MD_Mem::Z80_ReadB_68K_Ram);
	
	// Set Read Word handlers. (DEPRECATED)
	z80_Add_ReadW(&ms_Z80, 0x00, 0xFF, Z80_MD_Mem::Z80_ReadW_Bad);
	z80_Add_ReadW(&ms_Z80, 0x00, 0x3F, Z80_MD_Mem::Z80_ReadW_Ram);
	z80_Add_ReadW(&ms_Z80, 0x40, 0x5F, Z80_MD_Mem::Z80_ReadW_YM2612);
	z80_Add_ReadW(&ms_Z80, 0x60, 0x6F, Z80_MD_Mem::Z80_ReadW_Bank);
	z80_Add_ReadW(&ms_Z80, 0x70, 0x7F, Z80_MD_Mem::Z80_ReadW_PSG);
	z80_Add_ReadW(&ms_Z80, 0x80, 0xFF, Z80_MD_Mem::Z80_ReadW_68K_Ram);
	
	// Set Write Byte handlers.
	z80_Add_WriteB(&ms_Z80, 0x00, 0xFF, Z80_MD_Mem::Z80_WriteB_Bad);
	z80_Add_WriteB(&ms_Z80, 0x00, 0x3F, Z80_MD_Mem::Z80_WriteB_Ram);
	z80_Add_WriteB(&ms_Z80, 0x40, 0x5F, Z80_MD_Mem::Z80_WriteB_YM2612);
	z80_Add_WriteB(&ms_Z80, 0x60, 0x6F, Z80_MD_Mem::Z80_WriteB_Bank);
	z80_Add_WriteB(&ms_Z80, 0x70, 0x7F, Z80_MD_Mem::Z80_WriteB_PSG);
	z80_Add_WriteB(&ms_Z80, 0x80, 0xFF, Z80_MD_Mem::Z80_WriteB_68K_Ram);
	
	// Set Write Word handlers. (DEPRECATED)
	z80_Add_WriteW(&ms_Z80, 0x00, 0xFF, Z80_MD_Mem::Z80_WriteW_Bad);
	z80_Add_WriteW(&ms_Z80, 0x00, 0x3F, Z80_MD_Mem::Z80_WriteW_Ram);
	z80_Add_WriteW(&ms_Z80, 0x40, 0x5F, Z80_MD_Mem::Z80_WriteW_YM2612);
	z80_Add_WriteW(&ms_Z80, 0x60, 0x6F, Z80_MD_Mem::Z80_WriteW_Bank);
	z80_Add_WriteW(&ms_Z80, 0x70, 0x7F, Z80_MD_Mem::Z80_WriteW_PSG);
	z80_Add_WriteW(&ms_Z80, 0x80, 0xFF, Z80_MD_Mem::Z80_WriteW_68K_Ram);
	
	// Reinitialize the Z80.
	ReInit();
}


/**
 * End(): Shut down the Z80 CPU emulator.
 */
void Z80::End(void)
{
	// TODO
}


/**
 * ReInit(): Reinitialize the Z80 CPU.
 */
void Z80::ReInit(void)
{
	// Clear Z80 memory.
	memset(Ram_Z80, 0x00, sizeof(Ram_Z80));
	
	// Reset the M68K banking register.
	// TODO: 0xFF8000 or 0x000000?
	Z80_MD_Mem::Bank_Z80 = 0x000000;
	Z80_MD_Mem::Bank_Z80 = 0xFF8000;
	
	// Disable the Z80 initially.
	// NOTE: Bit 0 is used for the "Sound, Z80" option.
	M68K_Mem::Z80_State &= Z80_STATE_ENABLED;
	
	// Reset the BUSREQ variables.
	M68K_Mem::Last_BUS_REQ_Cnt = 0;
	M68K_Mem::Last_BUS_REQ_St = 0;
	
	// Reset the Z80.
	Reset();
}

}
