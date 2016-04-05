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
#include "M68K_Mem.hpp"

#include "cz80/cz80_context.h"
#include "cz80/cz80_flags.h"

// C includes.
#include <string.h>
#include <stdlib.h>

namespace LibGens {

/**
 * Initialize the Z80 CPU emulator.
 */
Z80::Z80()
	: m_cycleCnt(0)
{
	// Allocate the Z80 context.
	// TODO: Error handling.
	// FIXME: Cz80_alloc(), Cz80_free()?
	m_z80 = (cz80_struc*)malloc(sizeof(*m_z80));
	Cz80_Init(m_z80);

	// Set the Cz80 context pointer.
	Cz80_Set_Ctx(m_z80, this);

	// FIXME: This is MD only.

	// Set instruction fetch handlers.
	Cz80_Set_Fetch(m_z80, 0x0000, 0x1FFF, &m_ramZ80[0]);
	Cz80_Set_Fetch(m_z80, 0x2000, 0x3FFF, &m_ramZ80[0]);
	// TODO: Add insn fetch for the 68000 banking area?

	// Set memory read/write handlers.
	Cz80_Set_ReadB(m_z80, Z80_MD_ReadB_static);
	Cz80_Set_WriteB(m_z80, Z80_MD_WriteB_static);

	// Reinitialize the Z80.
	reinit();
}

/**
 * Shut down the Z80 CPU emulator.
 */
Z80::~Z80()
{
	// Free the Z80 context.
	// FIXME: Cz80_free()
	free(m_z80);
	m_z80 = nullptr;
}

/**
 * Reinitialize the Z80 CPU.
 */
void Z80::reinit(void)
{
	// Clear Z80 memory.
	memset(m_ramZ80, 0x00, sizeof(m_ramZ80));

	// Reset the M68K banking register.
	// TODO: 0xFF8000 or 0x000000?
	//m_bankZ80 = 0x000000;
	m_bankZ80 = 0xFF8000;

	// Disable the Z80 initially.
	// NOTE: Bit 0 is used for the "Sound, Z80" option.
	M68K_Mem::Z80_State &= Z80_STATE_ENABLED;

	// Reset the BUSREQ variables.
	M68K_Mem::Last_BUS_REQ_Cnt = 0;
	M68K_Mem::Last_BUS_REQ_St = 0;

	// Hard-reset the Z80.
	hardReset();
}

/** ZOMG savestate functions. **/

/**
 * Save the Z80 registers.
 * @param state Zomg_Z80RegSave_t struct to save to.
 */
void Z80::zomgSaveReg(Zomg_Z80RegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.

	// Main register set.
	state->AF = Cz80_Get_AF(m_z80);
	state->BC = Cz80_Get_BC(m_z80);
	state->DE = Cz80_Get_DE(m_z80);
	state->HL = Cz80_Get_HL(m_z80);
	state->IX = Cz80_Get_IX(m_z80);
	state->IY = Cz80_Get_IY(m_z80);
	state->PC = Cz80_Get_PC(m_z80);
	state->SP = Cz80_Get_SP(m_z80);

	// Shadow register set.
	state->AF2 = Cz80_Get_AF2(m_z80);
	state->BC2 = Cz80_Get_BC2(m_z80);
	state->DE2 = Cz80_Get_DE2(m_z80);
	state->HL2 = Cz80_Get_HL2(m_z80);

	// Other registers.
	state->IFF = Cz80_Get_IFF(m_z80);
	state->R = Cz80_Get_R(m_z80);
	state->I = Cz80_Get_I(m_z80);
	state->IM = Cz80_Get_IM(m_z80);
	state->WZ = Cz80_Get_WZ(m_z80);

	// Status.
	// FIXME: Add Cz80_Get_Status() wrapper.
	//uint8_t z80_status = Cz80_Get_Status(m_z80);
	uint8_t z80_status = m_z80->Status;
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
	//state->IntVect = Cz80_Get_IntVect(m_z80);
	state->IntVect = m_z80->IntVect;
}

/**
 * Restore the Z80 registers.
 * @param state Zomg_Z80RegSave_t struct to restore from.
 */
void Z80::zomgRestoreReg(const Zomg_Z80RegSave_t *state)
{
	// NOTE: Byteswapping is done in libzomg.

	// Main register set.
	Cz80_Set_AF(m_z80, state->AF);
	Cz80_Set_BC(m_z80, state->BC);
	Cz80_Set_DE(m_z80, state->DE);
	Cz80_Set_HL(m_z80, state->HL);
	Cz80_Set_IX(m_z80, state->IX);
	Cz80_Set_IY(m_z80, state->IY);
	Cz80_Set_PC(m_z80, state->PC);
	Cz80_Set_SP(m_z80, state->SP);

	// Shadow register set.
	Cz80_Set_AF2(m_z80, state->AF2);
	Cz80_Set_BC2(m_z80, state->BC2);
	Cz80_Set_DE2(m_z80, state->DE2);
	Cz80_Set_HL2(m_z80, state->HL2);

	// Other registers.
	Cz80_Set_IFF(m_z80, state->IFF);
	Cz80_Set_R(m_z80, state->R);
	Cz80_Set_I(m_z80, state->I);
	Cz80_Set_IM(m_z80, state->IM);
	Cz80_Set_WZ(m_z80, state->WZ);

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
	//Cz80_Set_Status(m_z80, mdZ80_status);
	m_z80->Status = z80_status;

	// Interrupt Vector. (IM 2)
	// TODO: Cz80_Set_IntVect() wrapper.
	//Cz80_Set_IntVect(m_z80, state->IntVect);
	m_z80->IntVect = state->IntVect;
}

}
