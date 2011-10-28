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
