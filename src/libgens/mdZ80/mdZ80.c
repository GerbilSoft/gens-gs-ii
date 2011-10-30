/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80.c: Main Z80 emulation functions.                                  *
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

// Default instruction fetch area. (Consists of HALT opcodes.)
static uint8_t mdZ80_insn_fetch[256];

/*! Z80 context allocation. **/

/**
 * Create a new Z80 context.
 * @return New Z80 context, or NULL on error.
 */
mdZ80_context *mdZ80_new(void)
{
	// Create a new Z80 context.
	mdZ80_context *z80 = (mdZ80_context*)calloc(1, sizeof(mdZ80_context));
	if (!z80)
		return NULL;
	
	// Initialize the default instruction fetch area.
	// TODO: Only initialize this once!
	for (size_t i = 0; i < (sizeof(mdZ80_insn_fetch)/sizeof(mdZ80_insn_fetch[0])); i++)
		mdZ80_insn_fetch[i] = 0x76;	// HLT
	
	// Initialize Fetch[].
	for (size_t i = 0; i < (sizeof(z80->Fetch)/sizeof(z80->Fetch[0])); i++)
		z80->Fetch[i] = (mdZ80_insn_fetch - (i * 256));
	
	// Set up the default memory and I/O handlers.
	z80->ReadB = mdZ80_def_ReadB;
	z80->WriteB = mdZ80_def_WriteB;
	z80->IN_C = mdZ80_def_In;
	z80->OUT_C = mdZ80_def_Out;
	
	// Return the new Z80 context.
	return z80;
}

/**
 * Free a Z80 context.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_free(mdZ80_context *z80)
{
	free(z80);
}

/**
 * Reset the Z80 CPU. (Hard Reset)
 * Resets *all* parameters, including cycle count.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_hard_reset(mdZ80_context *z80)
{
	// Save the Z80 Cycle Count.
	unsigned int cycleCnt = z80->CycleCnt;
	
	// Clear the Z80 struct up to CycleSup.
	// NOTE: offsetof() is used to prevent issues if the struct is resized.
	memset(z80, 0x00, offsetof(mdZ80_context, Fetch));
	
	// Restore the Z80 Cycle Count.
	z80->CycleCnt = cycleCnt;
	
	// TODO: Initialize registers to 0xFFFF?
	// Gens and genplus-gx initialize them to 0,
	// except for specific registers in Soft Reset.
	
	// Initialize the registers using Soft Reset.
	mdZ80_soft_reset(z80);
}

/**
 * Reset the Z80 CPU.
 * This is equivalent to asserting the !RESET line.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_soft_reset(mdZ80_context *z80)
{
	/**
	 * References:
	 * [1] "The Undocumented Z80 Docuemnted" by Sean Young, v0.91 (2005/09/18)
	 * [2] http://gs_server.gerbilsoft.ddns.info/bugs/show_bug.cgi?id=47
	 */
	
	// NOTE: Both [1] and [2] say that other registers are *not* touched
	// when !RESET is asserted, so they're left as-is.
	
	// TODO: Write a test program for MD, then test it on actual hardware.
	
	// Z80 program starts at 0x0000.
	mdZ80_set_PC(z80, 0);	// old Gens; [1]
	
	// TODO: Are IX and IY actually reset on !RESET?
	z80->IX.w = 0xFFFF;	// old Gens; also genplus-gx
	z80->IY.w = 0xFFFF;	// old Gens; also genplus-gx
	
	// TODO: Initialize AF2 to 0xFFFF?
	mdZ80_set_AF(z80, 0xFFFF);	// [1]; Gens originally used 0x4000 (ZF only).
	z80->SP.w = 0xFFFF;		// [1]
	z80->IFF = 0;			// [1]
	z80->R = 0;			// [2]
	z80->I = 0;			// [2]
	z80->IM = 0;			// [1]
}


/*! Odometer (clock cycle) functions. **/

/**
 * mdZ80_read_odo(): Read the Z80 odometer.
 * @param z80 Pointer to Z80 context.
 * @return Z80 odometer, or -1 during z80_Exec().
 */
unsigned int mdZ80_read_odo(mdZ80_context *z80)
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
void mdZ80_set_odo(mdZ80_context *z80, unsigned int odo)
{
	z80->CycleCnt = odo;
}


/**
 * Clear the Z80 odometer.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_clear_odo(mdZ80_context *z80)
{
	z80->CycleCnt = 0;
}


/**
 * mdZ80_add_cycles(): Add cycles to the Z80 odometer.
 * @param z80 Pointer to Z80 context.
 * @param cycles Number of cycles to add.
 */
void mdZ80_add_cycles(mdZ80_context *z80, unsigned int cycles)
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
void mdZ80_nmi(mdZ80_context *z80)
{
	z80->IntVect = 0x66;
	z80->IntLine = 0x80;	// NMI flag.
	
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
void mdZ80_interrupt(mdZ80_context *z80, unsigned char vector)
{
	// Set the interrupt data.
	z80->IntVect = vector;
	z80->IntLine = 0x01;	// IFF1 flag.
	
	// If the Z80 is currently running, don't do anything else.
	if (z80->Status & Z80_STATE_RUNNING)
		return;
	
	// Shift the cycle variables.
	z80->CycleSup = z80->CycleIO;
	z80->CycleIO = 0;
}


/** Default memory and I/O handlers. **/


/**
 * mdZ80_def_ReadB(): Default memory read handler.
 * @param address Address.
 * @return 0xFF.
 */
uint8_t FASTCALL mdZ80_def_ReadB(uint32_t address)
	{ ((void)address); return 0xFF; }

/**
 * mdZ80_def_In(): Default I/O read handler.
 * @param address Address.
 * @return 0xFF.
 */
uint8_t FASTCALL mdZ80_def_In(uint32_t address)
	{ ((void)address); return 0xFF; }

/**
 * mdZ80_def_WriteB(): Default memory write handler.
 * @param address Address.
 * @param data Data.
 */
void FASTCALL mdZ80_def_WriteB(uint32_t address, uint8_t data)
	{ ((void)address); ((void)data); }

/**
 * mdZ80_def_ReadB(): Default I/O write handler.
 * @param address Address.
 * @param data Data.
 */
void FASTCALL mdZ80_def_Out(uint32_t address, uint8_t data)
	{ ((void)address); ((void)data); }


/** Set memory and I/O handlers. **/


/**
 * mdZ80_Set_ReadB(): Set the ReadB handler.
 * @param z80 Z80 context.
 * @param func ReadB handler.
 */
void mdZ80_Set_ReadB(mdZ80_context *z80, Z80_RB *func)
{
	z80->ReadB = (func ? func : mdZ80_def_ReadB);
}

/**
 * mdZ80_Set_ReadB(): Set the WriteB handler.
 * @param z80 Z80 context.
 * @param func WriteB handler.
 */
void mdZ80_Set_WriteB(mdZ80_context *z80, Z80_WB *func)
{
	z80->WriteB = (func ? func : mdZ80_def_WriteB);
}

/**
 * mdZ80_Set_In(): Set the I/O IN handler.
 * @param z80 Z80 context.
 * @param func I/O IN handler.
 */
void mdZ80_Set_In(mdZ80_context *z80, Z80_RB *func)
{
	z80->IN_C = (func ? func : mdZ80_def_In);
}

/**
 * mdZ80_Set_In(): Set the I/O OUT handler.
 * @param z80 Z80 context.
 * @param func I/O OUT handler.
 */
void mdZ80_Set_Out(mdZ80_context *z80, Z80_WB *func)
{
	z80->OUT_C = (func ? func : mdZ80_def_Out);
}

/**
 * mdZ80_Add_Fetch(): Add an instruction fetch handler.
 * TODO: Convert instruction fetch to use standard memory read.
 * @param z80 Z80 context.
 * @param low_adr Low page.
 * @param high_adr High page.
 * @param region Memory region.
 */
void mdZ80_Add_Fetch(mdZ80_context *z80, uint8_t low_adr, uint8_t high_adr, uint8_t *region)
{
	region -= (low_adr << 8);
	for (int i = low_adr; i <= high_adr; i++)
		z80->Fetch[i] = region;
}
