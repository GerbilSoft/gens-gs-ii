/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_reg.c: Z80 register access functions.                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2011 by David Korth                                  *
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

#include "mdZ80.h"

// C includes.
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

// Z80 context definition.
#include "mdZ80_context.h"

// Z80 flag and state definitions.
#include "mdZ80_flags.h"

/*! Z80 register get functions. **/

#define Z80_GET_REGISTER_FUNC(reg_type, reg_name, reg_access) \
reg_type mdZ80_get_##reg_name(mdZ80_context *z80) \
{ \
	if (z80->Status & Z80_STATE_RUNNING) \
		return -1; \
	return z80->reg_access; \
}

uint16_t mdZ80_get_AF(mdZ80_context *z80)
{
	unsigned char F;
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;

	// F register.
	// The X and Y flags are stored separately from the
	// rest of the flags for some reason.
	F = (z80->AF.b.F & ~(Z80_FLAG_X | Z80_FLAG_Y)) |
	    (z80->AF.b.FXY & (Z80_FLAG_X | Z80_FLAG_Y));

	// Return AF.
	return ((z80->AF.b.A << 8) | F);
}

Z80_GET_REGISTER_FUNC(uint16_t, BC, BC.w)
Z80_GET_REGISTER_FUNC(uint16_t, DE, DE.w)
Z80_GET_REGISTER_FUNC(uint16_t, HL, HL.w)
Z80_GET_REGISTER_FUNC(uint16_t, IX, IX.w)
Z80_GET_REGISTER_FUNC(uint16_t, IY, IY.w)

uint16_t mdZ80_get_PC(mdZ80_context *z80)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;
	
	// Subtract the BasePC from PC to get the actual Z80 program counter.
	return (uint16_t)(z80->PC - z80->BasePC);
}

Z80_GET_REGISTER_FUNC(uint16_t, SP, SP.w)

uint16_t mdZ80_get_AF2(mdZ80_context *z80)
{
	unsigned char F2;
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;

	// F' register.
	// The X and Y flags are stored separately from the
	// rest of the flags for some reason.
	F2 = (z80->AF2.b.F2 & ~(Z80_FLAG_X | Z80_FLAG_Y)) |
	     (z80->AF2.b.FXY2 & (Z80_FLAG_X | Z80_FLAG_Y));

	// Return AF'.
	return ((z80->AF2.b.A2 << 8) | F2);
}

Z80_GET_REGISTER_FUNC(uint16_t, BC2, BC2)
Z80_GET_REGISTER_FUNC(uint16_t, DE2, DE2)
Z80_GET_REGISTER_FUNC(uint16_t, HL2, HL2)

uint8_t mdZ80_get_IFF(mdZ80_context *z80)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;
	return (z80->IFF & 3);
}

Z80_GET_REGISTER_FUNC(uint8_t, R, R)
Z80_GET_REGISTER_FUNC(uint8_t, I, I)
Z80_GET_REGISTER_FUNC(uint8_t, IM, IM)
Z80_GET_REGISTER_FUNC(uint8_t, IntVect, IntVect)
Z80_GET_REGISTER_FUNC(uint8_t, IntLine, IntLine)
Z80_GET_REGISTER_FUNC(uint8_t, Status, Status)

/*! Z80 register set functions. **/

#define Z80_SET_REGISTER_FUNC(reg_type, reg_name, reg_access) \
void mdZ80_set_##reg_name(mdZ80_context *z80, reg_type data) \
{ \
	if (z80->Status & Z80_STATE_RUNNING) \
		return; \
	z80->reg_access = data; \
}

void mdZ80_set_AF(mdZ80_context *z80, uint16_t data)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Set the A register.
	z80->AF.b.A = ((data >> 8) & 0xFF);
	
	// Set the F register.
	data &= 0xFF;
	z80->AF.b.F = (data & ~(Z80_FLAG_X | Z80_FLAG_Y));
	
	// Set the FXY register.
	z80->AF.b.FXY = (data & (Z80_FLAG_X | Z80_FLAG_Y));
}

Z80_SET_REGISTER_FUNC(uint16_t, BC, BC.w)
Z80_SET_REGISTER_FUNC(uint16_t, DE, DE.w)
Z80_SET_REGISTER_FUNC(uint16_t, HL, HL.w)
Z80_SET_REGISTER_FUNC(uint16_t, IX, IX.w)
Z80_SET_REGISTER_FUNC(uint16_t, IY, IY.w)

/**
 * Set the Z80 program counter.
 * @param z80 Pointer to Z80 context.
 * @param data New program counter.
 */
void mdZ80_set_PC(mdZ80_context *z80, uint16_t data)
{
	unsigned int newPC;
	if (z80->Status & Z80_STATE_RUNNING)
		return;

	// TODO: 32-bit specific code will break on 64-bit!
	data &= 0xFFFF;
	newPC = (unsigned int)(z80->Fetch[data >> 8]);
	z80->BasePC = newPC;
	z80->PC = newPC + data;
}

Z80_SET_REGISTER_FUNC(uint16_t, SP, SP.w)

/**
 * Set the AF' register.
 * @param z80 Pointer to Z80 context.
 * @param data New register value.
 */
void mdZ80_set_AF2(mdZ80_context *z80, uint16_t data)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Set the A' register.
	z80->AF2.b.A2 = ((data >> 8) & 0xFF);
	
	// Set the F' register.
	data &= 0xFF;
	z80->AF2.b.F2 = (data & ~(Z80_FLAG_X | Z80_FLAG_Y));
	
	// Set the FXY2 register.
	z80->AF2.b.FXY2 = (data & (Z80_FLAG_X | Z80_FLAG_Y));
}

Z80_SET_REGISTER_FUNC(uint16_t, BC2, BC2)
Z80_SET_REGISTER_FUNC(uint16_t, DE2, DE2)
Z80_SET_REGISTER_FUNC(uint16_t, HL2, HL2)

/**
 * Set the IFF flip-flops
 * @param z80 Pointer to Z80 context.
 * @param data New flip-flop value.
 */
void mdZ80_set_IFF(mdZ80_context *z80, uint8_t data)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	z80->IFF = (data & 3);
}

Z80_SET_REGISTER_FUNC(uint8_t, R, R)
Z80_SET_REGISTER_FUNC(uint8_t, I, I)
Z80_SET_REGISTER_FUNC(uint8_t, IM, IM)
Z80_SET_REGISTER_FUNC(uint8_t, IntVect, IntVect)
Z80_SET_REGISTER_FUNC(uint8_t, IntLine, IntLine)
Z80_SET_REGISTER_FUNC(uint8_t, Status, Status)
