/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpTypes.hpp: VDP types.                                                *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __LIBGENS_MD_VDPREG_HPP__
#define __LIBGENS_MD_VDPREG_HPP__

#include <stdint.h>

namespace LibGens
{

namespace VdpTypes
{
	// VDP registers.
	union VdpReg_t
	{
		uint8_t reg[24];
		struct m5_t
		{
			/**
			 * Mode 5 (MD) registers.
			 *
			 * DISP == Display Enable. (1 == on; 0 == off)
			 * IE0 == Enable V interrupt. (1 == on; 0 == off)
			 * IE1 == Enable H interrupt. (1 == on; 0 == off)
			 * IE2 == Enable external interrupt. (1 == on; 0 == off)
			 * M1 == DMA Enable. (1 == on; 0 == off)
			 * M2 == V cell mode. (1 == V30 [PAL only]; 0 == V28)
			 * M3 == HV counter latch. (1 == stop HV counter; 0 == enable read, H, V counter)
			 * M4/PSEL == Palette Select; if clear, masks high two bits of each CRam color component.
			 *            If M5 is off, acts like M4 instead of PSEL.
			 * M5 == Mode 4/5 toggle; set for Mode 5, clear for Mode 4.
			 * LCB == Left Column Blank. SMS VDP holdover; if set, masks the first 8 pixels
			 *        with the background color.
			 * VSCR == V Scroll mode. (0 == full; 1 == 2-cell)
			 * HSCR/LSCR == H Scroll mode. (00 == full; 01 == invalid; 10 == 1-cell; 11 == 1-line)
			 * RS0/RS1 == H cell mode. (11 == H40; 00 == H32; others == invalid)
			 * LSM1/LSM0 == Interlace mode. (00 == normal; 01 == interlace mode 1; 10 == invalid; 11 == interlace mode 2)
			 * S/TE == Highlight/Shadow enable. (1 == on; 0 == off)
			 * VSZ1/VSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
			 * HSZ1/HSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
			 */
			uint8_t Set1;		// Mode Set 1. [   0    0   LCB  IE1    0 PSEL   M3    0]
			uint8_t Set2;		// Mode Set 2. [   0 DISP   IE0   M1   M2   M5    0    0]
			uint8_t Pat_ScrA_Adr;	// Pattern name table base address for Scroll A.
			uint8_t Pat_Win_Adr;	// Pattern name table base address for Window.
			uint8_t Pat_ScrB_Adr;	// Pattern name table base address for Scroll B.
			uint8_t Spr_Att_Adr;	// Sprite Attribute Table base address.
			uint8_t Reg6;		// unused
			uint8_t BG_Color;	// Background color.
			uint8_t Reg8;		// unused
			uint8_t Reg9;		// unused
			uint8_t H_Int;		// H Interrupt.
			uint8_t Set3;		// Mode Set 3. [   0    0    0    0  IE2 VSCR HSCR LSCR]
			uint8_t Set4;		// Mode Set 4. [ RS0    0    0    0 S/TE LSM1 LSM0  RS1]
			uint8_t H_Scr_Adr;	// H Scroll Data Table base address.
			uint8_t Reg14;		// unused
			uint8_t Auto_Inc;	// Auto Increment.
			uint8_t Scr_Size;	// Scroll Size. [   0    0 VSZ1 VSZ0    0    0 HSZ1 HSZ0]
			uint8_t Win_H_Pos;	// Window H position.
			uint8_t Win_V_Pos;	// Window V position.
			uint8_t DMA_Length_L;	// DMA Length Counter Low.
			uint8_t DMA_Length_H;	// DMA Length Counter High.
			uint8_t DMA_Src_Adr_L;	// DMA Source Address Low.
			uint8_t DMA_Src_Adr_M;	// DMA Source Address Mid.
			uint8_t DMA_Src_Adr_H;	// DMA Source Address High.
		};
		m5_t m5;
		struct m4_t
		{
			/**
			* Mode 4 (SMS) registers.
			* NOTE: Mode 4 is currently not implemented.
			* This is here for future use.
			*
			* On SMS1, address bits with asterisks are bitwise-AND'ed
			* with the requested cell address. On SMS2/GG, these bits
			* are ignored.
			*
			* TODO: Add register descriptions.
			*/
			uint8_t Set1;		// Mode Set 1. [ VSI  HSI  LCB  IE1   SS   M4   M3   ES]
			uint8_t Set2;		// Mode Set 2. [   1 DISP  IE0   M1   M2    0   SZ  MAG]
			uint8_t NameTbl_Addr;	// Name table base address. [0 0 0 0 A13 A12 A11 *A10]
			uint8_t ColorTbl_Addr;	// Color table base address.
			uint8_t	Pat_BG_Addr;	// Background Pattern Generator base address.
			uint8_t Spr_Att_Addr;	// Sprite Attribute Table base address. [0 A13 A12 A11 A10 A9 A8 *A7]
			uint8_t Spr_Pat_addr;	// Sprite Pattern Generator base address. [0 0 0 0 0 A13 *A12 *A11]
			uint8_t BG_Color;	// Background color. [0 0 0 0 BG3 BG2 BG1 BG0]
			uint8_t H_Scroll;	// Horizontal scroll. [8-bit]
			uint8_t V_Scroll;	// Vertical scroll. [8-bit]
			uint8_t H_Int;		// H Interrupt. [8-bit]
		};
		m4_t m4;
	};
}

}

#endif /* __LIBGENS_MD_VDPREG_HPP__ */
