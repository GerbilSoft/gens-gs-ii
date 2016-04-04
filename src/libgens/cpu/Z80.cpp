/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K.hpp: Main 68000 CPU wrapper class.                                 *
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

#include <config.libgens.h>

#include "Z80.hpp"
#include "Z80_MD_Mem.hpp"
#include "M68K_Mem.hpp"

#include "cz80/cz80_context.h"
#include "cz80/cz80_flags.h"

// C includes.
#include <string.h>
#include <stdlib.h>

namespace LibGens {

// Static class variables.
cz80_struc *Z80::ms_Z80 = nullptr;
int Z80::ms_cycleCnt = 0;

/**
 * Initialize the Z80 CPU emulator.
 */
void Z80::Init(void)
{
	// Allocate the Z80 context.
	// TODO: Error handling.
	// FIXME: Cz80_alloc(), Cz80_free()?
	ms_Z80 = (cz80_struc*)malloc(sizeof(*ms_Z80));
	Cz80_Init(ms_Z80);

	// TODO: Set context and make this class non-static.
	//Cz80_Set_Ctx(ms_Z80, ???);

	// Set instruction fetch handlers.
	Cz80_Set_Fetch(ms_Z80, 0x0000, 0x1FFF, &Ram_Z80[0]);
	Cz80_Set_Fetch(ms_Z80, 0x2000, 0x3FFF, &Ram_Z80[0]);
	// TODO: Add insn fetch for the 68000 banking area?

	// Set memory read/write handlers.
	Cz80_Set_ReadB(ms_Z80, Z80_MD_Mem::Z80_ReadB);
	Cz80_Set_WriteB(ms_Z80, Z80_MD_Mem::Z80_WriteB);

	// Reinitialize the Z80.
	ReInit();
}

/**
 * Shut down the Z80 CPU emulator.
 */
void Z80::End(void)
{
	// Free the Z80 context.
	// FIXME: Cz80_free()
	free(ms_Z80);
	ms_Z80 = nullptr;

	// TODO: Other shutdown stuff.
}

/**
 * Reinitialize the Z80 CPU.
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
 * Save the Z80 registers.
 * @param state Zomg_Z80RegSave_t struct to save to.
 */
void Z80::ZomgSaveReg(Zomg_Z80RegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.

	// Main register set.
	state->AF = Cz80_Get_AF(ms_Z80);
	state->BC = Cz80_Get_BC(ms_Z80);
	state->DE = Cz80_Get_DE(ms_Z80);
	state->HL = Cz80_Get_HL(ms_Z80);
	state->IX = Cz80_Get_IX(ms_Z80);
	state->IY = Cz80_Get_IY(ms_Z80);
	state->PC = Cz80_Get_PC(ms_Z80);
	state->SP = Cz80_Get_SP(ms_Z80);

	// Shadow register set.
	state->AF2 = Cz80_Get_AF2(ms_Z80);
	state->BC2 = Cz80_Get_BC2(ms_Z80);
	state->DE2 = Cz80_Get_DE2(ms_Z80);
	state->HL2 = Cz80_Get_HL2(ms_Z80);

	// Other registers.
	state->IFF = Cz80_Get_IFF(ms_Z80);
	state->R = Cz80_Get_R(ms_Z80);
	state->I = Cz80_Get_I(ms_Z80);
	state->IM = Cz80_Get_IM(ms_Z80);

	// TODO: Remove this once we switch to CZ80,
	// since CZ80 supports WZ.
	state->WZ = 0;

	// Status.
	// FIXME: Add Cz80_Get_Status() wrapper.
	//uint8_t z80_status = Cz80_Get_Status(ms_Z80);
	uint8_t z80_status = ms_Z80->Status;
	uint8_t zomg_status = 0;
	if (z80_status & CZ80_HALTED) {
		zomg_status |= ZOMG_Z80_STATUS_HALTED;
	}
	if (z80_status & CZ80_FAULTED) {
		zomg_status |= ZOMG_Z80_STATUS_FAULTED;
	}
	// Interrupt status.
	if (z80_status & CZ80_HAS_INT) {
		zomg_status |= ZOMG_Z80_STATUS_INT_PENDING;
	}
	if (z80_status & CZ80_HAS_NMI) {
		zomg_status |= ZOMG_Z80_STATUS_NMI_PENDING;
	}
	state->Status = zomg_status;

	// TODO: Cz80_Get_IntVect() wrapper.
	//state->IntVect = Cz80_Get_IntVect(ms_Z80);
	state->IntVect = ms_Z80->IntVect;
}

/**
 * Restore the Z80 registers.
 * @param state Zomg_Z80RegSave_t struct to restore from.
 */
void Z80::ZomgRestoreReg(const Zomg_Z80RegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.

	// Main register set.
	Cz80_Set_AF(ms_Z80, state->AF);
	Cz80_Set_BC(ms_Z80, state->BC);
	Cz80_Set_DE(ms_Z80, state->DE);
	Cz80_Set_HL(ms_Z80, state->HL);
	Cz80_Set_IX(ms_Z80, state->IX);
	Cz80_Set_IY(ms_Z80, state->IY);
	Cz80_Set_PC(ms_Z80, state->PC);
	Cz80_Set_SP(ms_Z80, state->SP);

	// Shadow register set.
	Cz80_Set_AF2(ms_Z80, state->AF2);
	Cz80_Set_BC2(ms_Z80, state->BC2);
	Cz80_Set_DE2(ms_Z80, state->DE2);
	Cz80_Set_HL2(ms_Z80, state->HL2);

	// Other registers.
	Cz80_Set_IFF(ms_Z80, state->IFF);
	Cz80_Set_R(ms_Z80, state->R);
	Cz80_Set_I(ms_Z80, state->I);
	Cz80_Set_IM(ms_Z80, state->IM);

	// TODO: Load WZ.

	// Status.
	uint8_t z80_status = 0;
	if (state->Status & ZOMG_Z80_STATUS_HALTED) {
		z80_status |= CZ80_HALTED;
	}
	if (state->Status & ZOMG_Z80_STATUS_FAULTED) {
		z80_status |= CZ80_FAULTED;
	}
	// Interrupt lines.
	if (state->Status & ZOMG_Z80_STATUS_INT_PENDING) {
		z80_status |= CZ80_HAS_INT;
	}
	if (state->Status & ZOMG_Z80_STATUS_NMI_PENDING) {
		z80_status |= CZ80_HAS_NMI;
	}

	// FIXME: Add Cz80_Set_Status() wrapper.
	//Cz80_Set_Status(ms_Z80, mdZ80_status);
	ms_Z80->Status = z80_status;

	// Interrupt Vector. (IM 2)
	// TODO: Cz80_Set_IntVect() wrapper.
	//Cz80_Set_IntVect(ms_Z80, state->IntVect);
	ms_Z80->IntVect = state->IntVect;
}

}
