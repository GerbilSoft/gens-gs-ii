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

#include "macros/common.h"
#include "Cartridge/RomCartridgeMD.hpp"

// C includes. (C++ namespace)
#include <cstring>

namespace LibGens {

S68000CONTEXT M68K::ms_Context;

// Instruction fetch.
STARSCREAM_PROGRAMREGION M68K::M68K_Fetch[] =
{
#ifdef GENS_ENABLE_EMULATION
	// 32 entries for RAM, including mirrors.
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},

	// 64 entries for ROM handlers.
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
	{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0},
#endif /* GENS_ENABLE_EMULATION */
	
	// Terminator.
	{-1, -1, 0}
};

// M68K Starscream has a hack for RAM mirroring for data read.
STARSCREAM_DATAREGION M68K::M68K_Read_Byte[4] =
{
#ifdef GENS_ENABLE_EMULATION
	// TODO: 0x9FFFFF is valid for MD only.
	{0x000000, 0x9FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0xA00000, 0xFEFFFF, (void*)M68K_Mem::M68K_RB, NULL},
#endif /* GENS_ENABLE_EMULATION */
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data read.
STARSCREAM_DATAREGION M68K::M68K_Read_Word[4] =
{
#ifdef GENS_ENABLE_EMULATION
	// TODO: 0x9FFFFF is valid for MD only.
	{0x000000, 0x9FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0xA00000, 0xFEFFFF, (void*)M68K_Mem::M68K_RW, NULL},
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

// Last system ID.
M68K::SysID M68K::ms_LastSysID = SYSID_NONE;

/**
 * Reset handler.
 * TODO: What does this function do?
 */
void M68K::M68K_Reset_Handler(void)
{
	//Init_Memory_M68K(GENESIS);
}

/**
 * Initialize the M68K CPU emulator.
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
 * Shut down the M68K CPU emulator.
 */
void M68K::End(void)
{
	// TODO
}

/**
 * Initialize a specific system for the M68K CPU emulator.
 * @param system System ID.
 */
void M68K::InitSys(SysID system)
{
	// TODO: This is not 64-bit clean!
	ms_LastSysID = system;

	// Clear M68K RAM.
	memset(Ram_68k.u8, 0x00, sizeof(Ram_68k.u8));

	// Initialize the M68K memory handlers.
	M68K_Mem::InitSys(system);

#ifdef GENS_ENABLE_EMULATION
	// Initialize M68K RAM handlers.
	for (int i = 0; i < 32; i++) {
		uint32_t ram_addr = (0xE00000 | (i << 16));
		M68K_Fetch[i].lowaddr = ram_addr;
		M68K_Fetch[i].highaddr = (ram_addr | 0xFFFF);
		M68K_Fetch[i].offset = ((uint32_t)(&Ram_68k.u8[0]) - ram_addr);
	}

	// Update the system-specific banking setup.
	UpdateSysBanking();

	// Reset the M68K CPU.
	main68k_reset();
#endif /* GENS_ENABLE_EMULATION */
}

/**
 * Shut down M68K emulation.
 */
void M68K::EndSys(void)
{
	for (int i = 0; i < ARRAY_SIZE(M68K_Fetch); i++) {
		M68K_Fetch[i].lowaddr = -1;
		M68K_Fetch[i].highaddr = -1;
		M68K_Fetch[i].offset = 0;
	}
}

/**
 * Update system-specific memory banking.
 * Uses the last system initialized via InitSys().
 */
void M68K::UpdateSysBanking(void)
{
	// Start at M68K_Fetch[0x20].
	int cur_fetch = 0x20;
	switch (ms_LastSysID) {
		case SYSID_MD:
			// Sega Genesis / Mega Drive.
			cur_fetch += M68K_Mem::UpdateSysBanking(&M68K_Fetch[cur_fetch], 10);
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

	// Set the terminator.
	M68K_Fetch[cur_fetch].lowaddr = -1;
	M68K_Fetch[cur_fetch].highaddr = -1;
	M68K_Fetch[cur_fetch].offset = 0;

	// FIXME: Make sure Starscream's internal program counter
	// is updated to reflect the updated M68K_Fetch[].
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
	
	// Save the main registers.
	for (int i = 0; i < 8; i++)
		state->dreg[i] = m68k_context.dreg[i];
	for (int i = 0; i < 7; i++)
		state->areg[i] = m68k_context.areg[i];
	
	// Save the stack pointers.
	if (m68k_context.sr & 0x2000) {
		// Supervisor mode.
		// m68k_context.areg[7] == ssp
		// m68k_context.asp     == usp
		state->ssp = m68k_context.areg[7];
		state->usp = m68k_context.asp;
	} else {
		// User mode.
		// m68k_context.areg[7] == usp
		// m68k_context.asp     == ssp
		state->ssp = m68k_context.asp;
		state->usp = m68k_context.areg[7];
	}

	// Other registers.
	state->pc = m68k_context.pc;
	state->sr = m68k_context.sr;

	// Reserved fields.
	state->reserved1 = 0;
	state->reserved2 = 0;
#else
	memset(state, 0x00, sizeof(*state));
#endif /* GENS_ENABLE_EMULATION */
}


/**
 * Restore the M68K registers.
 * @param state Zomg_M68KRegSave_t struct to restore from.
 */
void M68K::ZomgRestoreReg(const Zomg_M68KRegSave_t *state)
{
#ifdef GENS_ENABLE_EMULATION
	main68k_GetContext(&ms_Context);

	// Load the main registers.
	for (int i = 0; i < 8; i++)
		ms_Context.dreg[i] = state->dreg[i];
	for (int i = 0; i < 7; i++)
		ms_Context.areg[i] = state->areg[i];

	// Load the stack pointers.
	if (ms_Context.sr & 0x2000) {
		// Supervisor mode.
		// ms_Context.areg[7] == ssp
		// ms_Context.asp     == usp
		ms_Context.areg[7] = state->ssp;
		ms_Context.asp     = state->usp;
	} else {
		// User mode.
		// ms_Context.areg[7] == usp
		// ms_Context.asp     == ssp
		ms_Context.asp     = state->ssp;
		ms_Context.areg[7] = state->usp;
	}

	// Other registers.
	ms_Context.pc = state->pc;
	ms_Context.sr = state->sr;

	main68k_SetContext(&ms_Context);
#endif /* GENS_ENABLE_EMULATION */
}

}
