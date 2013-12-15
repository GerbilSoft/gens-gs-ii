/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_exec.c: Z80 execution loop.                                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2013 by David Korth                                  *
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

/**
 * References:
 * - http://www.phy.davidson.edu/fachome/dmb/py310/Z80.Instruction%20set.pdf
 * - http://www.myquest.nl/z80undocumented/z80-documented-v0.91.pdf
 * - http://www.z80.info/z80sflag.htm
 * - http://www.z80.info/z80undoc3.txt
 * - http://www.z80.info/z80oplist.txt
 */

#include "mdZ80.h"
#include "mdZ80_context.h"
#include "mdZ80_flags.h"

/**
 * Set the program counter.
 * @param z80 Z80 context.
 * @param pc New program counter.
 */
static inline void set_pc(mdZ80_context *z80, uint16_t pc)
{
	z80->BasePC = (uintptr_t)z80->Fetch[pc >> 8];
	z80->PC = z80->BasePC + pc;
}

/**
 * Read the byte at the program counter.
 * @param z80 Z80 context.
 * @return Byte at the program counter.
 */
static inline uint8_t read_byte_pc(mdZ80_context *z80)
{
	// WARNING: This can potentially crash the emulator
	// if the program attempts to run past 0xFFFF!
	// No boundary checks are performed!
	// FIXME: Fix this in the C rewrite!
	return *((uint8_t*)z80->PC);
}

/**
 * Read a byte after the program counter.
 * @param z80 Z80 context.
 * @param offset Byte offset. (0 == PC)
 * @return Byte after the program counter.
 */
static inline uint8_t read_byte_offset_pc(mdZ80_context *z80, int offset)
{
	// WARNING: This can potentially crash the emulator
	// if the program attempts to run past 0xFFFF!
	// No boundary checks are performed!
	// FIXME: Fix this in the C rewrite!
	return *((uint8_t*)(z80->PC + offset));
}

/**
 * Read a signed byte after the program counter.
 * @param z80 Z80 context.
 * @param offset Byte offset. (0 == PC)
 * @return Signed byte after the program counter.
 */
static inline int8_t read_sbyte_offset_pc(mdZ80_context *z80, int offset)
{
	// WARNING: This can potentially crash the emulator
	// if the program attempts to run past 0xFFFF!
	// No boundary checks are performed!
	// FIXME: Fix this in the C rewrite!
	return *((int8_t*)(z80->PC + offset));
}

/**
 * Read a word after the program counter.
 * @param z80 Z80 context.
 * @param offset Byte offset. (0 == PC)
 * @return Word after the program counter.
 */
static inline uint16_t read_word_offset_pc(mdZ80_context *z80, int offset)
{
	// WARNING: This can potentially crash the emulator
	// if the program attempts to run past 0xFFFF!
	// No boundary checks are performed!
	// FIXME: Fix this in the C rewrite!
	return *((uint8_t*)(z80->PC + offset)) | (*((uint8_t*)(z80->PC + offset + 1)) << 8);
}

/**
 * Swap the contents of two uint16_t variables.
 * @param w1 Word 1.
 * @param w2 Word 2.
 */
#define SWAP16(w1, w2) \
	do { \
		const uint16_t tmp16 = w1; \
		w1 = w2; \
		w2 = tmp16; \
	} while (0)

// TODO: This is a hack for "optimization".
// Remove it later.
extern uint8_t Ram_Z80[0x2000];

static inline uint8_t READ_BYTE(mdZ80_context *z80, uint16_t addr)
{
	if (addr <= 0x3FFF)
		return Ram_Z80[addr & 0x1FFF];
	return z80->ReadB(addr);
}

static inline void WRITE_BYTE(mdZ80_context *z80, uint16_t addr, uint8_t data)
{
	if (addr <= 0x3FFF) {
		Ram_Z80[addr & 0x1FFF] = data;
		return;
	}
	z80->WriteB(addr, data);
}

static inline uint16_t READ_WORD(mdZ80_context *z80, uint16_t addr)
{
	if (addr <= 0x3FFF) {
		return (Ram_Z80[addr & 0x1FFF] |
		        (Ram_Z80[(addr+1) & 0x1FFF] << 8));
	}

	// TODO: Store odo in z80->CycleIO?
	uint16_t data = z80->ReadB(addr);
	data |= (z80->ReadB(addr+1) << 8);
	return data;
}

static inline void WRITE_WORD(mdZ80_context *z80, uint16_t addr, uint16_t data)
{
	if (addr <= 0x3FFF) {
		Ram_Z80[addr & 0x1FFF] = (data & 0xFF);
		Ram_Z80[(addr+1) & 0x1FFF] = (data >> 8);
		return;
	}

	// TODO: Store odo in z80->CycleIO?
	z80->WriteB(addr, (data & 0xFF));
	z80->WriteB((addr+1), (data >> 8));
}

static inline uint8_t DO_IN(mdZ80_context *z80, uint16_t io_addr)
{
	// TODO: Store odo in z80->CycleIO?
	return z80->IN_C(io_addr);
}

static inline void DO_OUT(mdZ80_context *z80, uint16_t io_addr, uint8_t data)
{
	// TODO: Store odo in z80->CycleIO?
	z80->OUT_C(io_addr, data);
}

/**
 * Calculate the Z80 parity flag.
 * @param data Data to check.
 * @return MDZ80_FLAG_P if even number of bits set; 0 otherwise.
 */
static inline uint8_t calc_parity_flag(uint8_t data)
{
	// TODO: Lookup table optimization?
	// TODO: Not needed if 'lahf' optimization is in use?
	data ^= (data >> 4);
	data ^= (data >> 2);
	data ^= (data >> 1);
	return ((data & 1) << 2);
}

/**
 * Check for interrupts.
 * @param z80 Z80 context.
 * @return Cycles used.
 */
static inline int check_interrupts(mdZ80_context *z80)
{
	/*
	 * Interrupt lines: (z80->IntLine)
	 * - 0x01: INT
	 * - 0x80: NMI
	 */
	int cycles = 0;

	if (!z80->IntLine) {
		// No interrupts.
		cycles = 0;
	} else if (z80->IntLine & 0x80) {
		// NMI

		// Push the current PC onto the stack.
		uint16_t curPC = (z80->PC - z80->BasePC);
		z80->SP -= 2;
		WRITE_WORD(z80, z80->SP, curPC);

		// Clear IFF1. (IFF2 remains as-is.)
		z80->IFF &= ~1;

		// Reset the interrupt status.
		z80->IntLine &= ~0x80;
		z80->Status &= ~MDZ80_STATUS_HALTED;

		// Set the PC to 0x66. (NMI vector)
		set_pc(z80, 0x66);

		// TODO: 0 cycles for an NMI?
		cycles = 0;
	} else if (z80->IntLine & z80->IFF) {
		// Interrupt.

		// Push the current PC onto the stack.
		uint16_t curPC = (z80->PC - z80->BasePC);
		z80->SP -= 2;
		WRITE_WORD(z80, z80->SP, curPC);

		// INT clears both IFF1 and IFF2.
		z80->IFF = 0;

		// Reset the interrupt status.
		z80->IntLine &= ~0x80;
		z80->Status &= ~MDZ80_STATUS_HALTED;

		// Determine the new PC based on the interrupt mode.
		uint16_t int_pc;
		switch (z80->IM & 3) {
			case 0:
				// IM0: Execute instruction.
				// Technically, this jumps to whatever
				// instruction is on the bus, but we
				// don't support it, since the MD only
				// supports IM1.

				// Assume we have an RST instruction.
				// (TODO: Is this correct?)
				int_pc = (z80->IntVect - 0xC7) & 0xFF;
				cycles = 13;
				break;

			case 1:
			default:
				// IM1: RST 38H
				// This interrupt mode always executes
				// an RST 38H instruction. It's the only
				// interrupt mode properly supported by
				// the Mega Drive (and Master System) hardware.
				int_pc = 0x38;
				cycles = 13;
				break;

			case 2: {
				// IM2: Vectored interrupts
				// An interrupt vector address is specified
				// with Z80.I as the high 8 bits, and
				// Z80.IntVect (the data bus) as the low 8 bits.
				uint16_t int_addr = (z80->IntVect | (z80->I << 8));
				// TODO: Implement READ_WORD().
				int_pc = READ_WORD(z80, int_addr);
				cycles = 19;
				break;
			}
		}

		// Set the PC.
		set_pc(z80, int_pc);
		// Return number of cycles to run.
		return cycles;
	}

	// Should not get here...
	return 0;
}

/**
 * Execute Z80 instructions.
 * @param z80 Z80 context.
 * @param odo Cycle number to run to.
 * @return 0 on success; -1 if no cycles remaining; other on error.
 */
int mdZ80_exec(mdZ80_context *z80, int odo)
{
	// Check if we've used up all the cycles already.
	odo -= z80->CycleCnt;
	if (odo <= 0) {
		// Out of cycles.
		return -1;
	}

	// Decrement the odometer.
	// z80->CycleTD is the current odometer.
	// Remember to save odo to z80->CycleTD before calling external functions.
	odo--;
	z80->CycleTD = odo;

	// Check for interrupts.
	odo -= check_interrupts(z80);

	// Check if the Z80 can't run for some reason.
	if (z80->Status & (MDZ80_STATUS_HALTED | MDZ80_STATUS_FAULTED | MDZ80_STATUS_RUNNING)) {
		// Z80 can't run. Figure out why.
		if (z80->Status & MDZ80_STATUS_HALTED) {
			// Z80 is halted.
			// Add the cycles to the counter.
			z80->CycleCnt += odo;
		}

		// We're done here.
		return 0; // NOTE: Original asm has unknown %eax here...
	}

	// Z80 is now running.
	z80->Status |= MDZ80_STATUS_RUNNING;
	z80->CycleSup = 0;

	// Z80 instruction table.
	#define __MDZ80_IN_EXEC 19840519
	#include "mdZ80_insn_table.inc.h"
	#undef __MDZ80_IN_EXEC

	// Get the next instruction.
	uint8_t insn = read_byte_pc(z80);
	goto *z80_insn_table[insn];

	// Instructions!
	#define __MDZ80_IN_EXEC 19840519
	#include "mdZ80_insn_exec.inc.h"
	#undef __MDZ80_IN_EXEC

	z80_Exec_Quit:
		// Out of cycles.
		// Check if there are any cycles "remaining". (?)
		odo += z80->CycleSup;
		z80->CycleSup = 0;
		if (odo >= 0) {
			// Check if an interrupt happened.
			odo -= check_interrupts(z80);

			// Get the next instruction.
			uint8_t insn = read_byte_pc(z80);
			goto *z80_insn_table[insn];
		}

	z80_Exec_Really_Quit: {
		// Terminate early for some reason.
		// FIXME: Better names for the variables.
		// FIXME: Verify that this is correct.
		uint32_t eax = z80->CycleTD;
		uint32_t ebx = z80->CycleCnt;
		eax -= odo;
		z80->Status &= ~MDZ80_STATUS_RUNNING;
		eax += ebx;
		z80->CycleCnt = eax;
		
		if (z80->Status & MDZ80_STATUS_HALTED) {
			// Z80 is halted.
			// Add the cycles to the counter.
			z80->CycleCnt += odo;
		}

		return 0;
	}
}
