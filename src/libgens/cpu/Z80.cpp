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
#ifdef GENS_ENABLE_EMULATION
	mdZ80_init(&ms_Z80);
	
	// Set instruction fetch handlers.
	mdZ80_Add_Fetch(&ms_Z80, 0x00, 0x1F, &Ram_Z80[0]);
	mdZ80_Add_Fetch(&ms_Z80, 0x20, 0x3F, &Ram_Z80[0]);
	
	// Set memory read/write handlers.
	mdZ80_Set_ReadB(&ms_Z80, Z80_MD_Mem::Z80_ReadB);
	mdZ80_Set_WriteB(&ms_Z80, Z80_MD_Mem::Z80_WriteB);
#endif
	
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
	state->AF = mdZ80_get_AF(&ms_Z80);
	state->BC = ms_Z80.BC.w.BC;
	state->DE = ms_Z80.DE.w.DE;
	state->HL = ms_Z80.HL.w.HL;
	state->IX = ms_Z80.IX.w.IX;
	state->IY = ms_Z80.IY.w.IY;
	state->PC = mdZ80_get_PC(&ms_Z80);
	state->SP = ms_Z80.SP.w.SP;
	
	// Shadow register set.
	state->AF2 = mdZ80_get_AF2(&ms_Z80);
	state->BC2 = ms_Z80.BC2.w.BC2;
	state->DE2 = ms_Z80.DE2.w.DE2;
	state->HL2 = ms_Z80.HL2.w.HL2;
	
	// Other registers.
	state->IFF = (ms_Z80.IFF & 0x03);
	state->R = ms_Z80.R;
	state->I = ms_Z80.I;
	state->IM = ms_Z80.IM;
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
	mdZ80_set_AF(&ms_Z80, state->AF);
	ms_Z80.BC.w.BC = state->BC;
	ms_Z80.DE.w.DE = state->DE;
	ms_Z80.HL.w.HL = state->HL;
	ms_Z80.IX.w.IX = state->IX;
	ms_Z80.IY.w.IY = state->IY;
	mdZ80_set_PC(&ms_Z80, state->PC);
	ms_Z80.SP.w.SP = state->SP;
	
	// Shadow register set.
	mdZ80_set_AF2(&ms_Z80, state->AF2);
	ms_Z80.BC2.w.BC2 = state->BC2;
	ms_Z80.DE2.w.DE2 = state->DE2;
	ms_Z80.HL2.w.HL2 = state->HL2;
	
	// Other registers.
	ms_Z80.IFF = (state->IFF & 0x03);
	ms_Z80.R = state->R;
	ms_Z80.I = state->I;
	ms_Z80.IM = state->IM;
#endif /* GENS_ENABLE_EMULATION */
}

}
