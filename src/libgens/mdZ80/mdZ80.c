/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80.c: Main Z80 emulation functions.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

// Z80 flag and state definitions.
#include "mdZ80_flags.h"


/**
 * mdZ80_init(): Initialize a Z80 context.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_init(Z80_CONTEXT *z80)
{
	// Clear the entire Z80 struct.
	memset(z80, 0x00, sizeof(Z80_CONTEXT));
	
	// Clear the default Z80 memory buffer.
	memset(mdZ80_def_mem, 0x00, sizeof(mdZ80_def_mem));
	
	// Set up the Z80 function pointer variables.
	unsigned int i;
	for (i = 0; i < 0x100; i++)
	{
		z80->ReadB[i] = mdZ80_def_ReadB;
		z80->ReadW[i] = mdZ80_def_ReadW;
		z80->WriteB[i] = mdZ80_def_WriteB;
		z80->WriteW[i] = mdZ80_def_WriteW;
		z80->Fetch[i] = mdZ80_def_mem;
	}
	
	// Set up the I/O handlers.
	z80->IN_C = mdZ80_def_In;
	z80->OUT_C = mdZ80_def_Out;
}


/**
 * mdZ80_reset(): Reset the Z80 CPU.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_reset(Z80_CONTEXT *z80)
{
	// Save the Z80 Cycle Count.
	unsigned int cycleCnt = z80->CycleCnt;
	
	// Clear the Z80 struct up to CycleSup.
	memset(z80, 0x00, 23*4);
	
	// Restore the Z80 Cycle Count.
	z80->CycleCnt = cycleCnt;
	
	// Initialize the program counter.
	mdZ80_set_PC(z80, 0);
	
	// Initialize the index and flag registers.
	z80->IX.d = 0xFFFF;
	z80->IY.d = 0xFFFF;
	z80->AF.d = 0x4000;
}


/**
 * mdZ80_get_PC(): Get the Z80 program counter.
 * @param z80 Pointer to Z80 context.
 * @return Z80 program counter, or -1 (0xFFFFFFFF) if the Z80 is running.
 */
unsigned int mdZ80_get_PC(Z80_CONTEXT *z80)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;
	
	// Subtract the BasePC from PC to get the actual Z80 program counter.
	return (z80->PC.d - z80->BasePC);
}


/**
 * mdZ80_set_PC(): Set the Z80 program counter.
 * @param z80 Pointer to Z80 context.
 * @param PC New program counter.
 */
void mdZ80_set_PC(Z80_CONTEXT *z80, unsigned int PC)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// TODO: 32-bit specific code will break on 64-bit!
	PC &= 0xFFFF;
	unsigned int newPC = (unsigned int)(z80->Fetch[PC >> 8]);
	z80->BasePC = newPC;
	z80->PC.d = newPC + PC;
}


/**
 * mdZ0_get_AF(): Get the AF register.
 * @param z80 Pointer to Z80 context.
 * @return AF register, or -1 during z80_Exec().
 */
unsigned int mdZ80_get_AF(Z80_CONTEXT *z80)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;
	
	// F register.
	// The X and Y flags are stored separately from the
	// rest of the flags for some reason.
	unsigned char F = (z80->AF.b.F & ~(Z80_FLAG_X | Z80_FLAG_Y)) |
			  (z80->AF.b.FXY & (Z80_FLAG_X | Z80_FLAG_Y));
	
	// Return AF.
	return ((z80->AF.b.A << 8) | F);
}


/**
 * mdZ80_set_AF(): Set the AF register.
 * @param z80 Pointer to Z80 context.
 * @param newAF New AF register value.
 */
void mdZ80_set_AF(Z80_CONTEXT *z80, unsigned int newAF)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Set the A register.
	z80->AF.b.A = (newAF >> 8) & 0xFF;
	
	// Set the F register.
	newAF &= 0xFF;
	z80->AF.b.F = newAF & ~(Z80_FLAG_X | Z80_FLAG_Y);
	
	// Set the FXY register.
	z80->AF.b.FXY = newAF & (Z80_FLAG_X | Z80_FLAG_Y);
}


/**
 * mdZ80_get_AF2(): Get the AF' register.
 * @param z80 Pointer to Z80 context.
 * @return AF' register, or -1 during z80_Exec().
 */
unsigned int mdZ80_get_AF2(Z80_CONTEXT *z80)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;
	
	// F' register.
	// The X and Y flags are stored separately from the
	// rest of the flags for some reason.
	unsigned char F2 = (z80->AF2.b.F2 & ~(Z80_FLAG_X | Z80_FLAG_Y)) |
			  (z80->AF2.b.FXY2 & (Z80_FLAG_X | Z80_FLAG_Y));
	
	// Return AF'.
	return ((z80->AF2.b.A2 << 8) | F2);
}


/**
 * mdZ0_set_AF2(): Set the AF' register.
 * @param z80 Pointer to Z80 context.
 * @param newAF New AF' register value.
 */
void mdZ80_set_AF2(Z80_CONTEXT *z80, unsigned int newAF2)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Set the A' register.
	z80->AF2.b.A2 = (newAF2 >> 8) & 0xFF;
	
	// Set the F' register.
	newAF2 &= 0xFF;
	z80->AF2.b.F2 = newAF2 & ~(Z80_FLAG_X | Z80_FLAG_Y);
	
	// Set the FXY2 register.
	z80->AF2.b.FXY2 = newAF2 & (Z80_FLAG_X | Z80_FLAG_Y);
}


/**
 * mdZ80_read_odo(): Read the Z80 odometer.
 * @param z80 Pointer to Z80 context.
 * @return Z80 odometer, or -1 during z80_Exec().
 */
unsigned int mdZ80_read_odo(Z80_CONTEXT *z80)
{
	if (z80->Status & Z80_STATE_RUNNING)
		return -1;
	
	return (z80->CycleCnt + z80->CycleTD - z80->CycleIO);
}


/**
 * mdZ80_set_odo(): Set the Z80 odometer.
 * @param z80 Pointer to Z80 context.
 * @param odo New value for the Z80 odometer.
 */
void mdZ80_set_odo(Z80_CONTEXT *z80, unsigned int odo)
{
	z80->CycleCnt = odo;
}


/**
 * mdZ80_add_cycles(): Add cycles to the Z80 odometer.
 * @param z80 Pointer to Z80 context.
 * @param cycles Number of cycles to add.
 */
void mdZ80_add_cycles(Z80_CONTEXT *z80, unsigned int cycles)
{
	if (z80->Status & Z80_STATE_RUNNING)
	{
		// z80_Exec() is running.
		z80->CycleIO -= cycles;
	}
	else
	{
		// z80_Exec() is not running.
		z80->CycleCnt += cycles;
	}
}


/**
 * mdZ80_nmi(): Raise a non-maskable interrupt.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_nmi(Z80_CONTEXT *z80)
{
	z80->IntVect = 0x66;
	z80->IntLine = 0x80;
	
	// If the Z80 is currently running, don't do anything else.
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Shift the cycle variables.
	z80->CycleSup = z80->CycleIO;
	z80->CycleIO = 0;
}


/**
 * mdZ80_interrupt(): Raise a Z80 interrupt.
 * TODO: Figure out the exact purpose of this function.
 * @param z80 Pointer to Z80 context.
 * @param vector Interrupt vector.
 */
void mdZ80_interrupt(Z80_CONTEXT *z80, unsigned char vector)
{
	// Set the interrupt data.
	z80->IntVect = vector;
	z80->IntLine = Z80_FLAG_P;	// because of IFF mask
	
	// If the Z80 is currently running, don't do anything else.
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Shift the cycle variables.
	z80->CycleSup = z80->CycleIO;
	z80->CycleIO = 0;
}
