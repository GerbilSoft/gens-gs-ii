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
mdZ80_context *Z80::ms_Z80 = NULL;


/**
 * Init(): Initialize the Z80 CPU emulator.
 */
void Z80::Init(void)
{
#ifdef GENS_ENABLE_EMULATION
	// Allocate the Z80 context.
	// TODO: Error handling.
	ms_Z80 = mdZ80_new();
	
	// Set instruction fetch handlers.
	mdZ80_Add_Fetch(ms_Z80, 0x00, 0x1F, &Ram_Z80[0]);
	mdZ80_Add_Fetch(ms_Z80, 0x20, 0x3F, &Ram_Z80[0]);
	
	// Set memory read/write handlers.
	mdZ80_Set_ReadB(ms_Z80, Z80_MD_Mem::Z80_ReadB);
	mdZ80_Set_WriteB(ms_Z80, Z80_MD_Mem::Z80_WriteB);
#endif
	
	// Reinitialize the Z80.
	ReInit();
}


/**
 * End(): Shut down the Z80 CPU emulator.
 */
void Z80::End(void)
{
	// Free the Z80 context.
#ifdef GENS_ENABLE_EMULATION
	mdZ80_free(ms_Z80);
#endif
	ms_Z80 = NULL;
	
	// TODO: Other shutdown stuff.
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
	
	// Hard-reset the Z80.
	HardReset();
}


/** ZOMG savestate functions. **/


/**
 * ZomgSaveReg(): Save the Z80 registers.
 * @param state Zomg_Z80RegSave_t struct to save to.
 */
void Z80::ZomgSaveReg(Zomg_Z80RegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.
	
#ifdef GENS_ENABLE_EMULATION
	// Main register set.
	state->AF = mdZ80_get_AF(ms_Z80);
	state->BC = mdZ80_get_BC(ms_Z80);
	state->DE = mdZ80_get_DE(ms_Z80);
	state->HL = mdZ80_get_HL(ms_Z80);
	state->IX = mdZ80_get_IX(ms_Z80);
	state->IY = mdZ80_get_IY(ms_Z80);
	state->PC = mdZ80_get_PC(ms_Z80);
	state->SP = mdZ80_get_SP(ms_Z80);
	
	// Shadow register set.
	state->AF2 = mdZ80_get_AF2(ms_Z80);
	state->BC2 = mdZ80_get_BC2(ms_Z80);
	state->DE2 = mdZ80_get_DE2(ms_Z80);
	state->HL2 = mdZ80_get_HL2(ms_Z80);
	
	// Other registers.
	state->IFF = mdZ80_get_IFF(ms_Z80);
	state->R = mdZ80_get_R(ms_Z80);
	state->I = mdZ80_get_I(ms_Z80);
	state->IM = mdZ80_get_IM(ms_Z80);
	// TODO: Save IntVect and IntLine.
	// TODO: Save Status.
#else
	memset(state, 0x00, sizeof(*state));
#endif /* GENS_ENABLE_EMULATION */
}


/**
 * ZomgRestoreReg(): Restore the Z80 registers.
 * @param state Zomg_Z80RegSave_t struct to restore from.
 */
void Z80::ZomgRestoreReg(const Zomg_Z80RegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.
	
#ifdef GENS_ENABLE_EMULATION
	// Main register set.
	mdZ80_set_AF(ms_Z80, state->AF);
	mdZ80_set_BC(ms_Z80, state->BC);
	mdZ80_set_DE(ms_Z80, state->DE);
	mdZ80_set_HL(ms_Z80, state->HL);
	mdZ80_set_IX(ms_Z80, state->IX);
	mdZ80_set_IY(ms_Z80, state->IY);
	mdZ80_set_PC(ms_Z80, state->PC);
	mdZ80_set_SP(ms_Z80, state->SP);
	
	// Shadow register set.
	mdZ80_set_AF2(ms_Z80, state->AF2);
	mdZ80_set_BC2(ms_Z80, state->BC2);
	mdZ80_set_DE2(ms_Z80, state->DE2);
	mdZ80_set_HL2(ms_Z80, state->HL2);
	
	// Other registers.
	mdZ80_set_IFF(ms_Z80, state->IFF);
	mdZ80_set_R(ms_Z80, state->R);
	mdZ80_set_I(ms_Z80, state->I);
	mdZ80_set_IM(ms_Z80, state->IM);
	// TODO: Load IntVect and IntLine.
	// TODO: Load Status.
#endif /* GENS_ENABLE_EMULATION */
}

}
