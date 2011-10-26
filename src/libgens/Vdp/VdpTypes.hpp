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

#ifndef __LIBGENS_MD_VDPTYPES_HPP__
#define __LIBGENS_MD_VDPTYPES_HPP__

#include <stdint.h>

namespace LibGens
{

namespace VdpTypes
{
	/**
	 * VRam: Video RAM.
	 * SMS/GG: 16 KB.
	 * MD: 64 KB. (32 KW)
	 */
	union VRam_t
	{
		uint8_t  u8[64*1024];
		uint16_t u16[(64*1024)>>1];
		uint32_t u32[(64*1024)>>2];
	};

	/**
	 * CRam: Color RAM.
	 * SMS: 32 bytes.
	 * MD: 128 bytes. (64 words)
	 * GG: 64 bytes. (32 words)
	 */
	union CRam_t
	{
		uint8_t  u8[64<<1];
		uint16_t u16[64];
		uint32_t u32[64>>1];
	};

	/**
	 * VSRam: Vertical Scroll RAM.
	 * MD: 40 words.
	 */
	union VSRam_t
	{
		uint8_t  u8[40<<1];
		uint16_t u16[40];
		
		uint8_t  reserved[128];		// TODO: Figure out how to remove this.
	};
	
	// VDP registers.
	union VdpReg_t
	{
		uint8_t reg[24];
		struct m5_t
		{
			/**
			 * Mode 5 (MD) registers.
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
			 * LMSK == Left column mask. SMS VDP holdover; if set, masks the first 8 pixels
			 *         with the background color.
			 * VSCR == V Scroll mode. (0 == full; 1 == 2-cell)
			 * HSCR/LSCR == H Scroll mode. (00 == full; 01 == invalid; 10 == 1-cell; 11 == 1-line)
			 * RS0/RS1 == H cell mode. (11 == H40; 00 == H32; others == invalid)
			 * LSM1/LSM0 == Interlace mode. (00 == normal; 01 == interlace mode 1; 10 == invalid; 11 == interlace mode 2)
			 * S/TE == Highlight/Shadow enable. (1 == on; 0 == off)
			 * VSZ1/VSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
			 * HSZ1/HSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
			 */
			uint8_t Set1;		// Mode Set 1.  [   0    0  LMSK  IE1    0 PSEL   M3    0]
			uint8_t Set2;		// Mode Set 2.  [   0 DISP   IE0   M1   M2   M5    0    0]
			uint8_t Pat_ScrA_Adr;	// Pattern name table base address for Scroll A.
			uint8_t Pat_Win_Adr;	// Pattern name table base address for Window.
			uint8_t Pat_ScrB_Adr;	// Pattern name table base address for Scroll B.
			uint8_t Spr_Att_Adr;	// Sprite Attribute Table base address.
			uint8_t Reg6;		// unused
			uint8_t BG_Color;	// Background color.
			uint8_t Reg8;		// unused
			uint8_t Reg9;		// unused
			uint8_t H_Int;		// H Interrupt.
			uint8_t Set3;		// Mode Set 3.  [   0    0    0    0  IE2 VSCR HSCR LSCR]
			uint8_t Set4;		// Mode Set 4.  [ RS0    0    0    0 S/TE LSM1 LSM0  RS1]
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
			*/
			uint8_t Set1;		// Mode Set 1.
			uint8_t Set2;		// Mode Set 2.
			uint8_t NameTbl_Addr;	// Name table base address.
			uint8_t ColorTbl_Addr;	// Color table base address.
			uint8_t	Pat_BG_Addr;	// Background Pattern Generator base address.
			uint8_t Spr_Att_Addr;	// Sprite Attribute Table base address.
			uint8_t Spr_Pat_addr;	// Sprite Pattern Generator base address.
			uint8_t BG_Color;	// Background color.
			uint8_t H_Scroll;	// Horizontal scroll.
			uint8_t V_Scroll;	// Vertical scroll.
			uint8_t H_Int;		// H Interrupt.
		};
		m4_t m4;
	};
	
	/**
	 * Interlaced display mode. (See VdpReg_t.m5.Set4)
	 * Source: http://wiki.megadrive.org/index.php?title=VDPRegs_Addendum (Jorge)
	 */
	enum Interlaced_t
	{
		/**
		 * Interlaced mode is off. [LSM1 LSM0] == [0 0]
		 */
		INTERLACED_OFF		= 0,
		
		/**
		 * Interlaced Mode 1. [LSM1 LSM0] == [0 1]
		 * The display is interlaced, but the image
		 * is exactly the same as INTERLACED_OFF.
		 */
		INTERLACED_MODE_1	= 1,	// [LSM1 LSM0] == [0 1]
		
		/**
		 * Interlaced mode is off. [LSM1 LSM0] = [1 0]
		 * Although LSM1 is set, the screen is still non-interlaced,
		 * and the image is regular resolution.
		 */
		INTERLACED_OFF2		= 2,
		
		/**
		 * Interlaced Mode 2. [LSM1 LSM0] = [1 1]
		 * The display is interlaced, and the vertical resolution
		 * is doubled. (x448, x480)
		 */
		INTERLACED_MODE_2	= 3,
	};
	
	/**
	 * Interlaced rendering mode.
	 * This controls the way INTERLACED_MODE_2 is rendered onscreen.
	 * TODO: Make Interlaced_t and IntRend_Mode_t less confusing.
	 */
	enum IntRend_Mode_t
	{
		INTREND_EVEN	= 0,	// Even lines only. (Old Gens)
		INTREND_ODD	= 1,	// Odd lines only.
		INTREND_FLICKER	= 2,	// Alternating fields. (Flickering Interlaced)
		INTREND_2X	= 3,	// 2x Resolution. (TODO)
	};
	
	// VDP line counters.
	// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
	struct VdpLines_t
	{
		/** Total lines using NTSC/PAL line numbering. **/
		struct Display_t
		{
			unsigned int Total;	// Total number of lines on the display. (262, 313)
			unsigned int Current;	// Current display line.
		};
		Display_t Display;
		
		/** Visible lines using VDP line numbering. **/
		struct Visible_t
		{
			int Total;		// Total number of visible lines. (192, 224, 240)
			int Current;		// Current visible line. (May be negative for top border.)
			int Border_Size;	// Size of the border. (192 lines == 24; 224 lines == 8)
		};
		Visible_t Visible;
		
		/** NTSC V30 handling. **/
		struct NTSC_V30_t
		{
			int Offset;		// Current NTSC V30 roll offset.
			int VBlank_Div;		// VBlank divider. (0 == VBlank is OK; 1 == no VBlank allowed)
		};
		NTSC_V30_t NTSC_V30;
	};
	
	// Update flags.
	union UpdateFlags_t
	{
		uint8_t flags;
		struct
		{
			bool VRam	:1;	// VRam was modified. (Implies VRam_Spr.)
			bool VRam_Spr	:1;	// Sprite Attribute Table was modified.
		};
	};
	
	// VDP emulation options.
	struct VdpEmuOptions_t
	{
		// Interlaced rendering mode.
		IntRend_Mode_t intRendMode;
		
		/**
		 * Enables border color emulation.
		 * If true, draws the background color in the screen borders.
		 * Otherwise, the screen borders default to black.
		 */
		bool borderColorEmulation;
		
		/**
		 * Enables "rolling" graphics in V30 on NTSC.
		 * This simulates the effect seen on an NTSC Genesis
		 * if 240-line mode is enabled.
		 */
		bool ntscV30Rolling;
		
		/**
		 * Enables zero-length DMA.
		 * Default is false (hardware-accurate).
		 * May need to be enabled for buggy hacks.
		 */
		bool zeroLengthDMA;
		
		/**
		 * Enables sprite limits.
		 * Default is true (hardware-accurate).
		 * May need to be disabled for buggy hacks.
		 */
		bool spriteLimits;
		
		/**
		 * Enables left-column VScroll bug emulation.
		 * Options:
		 * - false: disabled. (Majesco Genesis 3)
		 * - true:  enabled.  (MD1, MD2) [default]
		 * FIXME: Not implemented at the moment!
		 */
		bool vscrollBug;
	};
	
	// VDP layer flags.
	enum VdpLayerFlags
	{
		VDP_LAYER_SCROLLA_LOW		= (1 << 0),
		VDP_LAYER_SCROLLA_HIGH		= (1 << 1),
		VDP_LAYER_SCROLLA_SWAP		= (1 << 2),
		VDP_LAYER_SCROLLB_LOW		= (1 << 3),
		VDP_LAYER_SCROLLB_HIGH		= (1 << 4),
		VDP_LAYER_SCROLLB_SWAP		= (1 << 5),
		VDP_LAYER_SPRITE_LOW		= (1 << 6),
		VDP_LAYER_SPRITE_HIGH		= (1 << 7),
		VDP_LAYER_SPRITE_SWAP		= (1 << 8),
		VDP_LAYER_SPRITE_ALWAYSONTOP	= (1 << 9),
		VDP_LAYER_PALETTE_LOCK		= (1 << 10),
	};
	
	static const unsigned int VDP_LAYERS_DEFAULT =
		(VDP_LAYER_SCROLLA_LOW	|
			VDP_LAYER_SCROLLA_HIGH	|
			VDP_LAYER_SCROLLB_LOW	|
			VDP_LAYER_SCROLLB_HIGH	|
			VDP_LAYER_SPRITE_LOW	|
			VDP_LAYER_SPRITE_HIGH);
}

}

#endif /* __LIBGENS_MD_VDPTYPES_HPP__ */
