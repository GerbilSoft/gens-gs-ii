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

#define Z80_GET_REGISTER_FUNC(reg_type, reg_name) \
reg_type mdZ80_get_##reg_name(mdZ80_context *z80) \
{ \
	if (z80->Status & MDZ80_STATUS_RUNNING) \
		return -1; \
	return z80->reg_name; \
}

Z80_GET_REGISTER_FUNC(uint16_t, AF)
Z80_GET_REGISTER_FUNC(uint16_t, BC)
Z80_GET_REGISTER_FUNC(uint16_t, DE)
Z80_GET_REGISTER_FUNC(uint16_t, HL)
Z80_GET_REGISTER_FUNC(uint16_t, IX)
Z80_GET_REGISTER_FUNC(uint16_t, IY)

uint16_t mdZ80_get_PC(mdZ80_context *z80)
{
	if (z80->Status & MDZ80_STATUS_RUNNING)
		return -1;
	
	// Subtract the BasePC from PC to get the actual Z80 program counter.
	return (uint16_t)(z80->PC - z80->BasePC);
}

Z80_GET_REGISTER_FUNC(uint16_t, SP)

Z80_GET_REGISTER_FUNC(uint16_t, AF2)
Z80_GET_REGISTER_FUNC(uint16_t, BC2)
Z80_GET_REGISTER_FUNC(uint16_t, DE2)
Z80_GET_REGISTER_FUNC(uint16_t, HL2)

uint8_t mdZ80_get_IFF(mdZ80_context *z80)
{
	if (z80->Status & MDZ80_STATUS_RUNNING)
		return -1;
	return (z80->IFF & 3);
}

Z80_GET_REGISTER_FUNC(uint8_t, R)
Z80_GET_REGISTER_FUNC(uint8_t, I)
Z80_GET_REGISTER_FUNC(uint8_t, IM)
Z80_GET_REGISTER_FUNC(uint8_t, IntVect)
Z80_GET_REGISTER_FUNC(uint8_t, IntLine)
Z80_GET_REGISTER_FUNC(uint8_t, Status)

/*! Z80 register set functions. **/

#define Z80_SET_REGISTER_FUNC(reg_type, reg_name) \
void mdZ80_set_##reg_name(mdZ80_context *z80, reg_type data) \
{ \
	if (z80->Status & MDZ80_STATUS_RUNNING) \
		return; \
	z80->reg_name = data; \
}

Z80_SET_REGISTER_FUNC(uint16_t, AF)
Z80_SET_REGISTER_FUNC(uint16_t, BC)
Z80_SET_REGISTER_FUNC(uint16_t, DE)
Z80_SET_REGISTER_FUNC(uint16_t, HL)
Z80_SET_REGISTER_FUNC(uint16_t, IX)
Z80_SET_REGISTER_FUNC(uint16_t, IY)

/**
 * Set the Z80 program counter.
 * @param z80 Pointer to Z80 context.
 * @param data New program counter.
 */
void mdZ80_set_PC(mdZ80_context *z80, uint16_t data)
{
	if (z80->Status & MDZ80_STATUS_RUNNING)
		return;
	
	// TODO: 32-bit specific code will break on 64-bit!
	data &= 0xFFFF;
	unsigned int newPC = (unsigned int)(z80->Fetch[data >> 8]);
	z80->BasePC = newPC;
	z80->PC = newPC + data;
}

Z80_SET_REGISTER_FUNC(uint16_t, SP)

Z80_SET_REGISTER_FUNC(uint16_t, AF2)
Z80_SET_REGISTER_FUNC(uint16_t, BC2)
Z80_SET_REGISTER_FUNC(uint16_t, DE2)
Z80_SET_REGISTER_FUNC(uint16_t, HL2)

/**
 * Set the IFF flip-flops
 * @param z80 Pointer to Z80 context.
 * @param data New flip-flop value.
 */
void mdZ80_set_IFF(mdZ80_context *z80, uint8_t data)
{
	if (z80->Status & MDZ80_STATUS_RUNNING)
		return;
	z80->IFF = (data & 3);
}

Z80_SET_REGISTER_FUNC(uint8_t, R)
Z80_SET_REGISTER_FUNC(uint8_t, I)
Z80_SET_REGISTER_FUNC(uint8_t, IM)
Z80_SET_REGISTER_FUNC(uint8_t, IntVect)
Z80_SET_REGISTER_FUNC(uint8_t, IntLine)
Z80_SET_REGISTER_FUNC(uint8_t, Status)
