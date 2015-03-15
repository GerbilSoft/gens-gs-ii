/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpReg.hpp: VDP registers.                                              *
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

/**
 * References:
 * - "VDP 128Kb Extended VRAM mode"
 *   - http://gendev.spritesmind.net/forum/viewtopic.php?t=1368
 */

#ifndef __LIBGENS_MD_VDPREG_HPP__
#define __LIBGENS_MD_VDPREG_HPP__

#include <stdint.h>

namespace LibGens
{

namespace VdpTypes
{
	// VDP registers.
	union VdpReg_t {
		uint8_t reg[24];
		struct {
			/**
			 * Mode 5 (MD) registers.
			 */

			/**
			 * Register 0: Mode Set 1.
			 * [   0    0   LCB  IE1    0 PSEL   M3    0]
			 * 
			 * LCB: Left Column Blank. SMS VDP leftover; if set, masks the first 8 pixels
			 *      with the background color.
			 * IE1: Enable H interrupt. (1 == on; 0 == off)
			 * M4/PSEL: Palette Select. If clear, masks high two bits of each CRam color component.
			 *          If M5 is off, acts like M4 instead of PSEL.
			 * M3: HV counter latch. (1 == stop HV counter; 0 == enable read, H, V counter)
			 */
			uint8_t Set1;

			/**
			 * Register 1: Mode Set 2.
			 * [128K DISP   IE0   M1   M2   M5    0    0]
			 * 
			 * 128K: Extended VRAM mode. (1 == on; 0 == off)
			 * DISP: Display Enable. (1 == on; 0 == off)
			 * IE0: Enable V interrupt. (1 == on; 0 == off)
			 * M1: DMA Enable. (1 == on; 0 == off)
			 * M2: V resolution. (1 == V30 [PAL only]; 0 == V28)
			 * M5: Mode 4/5 toggle. (1 == M5; 0 == M4)
			 */
			uint8_t Set2;

			/**
			 * Register 2: Pattern name table base address for Scroll A.
			 * [   x    x SA15 SA14 SA13    x    x    x]
			 */
			uint8_t Pat_ScrA_Adr;

			/**
			 * Register 3: Pattern name table base address for Window.
			 * [   x    x WD15 WD14 WD13 WD12 WD11    x]
			 */
			uint8_t Pat_Win_Adr;

			/**
			 * Register 4: Pattern name table base address for Scroll B.
			 * [   x    x    x    x    x SB15 SB14 SB13]
			 */
			uint8_t Pat_ScrB_Adr;

			/**
			 * Register 5: Sprite Attribute Table base address.
			 * [   x AT15 AT14 AT13 AT12 AT11 AT10  AT9]
			 */
			uint8_t Spr_Att_Adr;

			/**
			 * Register 6: Sprite Pattern Generator base address.
			 * NOTE: ONLY used in 128 KB VRAM mode!
			 * [   x    x AP16    x    x    x    x    x]
			 */
			uint8_t Spr_Pat_Adr;

			/**
			 * Register 7: Background color.
			 * [   x    x CPT1 CPT0 COL3 COL2 COL1 COL0]
			 */
			uint8_t BG_Color;

			/**
			 * Register 8: Unused.
			 */
			uint8_t Reg8;

			/**
			 * Register 9: Unused.
			 */
			uint8_t Reg9;

			/**
			 * Register 10: H Interrupt register.
			 * [HIT7 HIT6 HIT5 HIT4 HIT3 HIT2 HIT1 HIT0]
			 */
			uint8_t H_Int;

			/**
			 * Register 11: Mode Set 3.
			 * [   0    0    0    0  IE2 VSCR HSCR LSCR]
			 * 
			 * IE2: Enable external interrupt. (1 == on; 0 == off)
			 * VSCR: V Scroll mode. (0 == full; 1 == 2-cell)
			 * HSCR/LSCR: H Scroll mode.
			 *   - 00 == full
			 *   - 01 == invalid (first 8 entries used for whole screen)
			 *   - 10 == per-cell
			 *   - 11 == per-line
			 */
			uint8_t Set3;

			/**
			 * Register 12: Mode Set 4.
			 * [ RS0    0    0    0 S/TE LSM1 LSM0  RS1]
			 * 
			 * RS0/RS1: H resolution. (00 == H32; 11 == H40; others == invalid)
			 * S/TE: Shadow/Highlight enable. (1 == on; 0 == off)
			 * LSM1/LSM0: Interlace mode.
			 *   - 00 == normal
			 *   - 01 == interlace mode 1
			 *   - 10 == invalid (acts like 00)
			 *   - 11 == interlace mode 2
			 */
			uint8_t Set4;

			/**
			 * Register 13: H Scroll Data Table base address.
			 * [   x    x HS15 HS14 HS13 HS12 HS11 HS10]
			 */
			uint8_t H_Scr_Adr;

			/**
			 * Register 14: Pattern data base address.
			 * NOTE: ONLY used in 128 KB VRAM mode!
			 * [   x    x    x PB16    x    x    x PA16]
			 *
			 * PA16: When set, layer A is rebased to the upper 64 KB.
			 * PB16: When this and PA16 are set, layer B is rebased to the upper 64KB.
			 */
			uint8_t Pat_Data_Adr;

			/**
			 * Register 15: Auto Increment Data.
			 * [INC7 INC6 INC5 INC4 INC3 INC2 INC1 INC0]
			 */
			uint8_t Auto_Inc;

			/**
			 * Register 16: Scroll Size.
			 * [   0    0 VSZ1 VSZ2    0    0 HSZ1 HSZ0]
			 * 
			 * VSZ1/VSZ2: Vertical scroll size.
			 * HSZ1/HSZ2: Horizontal scroll size.
			 * 
			 * Scroll sizes:
			 * - 00 == 32 cells
			 * - 01 == 64 cells
			 * - 10 == invalid
			 * - 11 == 128 cells
			 */
			uint8_t Scr_Size;

			/**
			 * Register 17: Window H position.
			 * [RIGT    0    0 WHP5 WHP4 WHP3 WHP2 WHP1]
			 */
			uint8_t Win_H_Pos;

			/**
			 * Register 18: Window V position.
			 * [DOWN    0    0 WVP5 WVP4 WVP3 WVP2 WVP1]
			 */
			uint8_t Win_V_Pos;

			/**
			 * Registers 19, 20: DMA Length counter.
			 */
			uint8_t DMA_Length_L;
			uint8_t DMA_Length_H;

			/**
			 * Registers 21, 22, 23: DMA Source address.
			 */
			uint8_t DMA_Src_Adr_L;
			uint8_t DMA_Src_Adr_M;
			uint8_t DMA_Src_Adr_H;
		} m5;

		struct {
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
			uint8_t Set2;		// Mode Set 2. [DRAM DISP  IE0   M1   M2    0   SZ  MAG]
			uint8_t NameTbl_Adr;	// Name table base address. [0 0 0 0 A13 A12 A11 *A10]
			uint8_t ColorTbl_Adr;	// Color table base address.
			uint8_t	Pat_BG_Adr;	// Background Pattern Generator base address.
			uint8_t Spr_Att_Adr;	// Sprite Attribute Table base address. [0 A13 A12 A11 A10 A9 A8 *A7]
			uint8_t Spr_Pat_Adr;	// Sprite Pattern Generator base address. [0 0 0 0 0 A13 *A12 *A11]
			uint8_t BG_Color;	// Background color. [0 0 0 0 BG3 BG2 BG1 BG0]
			uint8_t H_Scroll;	// Horizontal scroll. [8-bit]
			uint8_t V_Scroll;	// Vertical scroll. [8-bit]
			uint8_t H_Int;		// H Interrupt. [8-bit]
		} m4;
	};

	/**
	  * VDP Codeword register.
	  * MD: [CD5 CD4 CD3 CD2 CD1 CD0]
	  * - CD5: DMA enable.
	  * - CD4: Busy.
	  * - CD3-CD1: Destination.
	  * - CD0: Read/Write. (0 = read, 1 = write)
	  */
	enum VDP_CodeWord {
		// CD5: DMA enable.
		CD_DMA_ENABLE		= (1 << 5),

		// CD4: Done. (if 0, VDP has work to do)
		CD_DONE			= (1 << 4),

		// CD3-1: Destination.
		// NOTE: CRAM has different destinations
		// depending on read/write mode.
		CD_DEST_VRAM		= 0x00,
		CD_DEST_REGISTER_INT_W	= 0x02, // CD0=0
		CD_DEST_CRAM_INT_W	= 0x02, // CD0=1
		CD_DEST_VSRAM		= 0x04,
		CD_DEST_CRAM_INT_R	= 0x08,
		CD_DEST_VRAM_8BIT	= 0x0C,	// undocumented
		CD_DEST_MASK		= 0x0E,

		// CD0: Read/write.
		CD_MODE_READ		= 0x00,
		CD_MODE_WRITE		= 0x01,
		CD_MODE_MASK		= 0x01,

		// CD3-0 combined.
		CD_DEST_VRAM_READ	= (CD_DEST_VRAM | CD_MODE_READ),
		CD_DEST_VRAM_WRITE	= (CD_DEST_VRAM | CD_MODE_WRITE),
		CD_DEST_CRAM_READ	= (CD_DEST_CRAM_INT_R | CD_MODE_READ),
		CD_DEST_CRAM_WRITE	= (CD_DEST_CRAM_INT_W | CD_MODE_WRITE),
		CD_DEST_VSRAM_READ	= (CD_DEST_VSRAM | CD_MODE_READ),
		CD_DEST_VSRAM_WRITE	= (CD_DEST_VSRAM | CD_MODE_WRITE),
		// NOTE: REGISTER_WRITE looks like a READ, but it isn't!
		CD_DEST_REGISTER_WRITE	= (CD_DEST_REGISTER_INT_W | CD_MODE_READ),
		CD_DEST_MODE_MASK	= 0x0F,
		CD_DEST_MODE_CD4_MASK	= 0x1F,
	};
}

}

#endif /* __LIBGENS_MD_VDPREG_HPP__ */
