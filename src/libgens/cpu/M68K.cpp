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

#include "M68K.hpp"
#include "M68K_Mem.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

S68000CONTEXT M68K::ms_Context;

// Instruction fetch.
STARSCREAM_PROGRAMREGION M68K::M68K_Fetch[] =
{
#ifdef GENS_ENABLE_EMULATION
	// RAM, including mirrors.
	{0xE00000, 0xE0FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE00000},
	{0xE10000, 0xE1FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE10000},
	{0xE20000, 0xE2FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE20000},
	{0xE30000, 0xE3FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE30000},
	{0xE40000, 0xE4FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE40000},
	{0xE50000, 0xE5FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE50000},
	{0xE60000, 0xE6FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE60000},
	{0xE70000, 0xE7FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE70000},
	{0xE80000, 0xE8FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE80000},
	{0xE90000, 0xE9FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE90000},
	{0xEA0000, 0xEAFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEA0000},
	{0xEB0000, 0xEBFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEB0000},
	{0xEC0000, 0xECFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEC0000},
	{0xED0000, 0xEDFFFF, (unsigned int)&Ram_68k.u8[0] - 0xED0000},
	{0xEE0000, 0xEEFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEE0000},
	{0xEF0000, 0xEFFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEF0000},
	{0xF00000, 0xF0FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF00000},
	{0xF10000, 0xF1FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF10000},
	{0xF20000, 0xF2FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF20000},
	{0xF30000, 0xF3FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF30000},
	{0xF40000, 0xF4FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF40000},
	{0xF50000, 0xF5FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF50000},
	{0xF60000, 0xF6FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF60000},
	{0xF70000, 0xF7FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF70000},
	{0xF80000, 0xF8FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF80000},
	{0xF90000, 0xF9FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF90000},
	{0xFA0000, 0xFAFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFA0000},
	{0xFB0000, 0xFBFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFB0000},
	{0xFC0000, 0xFCFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFC0000},
	{0xFD0000, 0xFDFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFD0000},
	{0xFE0000, 0xFEFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFE0000},
	{0xFF0000, 0xFFFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFF0000},
	
	// The following four entries are available for the various different systems.
	{-1, -1, 0},	// 32
	{-1, -1, 0},	// 33
	{-1, -1, 0},	// 34
	{-1, -1, 0},	// 35
#endif /* GENS_ENABLE_EMULATION */
	
	// Terminator.
	{-1, -1, 0}
};

// M68K Starscream has a hack for RAM mirroring for data read.
STARSCREAM_DATAREGION M68K::M68K_Read_Byte[4] =
{
#ifdef GENS_ENABLE_EMULATION
	{0x000000, 0x3FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x400000, 0xFEFFFF, (void*)M68K_Mem::M68K_RB, NULL},
#endif /* GENS_ENABLE_EMULATION */
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data read.
STARSCREAM_DATAREGION M68K::M68K_Read_Word[4] =
{
#ifdef GENS_ENABLE_EMULATION
	{0x000000, 0x3FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x400000, 0xFEFFFF, (void*)M68K_Mem::M68K_RW, NULL},
#endif /* GENS_ENABLE_EMULATION */
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data write.
STARSCREAM_DATAREGION M68K::M68K_Write_Byte[3] =
{
#ifdef GENS_ENABLE_EMULATION
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x000000, 0xFEFFFF, (void*)M68K_Mem::M68K_WB, NULL},
#endif /* GENS_ENABLE_EMULATION */
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data write.
STARSCREAM_DATAREGION M68K::M68K_Write_Word[3] =
{
#ifdef GENS_ENABLE_EMULATION
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x000000, 0xFEFFFF, (void*)M68K_Mem::M68K_WW, NULL},
#endif /* GENS_ENABLE_EMULATION */
	{-1, -1, NULL, NULL}
};


/**
 * M68K_Reset_Handler(): Reset handler.
 * TODO: What does this function do?
 */
void M68K::M68K_Reset_Handler(void)
{
	//Init_Memory_M68K(GENESIS);
}


/**
 * Init(): Initialize the M68K CPU emulator.
 */
void M68K::Init(void)
{
	// Clear the 68000 context.
	memset(&ms_Context, 0x00, sizeof(ms_Context));
	
	// Initialize the memory handlers.
	ms_Context.s_fetch = ms_Context.u_fetch =
		ms_Context.fetch = M68K_Fetch;
	
	ms_Context.s_readbyte = ms_Context.u_readbyte =
		ms_Context.readbyte = M68K_Read_Byte;
	
	ms_Context.s_readword = ms_Context.u_readword =
		ms_Context.readword = M68K_Read_Word;
	
	ms_Context.s_writebyte = ms_Context.u_writebyte =
		ms_Context.writebyte = M68K_Write_Byte;
	
	ms_Context.s_writeword = ms_Context.u_writeword =
		ms_Context.writeword = M68K_Write_Word;
	
	ms_Context.resethandler = M68K_Reset_Handler;
	
#ifdef GENS_ENABLE_EMULATION
	// Set up the main68k context.
	main68k_SetContext(&ms_Context);
	main68k_init();
#endif /* GENS_ENABLE_EMULATION */
}


/**
 * End(): Shut down the M68K CPU emulator.
 */
void M68K::End(void)
{
	// TODO
}


/**
 * InitSys(): Initialize a specific system for the M68K CPU emulator.
 * @param system System ID.
 */
void M68K::InitSys(SysID system)
{
	// TODO: This is not 64-bit clean!
	
	// Clear M68K RAM.
	memset(Ram_68k.u8, 0x00, sizeof(Ram_68k.u8));
	
#ifdef GENS_ENABLE_EMULATION
	// Set the ROM fetch.
	M68K_Fetch[32].lowaddr  = 0x000000;
	M68K_Fetch[32].highaddr = (M68K_Mem::Rom_Size - 1);
	M68K_Fetch[32].offset   = (unsigned int)(&M68K_Mem::Rom_Data.u8[0]) - 0x000000;
	
	switch (system)
	{
		case SYSID_MD:
			// Sega Genesis / Mega Drive.
			// Nothing else is required. Terminate the list.
			M68K_Fetch[33].lowaddr  = -1;
			M68K_Fetch[33].highaddr = -1;
			M68K_Fetch[33].offset   = (unsigned int)NULL;
			break;
		
		case SYSID_MCD:
			// Sega CD.
			// TODO
#if 0
			Bank_M68K = 0;
			
			// Set up Word RAM. (Entry 33)
			MS68K_Set_Word_Ram();
			
			// Set up Program RAM. (Entry 34)
			M68K_Fetch[34].lowaddr = 0x020000;
			M68K_Fetch[34].highaddr = 0x03FFFF;
			M68K_Set_Prg_Ram();
			
			// Terminate the list.
			M68K_Fetch[35].lowaddr = -1;
			M68K_Fetch[35].highaddr = -1;
			M68K_Fetch[35].offset = (unsigned int)NULL;
#endif
			break;
		
		case SYSID_32X:
			// Sega 32X.
			// TODO
#if 0
			Bank_SH2 = 0;
			
			// Nothing else is required. Terminate the list.
			M68K_Fetch[33].lowaddr  = -1;
			M68K_Fetch[33].highaddr = -1;
			M68K_Fetch[33].offset   = (unsigned int)NULL;
#endif
			break;
		
		default:
			break;
	}
	
	// Reset the M68K CPU.
	main68k_reset();
#endif /* GENS_ENABLE_EMULATION */
	
	// Initialize the M68K memory handlers.
	M68K_Mem::InitSys(system);
}


/** ZOMG savestate functions. **/


/**
 * ZomgSaveReg(): Save the M68K registers.
 * @param state Zomg_M68KRegSave_t struct to save to.
 */
void M68K::ZomgSaveReg(Zomg_M68KRegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.
	
#ifdef GENS_ENABLE_EMULATION
	struct S68000CONTEXT m68k_context;
	main68k_GetContext(&m68k_context);
	for (int i = 0; i < 8; i++)
	{
		state->areg[i] = m68k_context.areg[i];
		state->dreg[i] = m68k_context.dreg[i];
	}
	state->asp = m68k_context.asp;
	state->pc = m68k_context.pc;
	state->sr = m68k_context.sr;
#else
	memset(state, 0x00, sizeof(*state));
#endif /* GENS_ENABLE_EMULATION */
}


/**
 * ZomgRestoreReg(): Restore the M68K registers.
 * @param state Zomg_M68KRegSave_t struct to restore from.
 */
void M68K::ZomgRestoreReg(const Zomg_M68KRegSave_t *state)
{
#ifdef GENS_ENABLE_EMULATION
	main68k_GetContext(&ms_Context);
	for (int i = 0; i < 8; i++)
	{
		ms_Context.areg[i] = state->areg[i];
		ms_Context.dreg[i] = state->dreg[i];
	}
	ms_Context.asp = state->asp;
	ms_Context.pc = state->pc;
	ms_Context.sr = state->sr;
	main68k_SetContext(&ms_Context);
#endif /* GENS_ENABLE_EMULATION */
}

}
