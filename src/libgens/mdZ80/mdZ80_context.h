/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_context.h: Z80 context definition.                                *
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

#ifndef __MDZ80_CONTEXT_H__
#define __MDZ80_CONTEXT_H__

// NOTE: This file should ONLY be included directly by mdZ80 code!

/****************************/
/* Structures & definitions */
/****************************/

// TODO: Big-endian support.
struct _mdZ80_context {
	union {
		struct {
			uint8_t A;
			uint8_t F;
		};
		uint16_t AF;
	};
	union {
		struct {
			uint8_t C;
			uint8_t B;
		};
		uint16_t BC;
	};
	union {
		struct {
			uint8_t E;
			uint8_t D;
		};
		uint16_t DE;
	};
	union {
		struct {
			uint8_t L;
			uint8_t H;
		};
		uint16_t HL;
	};
	union {
		struct {
			uint8_t IXl;
			uint8_t IXh;
		};
		uint16_t IX;
	};
	union {
		struct {
			uint8_t IYl;
			uint8_t IYh;
		};
		uint16_t IY;
	};

	uintptr_t PC;	// PC == BasePC + Z80 PC [x86 pointer!]

	union {
		struct {
			uint8_t SPL;
			uint8_t SPH;
		};
		uint16_t SP;
	};

	// Second register set.
	union {
		struct {
			uint8_t A2;
			uint8_t F2;
		};
		uint16_t AF2;
	};
	uint16_t BC2;
	uint16_t DE2;
	uint16_t HL2;

	// Internal registers.
	uint8_t IFF;		// Interrupt flip-flops.
	uint8_t R;		// Refresh register.

	// Interrupt registers.
	uint8_t I;		// Interrupt vector page. (IM 2)
	uint8_t IM;		// Interrupt mode.
	uint8_t IntVect;	// Interrupt vector. (IM 0, IM 2)
	uint8_t IntLine;	// Interrupt line. (0x01 == INT; 0x80 == NMI)

	// Z80 status flags.
	uint8_t Status;
	uint8_t reserved_stat;	// Reserved for struct alignment.

	/** Internal registers. **/

	uintptr_t BasePC;	// Pointer to x86 memory location where Z80 RAM starts.

	// TODO: Convert to int?
	uint32_t CycleCnt;
	uint32_t CycleTD;
	// DEPRECATED: CycleIO is used to temporarily store edi (odometer)
	// when calling I/O functions. This is not needed in the C version,
	// so it should be removed.
	uint32_t CycleIO;
	uint32_t CycleSup;

	// Instruction fetch locations.
	const uint8_t *Fetch[0x100];

	// Memory read/write functions.
	Z80_RB *ReadB;
	Z80_WB *WriteB;

	// I/O read/write functions.
	Z80_RB *IN_C;
	Z80_WB *OUT_C;
};

#endif /* __MDZ80_CONTEXT_H__ */
