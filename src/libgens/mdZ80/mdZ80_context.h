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

/*! NOTE: This file should NOT be included outside of mdZ80.c! **/

#ifndef __MDZ80_CONTEXT_H__
#define __MDZ80_CONTEXT_H__

/****************************/
/* Structures & definitions */
/****************************/

typedef uint8_t FASTCALL Z80_RB(uint32_t adr);
typedef void FASTCALL Z80_WB(uint32_t adr, uint8_t data);

typedef struct _mdZ80_context
{
	union
	{
		struct
		{
			uint8_t A;
			uint8_t F;
			uint8_t x;
			uint8_t FXY;
		} b;
		struct
		{
			uint16_t AF;
			uint16_t FXYW;
		} w;
		uint32_t d;
	} AF;
	union
	{
		struct
		{
			uint8_t C;
			uint8_t B;
		} b;
		uint16_t w;
	} BC;
	union
	{
		struct
		{
			uint8_t E;
			uint8_t D;
		} b;
		uint16_t w;
	} DE;
	union
	{
		struct
		{
			uint8_t L;
			uint8_t H;
		} b;
		uint16_t w;
	} HL;
	union
	{
		struct
		{
			uint8_t IXL;
			uint8_t IXH;
		} b;
		uint16_t w;
	} IX;
	union
	{
		struct
		{
			uint8_t IYL;
			uint8_t IYH;
		} b;
		uint16_t w;
	} IY;
	uint16_t reserved_reg;	// Reserved for struct alignment.
	
	uint32_t PC;	// PC == BasePC + Z80 PC [x86 pointer!]
	
	union
	{
		struct
		{
			uint8_t SPL;
			uint8_t SPH;
		} b;
		uint16_t w;
	} SP;
	uint16_t reserved_sp;	// Reserved for struct alignment.
	
	union
	{
		struct
		{
			uint8_t A2;
			uint8_t F2;
			uint8_t x;
			uint8_t FXY2;
		} b;
		struct
		{
			uint16_t AF2;
			uint16_t FXYW2;
		} w;
		uint32_t d;
	} AF2;
	uint16_t BC2;
	uint16_t DE2;
	uint16_t HL2;
	uint16_t reserved_reg2;	// Reserved for struct alignment.
	
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
	
	uint32_t BasePC;	// Pointer to x86 memory location where Z80 RAM starts.
	
	uint32_t CycleCnt;
	uint32_t CycleTD;
	uint32_t CycleIO;
	uint32_t CycleSup;
	uint8_t *Fetch[0x100];
	
	Z80_RB *ReadB;
	Z80_WB *WriteB;
	
	Z80_RB *IN_C;
	Z80_WB *OUT_C;
} mdZ80_context;

#endif /* __MDZ80_CONTEXT_H__ */
