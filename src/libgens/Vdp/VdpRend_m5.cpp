/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_m5.cpp: VDP Mode 5 rendering code. (Part of the Vdp class.)     *
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

#include "Vdp.hpp"
#include "VdpTypes.hpp"

// M68K_Mem::ms_Region is needed for region detection.
#include "cpu/M68K_Mem.hpp"

// C includes. (C++ namespace)
#include <cstring>

// ARRAY_SIZE(x)
#include "macros/common.h"

// TODO: Maybe move these to class enum constants?
#define LINEBUF_HIGH_B	0x80
#define LINEBUF_SHAD_B	0x40
#define LINEBUF_PRIO_B	0x01
#define LINEBUF_SPR_B	0x20
#define LINEBUF_WIN_B	0x02

#define LINEBUF_HIGH_W	0x8080
#define LINEBUF_SHAD_W	0x4040
#define LINEBUF_PRIO_W	0x0100
#define LINEBUF_SPR_W	0x2000
#define LINEBUF_WIN_W	0x0200

// Tile pixel positions.
#include "Util/byteswap.h"

#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define TILE_PX0	0x0000F000
#define TILE_PX1	0x00000F00
#define TILE_PX2	0x000000F0
#define TILE_PX3	0x0000000F
#define TILE_PX4	0xF0000000
#define TILE_PX5	0x0F000000
#define TILE_PX6	0x00F00000
#define TILE_PX7	0x000F0000
#define TILE_SHIFT0	12
#define TILE_SHIFT1	8
#define TILE_SHIFT2	4
#define TILE_SHIFT3	0
#define TILE_SHIFT4	28
#define TILE_SHIFT5	24
#define TILE_SHIFT6	20
#define TILE_SHIFT7	16
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
#define TILE_PX0	0xF0000000
#define TILE_PX1	0x0F000000
#define TILE_PX2	0x00F00000
#define TILE_PX3	0x000F0000
#define TILE_PX4	0x0000F000
#define TILE_PX5	0x00000F00
#define TILE_PX6	0x000000F0
#define TILE_PX7	0x0000000F
#define TILE_SHIFT0	28
#define TILE_SHIFT1	24
#define TILE_SHIFT2	20
#define TILE_SHIFT3	16
#define TILE_SHIFT4	12
#define TILE_SHIFT5	8
#define TILE_SHIFT6	4
#define TILE_SHIFT7	0
#endif

namespace LibGens
{

/**
 * Get the current line number, adjusted for interlaced display.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @return Line number.
 */
template<bool interlaced>
FORCE_INLINE int Vdp::T_GetLineNumber(void) const
{
	// Get the current line number.
	int vdp_line = VDP_Lines.currentLine;
	
	if (interlaced) {
		// Adjust the VDP line number for Flickering Interlaced display.
		vdp_line *= 2;

		switch (options.intRendMode) {
			case VdpTypes::INTREND_EVEN:
			default:
				// Even lines only.
				// Don't do anything.
				break;

			case VdpTypes::INTREND_ODD:
				// Odd lines only.
				vdp_line++;
				break;

			case VdpTypes::INTREND_FLICKER:
				// Flickering Interlaced mode.
				if (Reg_Status.isOddFrame())
					vdp_line++;
				break;
		}
	}

	return vdp_line;
}

/**
 * Vdp::T_PutPixel_P0(): Put a pixel in background graphics layer 0. (low-priority)
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
FORCE_INLINE void Vdp::T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// Check if this is a transparent pixel.
	if (!(pattern & mask))
		return;
	
	// Check the layer bits of the current pixel.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	uint8_t layer_bits = LineBuf.px[LineBuf_pixnum].layer;
	
	// Check if this pixel is masked.
	if (plane && (layer_bits & (LINEBUF_PRIO_B | LINEBUF_WIN_B)))
	{
		// Scroll A: Either the pixel has priority set,
		// or the pixel is a window pixel.
		return;
	}
	
	// Shift the pattern data.
	uint8_t pat8 = (pattern >> shift) & 0x0F;
	
	// Apply palette data.
	pat8 |= palette;
	
	// If Highlight/Shadow is enabled, adjust the shadow flags.
	if (h_s)
	{
		// Scroll A: Mark as shadow if the layer is marked as shadow.
		// Scroll B: Always mark as shadow.
		if (plane)
			pat8 |= (layer_bits & LINEBUF_SHAD_B);
		else
			pat8 |= LINEBUF_SHAD_B;
	}
	
	// Write the new pixel to the line buffer.
	LineBuf.px[LineBuf_pixnum].pixel = pat8;
}


/**
 * Vdp::T_PutPixel_P1(): Put a pixel in background graphics layer 1. (high-priority)
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
FORCE_INLINE void Vdp::T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return;
	
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	
	if (plane)
	{
		// Scroll A: If the pixel is a Window pixel, don't do anything.
		if (LineBuf.px[LineBuf_pixnum].layer & LINEBUF_WIN_B)
			return;
	}
	
	// Shift the pixel.
	px >>= shift;
	
	// Update the pixel:
	// - Add palette information.
	// - Mark the pixel as priority.
	// - Save it to the linebuffer.
	px |= palette | LINEBUF_PRIO_W;
	LineBuf.u16[LineBuf_pixnum] = (uint16_t)px;
}


/**
 * Vdp::T_PutPixel_Sprite(): Put a pixel in the sprite layer.
 * @param priority	[in] Sprite priority.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param pat_pixnum	[in] Pattern pixel number.
 * @param mask		[in] Mask to isolate the good pixel.
 * @param shift		[in] Shift.
 * @param disp_pixnum	[in] Display pixel number.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 * @return Linebuffer byte.
 */
template<bool priority, bool h_s, int pat_pixnum, uint32_t mask, int shift>
FORCE_INLINE uint8_t Vdp::T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return 0;
	
	// Get the pixel number in the linebuffer.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum + 8);
	uint8_t layer_bits = LineBuf.px[LineBuf_pixnum].layer;
	
	if (layer_bits & ((LINEBUF_PRIO_B | LINEBUF_SPR_B) - priority))
	{
		// Priority bit is set. (TODO: Is that what this means?)
		if (!priority)
		{
			// Set the sprite bit in the linebuffer.
			LineBuf.px[LineBuf_pixnum].layer |= LINEBUF_SPR_B;
		}
		
		// Return the original linebuffer priority data.
		return layer_bits;
	}
	
	// Shift the pixel and apply the palette.
	px = ((px >> shift) | palette);
	
	if (h_s)
	{
		// Highlight/Shadow enabled.
		if (px == 0x3E)
		{
			// Palette 3, color 14: Highlight. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum] |= LINEBUF_HIGH_W;
			return 0;
		}
		else if (px == 0x3F)
		{
			// Palette 3, color 15: Shadow. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum] |= LINEBUF_SHAD_W;
			return 0;
		}
		
		// Apply highlight/shadow based on priority.
		if (!priority)
		{
			// Low priority. Pixel can be normal, shadowed, or highlighted.
			layer_bits &= (LINEBUF_SHAD_B | LINEBUF_HIGH_B);
			
			if ((px & 0x0F) == 0x0E)
			{
				// Color 14 in palettes 0-2 are never shadowed.
				layer_bits &= ~LINEBUF_SHAD_B;
			}
		}
		else
		{
			// High priority. Pixel can either be normal or highlighted.
			layer_bits &= LINEBUF_HIGH_B;
		}
		
		// Apply the layer bits.
		px |= layer_bits;
	}
	
	// Mark the pixel as a sprite pixel.
	px |= LINEBUF_SPR_W;
	
	// Save the pixel in the linebuffer.
	LineBuf.u16[LineBuf_pixnum] = px;
	
	return 0;
}


#define LINEBUF_HIGH_D	0x80808080
#define LINEBUF_SHAD_D	0x40404040
#define LINEBUF_PRIO_D	0x01000100
#define LINEBUF_SPR_D	0x20002000
#define LINEBUF_WIN_D	0x02000200


/**
 * Vdp::T_PutLine_P0(): Put a line in background graphics layer 0. (low-priority)
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
FORCE_INLINE void Vdp::T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane)
	{
		// Scroll B.
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLB_LOW))
			return;
	}
	else
	{
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLA_LOW))
			return;
	}
	
	// Don't do anything if the pattern is empty.
	if (pattern == 0)
		return;
	
	// Put the pixels.
	if (!flip)
	{
		// No flip.
		T_PutPixel_P0<plane, h_s, 0, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 1, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 2, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 3, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 4, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 5, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 6, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 7, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		T_PutPixel_P0<plane, h_s, 0, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 1, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 2, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 3, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 4, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 5, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 6, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 7, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
	}
}


/**
 * Vdp::T_PutLine_P1(): Put a line in background graphics layer 1. (high-priority)
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
FORCE_INLINE void Vdp::T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane)
	{
		// Scroll B.
		// Clear the line.
		memset(&LineBuf.u16[disp_pixnum], 0x00, 8*2);
		
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLB_LOW))
			return;
	}
	else
	{
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLA_LOW))
			return;
		
		// AND the linebuffer with ~LINEBUF_SHAD_W.
		// TODO: Optimize this to use 32-bit operations instead of 16-bit.
		LineBuf.u16[disp_pixnum]   &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+1] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+2] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+3] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+4] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+5] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+6] &= ~LINEBUF_SHAD_W;
		LineBuf.u16[disp_pixnum+7] &= ~LINEBUF_SHAD_W;
	}
	
	// Don't do anything if the pattern is empty.
	if (pattern == 0)
		return;
	
	// Put the pixels.
	if (!flip)
	{
		// No flip.
		T_PutPixel_P1<plane, h_s, 0, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 1, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 2, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 3, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 4, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 5, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 6, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 7, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		T_PutPixel_P1<plane, h_s, 0, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 1, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 2, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 3, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 4, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 5, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 6, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 7, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
	}
}


/**
 * Vdp::T_PutLine_Sprite(): Put a line in the sprite layer.
 * @param priority	[in] Sprite priority. (false == low, true == high)
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool priority, bool h_s, bool flip>
FORCE_INLINE void Vdp::T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette)
{
	// Check if the sprite layer is disabled.
	if (!(VDP_Layers & (priority ? VdpTypes::VDP_LAYER_SPRITE_HIGH : VdpTypes::VDP_LAYER_SPRITE_LOW)))
	{
		// Sprite layer is disabled.
		return;
	}
	
	// Put the sprite pixels.
	uint8_t status = 0;
	if (!flip)
	{
		// No flip.
		status |= T_PutPixel_Sprite<priority, h_s, 0, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 1, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 2, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 3, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 4, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 5, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 6, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 7, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
	}
	else
	{
		// Horizontal flip.
		status |= T_PutPixel_Sprite<priority, h_s, 0, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 1, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 2, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 3, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 4, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 5, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 6, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 7, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
	}
	
	// Check for sprite collision.
	if (status & LINEBUF_SPR_B)
		Reg_Status.setBit(VdpStatus::VDP_STATUS_COLLISION, true);
}

/**
 * Get the X offset for the line. (Horizontal Scroll Table)
 * @param plane True for Scroll A; false for Scroll B.
 * @return X offset.
 */
template<bool plane>
FORCE_INLINE uint16_t Vdp::T_Get_X_Offset(void)
{
	// NOTE: Multiply by 4 for 16-bit access.
	// * 2 == select A/B; * 2 == 16-bit
	const unsigned int H_Scroll_Offset = (VDP_Lines.currentLine & H_Scroll_Mask) * 4;

	if (plane) {
		// Scroll A.
		return H_Scroll_Addr_u16(H_Scroll_Offset) & 0x3FF;
	} else {
		// Scroll B.
		return H_Scroll_Addr_u16(H_Scroll_Offset + 2) & 0x3FF;
	}
}

/**
 * Get the Y offset.
 * @param plane True for Scroll A; false for Scroll B.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param cell_cur Current X cell number.
 * @return Y offset, in pixels. (Includes cell offset and fine offset.)
 */
template<bool plane, bool interlaced>
FORCE_INLINE unsigned int Vdp::T_Get_Y_Offset(int cell_cur)
{
	// NOTE: Cell offset masking is handled in T_Get_Y_Cell_Offset().
	// We don't need to do it here.
	unsigned int y_offset;

	if (cell_cur < 0 || cell_cur >= 40) {
		// Cell number is invalid.
		// This usually happens if 2-cell VScroll is used
		// at the same time as HScroll.
		if (options.vscrollBug) {
			/**
			 * VScroll bug is enabled. (MD1, MD2)
			 * 
			 * H32: VScroll is fixed to 0.
			 * - Test ROM: Kawasaki Superbike Challenge
			 * 
			 * H40: Result is VSRam.u16[38] & VSRam.u16[39].
			 * That is, column 19 from both planes A and B, ANDed together.
			 * This is used for both scroll planes.
			 * - Test ROM: Oerg's MDEM 2011 demo. (without masking sprites)
			 *   - Video demonstrating the VScroll bug:
			 *     http://www.youtube.com/watch?v=UsW8i7zsY8w
			 *
			 * References:
			 * - Sik on #GensGS.
			 * - http://gendev.spritesmind.net/forum/viewtopic.php?p=11728#11728
			 */

			if (isH40())
				y_offset = (VSRam.u16[38] & VSRam.u16[39]);
			else // H32
				y_offset = 0;

			// Add the current line number to the Y offset.
			y_offset += T_GetLineNumber<interlaced>();
			return y_offset;
		} else {
			/**
			 * VScroll bug is disabled. (MD3)
			 * Handle this column the same as column 0.
			 */
			cell_cur = 0;
		}
	}

	// Mask off odd columns.
	cell_cur &= ~1;

	// Plane A VScroll is stored on even addresses.
	// Plane B VScroll is stored on odd addresses.
	if (!plane)
		cell_cur++;

	// Get the Y offset.
	y_offset = VSRam.u16[cell_cur];

	// Add the current line number to the Y offset.
	y_offset += T_GetLineNumber<interlaced>();
	return y_offset;
}

/**
 * Get the cell offset from a Y offset.
 * This function applies the VScroll cell mask.
 * @param y_offset Y offset.
 * @param vscroll_mask If true, mask with the VScroll cell mask.
 * @return Cell offset.
 */
template<bool interlaced, bool vscroll_mask>
FORCE_INLINE unsigned int Vdp::T_Get_Y_Cell_Offset(unsigned int y_offset)
{
	// Non-Interlaced: 8x8 cells
	// Interlaced: 8x16 cells
	unsigned int cell_offset;
	if (!interlaced)
		cell_offset = (y_offset >> 3);
	else
		cell_offset = (y_offset >> 4);

	if (vscroll_mask)
		cell_offset &= V_Scroll_CMask;
	
	return cell_offset;
}

/**
 * Get the fine offset from a Y offset.
 * @param y_offset Y offset.
 * @return Fine offset.
 */
template<bool interlaced>
FORCE_INLINE unsigned int Vdp::T_Get_Y_Fine_Offset(unsigned int y_offset)
{
	// Non-Interlaced: 8x8 cells
	// Interlaced: 8x16 cells
	if (!interlaced)
		return (y_offset & 7);
	else
		return (y_offset & 15);
}

/**
 * Get pattern info from a scroll plane.
 * H_Scroll_CMul must be initialized correctly.
 * @param plane True for Scroll A; false for Scroll B.
 * @param x X tile number.
 * @param y Y tile number.
 * @return Pattern info.
 */
template<bool plane>
FORCE_INLINE uint16_t Vdp::T_Get_Pattern_Info(unsigned int x, unsigned int y)
{
	// Get the offset.
	// H_Scroll_CMul is the shift value required for the proper vertical offset.
	// NOTE: Multiply by 2 for 16-bit access.
	const unsigned int offset = ((y << H_Scroll_CMul) + x) * 2;

	// Return the pattern information.
	return (plane ? ScrA_Addr_u16(offset) : ScrB_Addr_u16(offset));
}

/**
 * Get pattern data for a given tile for the current line.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param pattern Pattern info.
 * @param y_fine_offset Y fine offset.
 * @return Pattern data.
 */
template<bool interlaced>
FORCE_INLINE uint32_t Vdp::T_Get_Pattern_Data(uint16_t pattern, unsigned int y_fine_offset)
{
	// Get the tile address.
	unsigned int TileAddr;
	if (interlaced)
		TileAddr = (pattern & 0x3FF) << 6;
	else
		TileAddr = (pattern & 0x7FF) << 5;

	if (pattern & 0x1000) {
		// V Flip enabled. Flip the tile vertically.
		if (interlaced)
			y_fine_offset ^= 15;
		else
			y_fine_offset ^= 7;
	}

	// Return the pattern data.
	return VRam.u32[(TileAddr + (y_fine_offset * 4)) >> 2];
}

/**
 * Render a scroll line.
 * @param plane		[in] True for Scroll A / Window; false for Scroll B.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param cell_start	[in] (Scroll A) First cell to draw.
 * @param cell_length	[in] (Scroll A) Number of cells to draw.
 */
template<bool plane, bool interlaced, bool vscroll, bool h_s>
FORCE_INLINE void Vdp::T_Render_Line_Scroll(int cell_start, int cell_length)
{
	// Get the horizontal scroll offset. (cell and fine offset)
	unsigned int x_cell_offset = T_Get_X_Offset<plane>();

	// Drawing will start at the fine cell offset.
	// LineBuf.u16[X_offset_cell & 7]
	unsigned int disp_pixnum = (x_cell_offset & 7);

	// Determine if we should apply the Left Window bug.
	int LeftWindowBugCnt = 0;	// Left Window bug counter.
	if (plane && (cell_start != 0)) {
		// Determine the value for the Left Window bug counter.
		// First tile: Counter should be 2.
		// Second tile: Counter should be 1.
		LeftWindowBugCnt = ((x_cell_offset & 8) ? 2 : 1);
	}

	if (plane) {
		// Adjust for the cell starting position.
		const int cell_start_px = (cell_start << 3);
		x_cell_offset -= cell_start_px;
		disp_pixnum += cell_start_px;
	}

	// Get the correct cell offset:
	// - Invert the cell position.
	// - Right-shift by 3 for the cell number.
	// - AND with the horizontal scrolling cell mask to prevent overflow.
	x_cell_offset = (((x_cell_offset ^ 0x3FF) >> 3) & H_Scroll_CMask);

	// VSRam cell number.
	// TODO: Adjust for left-window?
	// NOTE: This starts at -1 or -2, since we're rendering within the 336px buffer.
	// (Rendering starts from 0 to 7 px off the left side of the screen.)
	int VSRam_Cell = ((x_cell_offset & 1) - 2);

	// Initialize the Y offset.
	unsigned int y_offset, y_cell_offset, y_fine_offset;
	if (!vscroll) {
		// Full vertical scrolling.
		// Initialize the Y offset here.
		// NOTE: We're using 0 instead of (VSRam_Cell + 2),
		// since VSRam_Cell will be either -2 or -1 here.
		// T_Update_Y_Offset() ANDs the result with ~1, so
		// the resulting value will always be 0.
		y_offset = T_Get_Y_Offset<plane, interlaced>(0);
		y_cell_offset = T_Get_Y_Cell_Offset<interlaced, true>(y_offset);
		y_fine_offset = T_Get_Y_Fine_Offset<interlaced>(y_offset);
	}

	// Loop through the cells.
	for (int x = cell_length; x >= 0; x--, VSRam_Cell++) {
		if (vscroll) {
			// 2-cell vertical scrolling.
			// Update the Y offset.
			y_offset = T_Get_Y_Offset<plane, interlaced>(VSRam_Cell);
			y_cell_offset = T_Get_Y_Cell_Offset<interlaced, true>(y_offset);
			y_fine_offset = T_Get_Y_Fine_Offset<interlaced>(y_offset);
		}

		// Get the pattern info for the current tile.
		uint16_t pattern_info;
		if (!plane) {
			// Scroll B.
			pattern_info = T_Get_Pattern_Info<plane>(x_cell_offset, y_cell_offset);
		} else {
			// Scroll A. Check if we need to emulate the Left Window bug.
			if (LeftWindowBugCnt <= 0) {
				// Left Window bug doesn't apply or has already been emulated.
				pattern_info = T_Get_Pattern_Info<plane>(x_cell_offset, y_cell_offset);
			} else {
				// Left Window bug applies.
				LeftWindowBugCnt--;
				const unsigned int TmpXCell = ((x_cell_offset + 2) & H_Scroll_CMask);
				pattern_info = T_Get_Pattern_Info<plane>(TmpXCell, y_cell_offset);
			}
		}

		// Get the pattern data for the current tile.
		uint32_t pattern_data = T_Get_Pattern_Data<interlaced>(pattern_info, y_fine_offset);

		// Extract the palette number.
		// Resulting number is palette * 16.
		unsigned int palette = (pattern_info >> 9) & 0x30;

		// Check for swapped Scroll B priority.
		if (VDP_Layers & VdpTypes::VDP_LAYER_SCROLLB_SWAP)
			pattern_info ^= 0x8000;

		// Check for horizontal flip.
		if (pattern_info & 0x0800) {
			// Pattern has H-Flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<plane, h_s, true>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, true>(disp_pixnum, pattern_data, palette);
		} else {
			// Pattern doesn't have flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<plane, h_s, false>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, false>(disp_pixnum, pattern_data, palette);
		}

		// Go to the next H cell.
		x_cell_offset = (x_cell_offset + 1) & H_Scroll_CMask;

		// Go to the next pattern.
		disp_pixnum += 8;
	}
}

/**
 * Render a line for Scroll A / Window.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool vscroll, bool h_s>
FORCE_INLINE void Vdp::T_Render_Line_ScrollA_Window(void)
{
	// Cell counts for Scroll A.
	int ScrA_Start, ScrA_Length;
	int Win_Start, Win_Length = 0;
	
	// Check if the entire line is part of the window.
	// TODO: Verify interlaced operation!
	const int vdp_cells = (VDP_Lines.currentLine >> 3);
	if (VDP_Reg.m5.Win_V_Pos & 0x80) {
		// Window starts from the bottom.
		if (vdp_cells >= Win_Y_Pos) {
			// Current line is >= starting line.
			// Entire line is part of the window.
			ScrA_Start = 0;
			ScrA_Length = 0;
			Win_Start = 0;
			Win_Length = GetHCells();
		}
	} else if (vdp_cells < Win_Y_Pos) {
		// Current line is < ending line.
		// Entire line is part of the window.
		ScrA_Start = 0;
		ScrA_Length = 0;
		Win_Start = 0;
		Win_Length = GetHCells();
	}

	if (Win_Length == 0) {
		// Determine the cell starting position and length.
		if (VDP_Reg.m5.Win_H_Pos & 0x80) {
			// Window is right-aligned.
			ScrA_Start = 0;
			ScrA_Length = Win_X_Pos;
			Win_Start = Win_X_Pos;
			Win_Length = (GetHCells() - Win_X_Pos);
		} else {
			// Window is left-aligned.
			Win_Start = 0;
			Win_Length = Win_X_Pos;
			ScrA_Start = Win_X_Pos;
			ScrA_Length = (GetHCells() - Win_X_Pos);
		}
	}

	if (Win_Length > 0) {
		// Draw the window.

		// Drawing will start at the first window cell.
		// (Window is not scrollable.)
		unsigned int disp_pixnum = (Win_Start * 8) + 8;

		// Calculate the Y offsets.
		const int y_offset = T_GetLineNumber<interlaced>();
		unsigned int y_cell_offset, y_fine_offset;

		// Non-Interlaced: 8x8 cells
		// Interlaced: 8x16 cells
		y_cell_offset = T_Get_Y_Cell_Offset<interlaced, false>(y_offset);
		y_fine_offset = T_Get_Y_Fine_Offset<interlaced>(y_offset);

		// TODO: See if we need to handle address wraparound.
		// NOTE: Multiply by 2 for 16-bit access.
		const uint16_t *Win_Row_Addr = Win_Addr_Ptr16((y_cell_offset << H_Win_Shift) * 2) + Win_Start;

		// Loop through the cells.
		for (int x = Win_Length; x > 0; x--, disp_pixnum += 8) {
			// Get the pattern info and data for the current tile.
			register uint16_t pattern_info = *Win_Row_Addr++;
			uint32_t pattern_data = T_Get_Pattern_Data<interlaced>(pattern_info, y_fine_offset);

			// Extract the palette number.
			// Resulting number is palette * 16.
			unsigned int palette = (pattern_info >> 9) & 0x30;

			// Check for swapped Scroll A priority.
			if (VDP_Layers & VdpTypes::VDP_LAYER_SCROLLA_SWAP)
				pattern_info ^= 0x8000;

			// Check for horizontal flip.
			if (pattern_info & 0x0800) {
				// Pattern has H-Flip enabled.
				if (pattern_info & 0x8000)
					T_PutLine_P1<true, h_s, true>(disp_pixnum, pattern_data, palette);
				else
					T_PutLine_P0<true, h_s, true>(disp_pixnum, pattern_data, palette);
			} else {
				// Pattern doesn't have flip enabled.
				if (pattern_info & 0x8000)
					T_PutLine_P1<true, h_s, false>(disp_pixnum, pattern_data, palette);
				else
					T_PutLine_P0<true, h_s, false>(disp_pixnum, pattern_data, palette);
			}
		}

		// Mark window pixels.
		// TODO: Do this in the Window drawing code!
		if (ScrA_Length > 0) {
			const int StartPx = ((Win_Start * 8) + 8) / 2;
			const int EndPx = StartPx + ((Win_Length * 8) / 2);

			for (int x = StartPx; x < EndPx; x++)
				LineBuf.u32[x] |= LINEBUF_WIN_D;
		}
	}

	if (ScrA_Length > 0) {
		// Draw the scroll area.
		T_Render_Line_Scroll<true, interlaced, vscroll, h_s>(ScrA_Start, ScrA_Length);
	}
}

/**
 * Fill Sprite_Struct[] with information from the Sprite Attribute Table.
 * @param interlaced If true, using Interlaced Mode 2. (2x res)
 * @param partial If true, only do a partial update. (X pos, X size)
 */
template<bool interlaced, bool partial>
FORCE_INLINE void Vdp::T_Make_Sprite_Struct(void)
{
	uint8_t spr_num = 0;
	uint8_t link = 0;

	// H40 allows 80 sprites; H32 allows 64 sprites.
	// Essentially, it's (GetHCells() * 2).
	// [Nemesis' Sprite Masking and Overflow Test ROM: Test #9]
	// TODO: 80 sprites with Sprite Limit disabled, or 128?
	// (Old Gens limited to 80 sprites regardless of video mode.)
	const unsigned int max_spr = (options.spriteLimits
					? (GetHCells() * 2)
					: (unsigned int)ARRAY_SIZE(Sprite_Struct));

	// Get the first sprite address in VRam.
	const uint16_t *CurSpr = Spr_Addr_Ptr16(0);

	do {
		// Sprite X position and size is updated for all types of updates.

		// Sprite X position.
		Sprite_Struct[spr_num].Pos_X = (CurSpr[3] & 0x1FF) - 128;

		// Sprite size.
		const uint8_t sz = ((CurSpr[1] >> 8) & 0xFF);
		Sprite_Struct[spr_num].Size_X = ((sz >> 2) & 3) + 1;	// 1 more than the original value.

		// Determine the maximum positions.
		Sprite_Struct[spr_num].Pos_X_Max =
				Sprite_Struct[spr_num].Pos_X +
				((Sprite_Struct[spr_num].Size_X * 8) - 1);

		if (!partial) {
			// Full sprite update: Update Y position, size, and tile number.
			Sprite_Struct[spr_num].Size_Y = sz & 3;	// Exactly the original value.

			if (interlaced) {
				// Interlaced mode:
				// * Y position is 11-bit.
				// * Cells are 8x16.
				Sprite_Struct[spr_num].Pos_Y = (CurSpr[0] & 0x3FF) - 256;
				Sprite_Struct[spr_num].Pos_Y_Max =
						Sprite_Struct[spr_num].Pos_Y +
						((Sprite_Struct[spr_num].Size_Y * 16) + 15);
			} else {
				// Non-Interlaced mode:
				// * Y position is 10-bit.
				// * Cells are 8x8.
				Sprite_Struct[spr_num].Pos_Y = (CurSpr[0] & 0x1FF) - 128;
				Sprite_Struct[spr_num].Pos_Y_Max =
						Sprite_Struct[spr_num].Pos_Y +
						((Sprite_Struct[spr_num].Size_Y * 8) + 7);
			}

			// Tile number. (Also includes palette, priority, and flip bits.)
			Sprite_Struct[spr_num].Num_Tile = CurSpr[2];
		}

		// Link field.
		// NOTE: Link field is 7-bit. Usually this won't cause a problem,
		// since most games won't set the high bit.
		// Dino Land incorrectly sets the high bit on some sprites,
		// so we have to mask it off.
		// TODO: Do we update the link field on partial updates?
		link = (CurSpr[1] & 0x7F);

		// Increment the sprite number.
		spr_num++;
		if (link == 0)
			break;

		// Get the next sprite address in VRam.
		// NOTE: Original byte offset needs to be used here.
		// (Spr_Addr_Ptr16() divides by 2 for 16-bit access.)
		CurSpr = Spr_Addr_Ptr16(link * 8);

		// Stop processing after:
		// - Link number is 0. (checked above)
		// - Link number exceeds maximum number of sprites.
		// - We've processed the maximum number of sprites.
	} while (link < max_spr && spr_num < max_spr);

	// Store the total number of sprites.
	if (!partial)
		TotalSprites = spr_num;
}

/**
 * Update Sprite_Visible[] using sprite masking.
 * @param sprite_limit If true, emulates sprite limits.
 * @param interlaced If true, uses interlaced mode.
 * @return Number of visible sprites.
 */
template<bool sprite_limit, bool interlaced>
FORCE_INLINE unsigned int Vdp::T_Update_Mask_Sprite(void)
{
	// If Sprite Limit is on, the following limits are enforced: (H32/H40)
	// - Maximum sprite dots per line: 256/320
	// - Maximum sprites per line: 16/20
	int max_cells = GetHCells();
	int max_sprites = (max_cells / 2);

	bool overflow = false;

	// sprite_on_line is set if at least one sprite is on the scanline
	// that is not a sprite mask (x == 0). Sprite masks are only effective
	// if there is at least one higher-priority sprite on the scanline.
	// Thus, if a sprite mask is the first sprite on the scanline it is ignored.
	// However, if the previous line had a sprite dot overflow, it is *not*
	// ignored, so it is processed as a regular mask.
	bool sprite_on_line = !!SpriteDotOverflow;

	// sprite_mask_active is set if a sprite mask is preventing
	// remaining sprites from showing up on the scanline.
	// Those sprites still count towards total sprite and sprite dot counts.
	bool sprite_mask_active = false;

	uint8_t spr_num = 0;	// Current sprite number in Sprite_Struct[].
	uint8_t spr_vis = 0;	// Current visible sprite in Sprite_Visible[].

	// Get the current line number.
	const int vdp_line = T_GetLineNumber<interlaced>();

	// Search for all sprites visible on the current scanline.
	for (; spr_num < TotalSprites; spr_num++) {
		if (Sprite_Struct[spr_num].Pos_Y > vdp_line ||
		    Sprite_Struct[spr_num].Pos_Y_Max < vdp_line)
		{
			// Sprite is not on the current line.
			continue;
		}

		if (sprite_limit) {
			// Sprite limit is enabled.
			// Decrement the maximum cell and sprite counters.
			max_cells -= Sprite_Struct[spr_num].Size_X;
			max_sprites--;
		}

		// Check for sprite masking.
		if (Sprite_Struct[spr_num].Pos_X == -128) {
			// Sprite mask.
			if (sprite_on_line) {
				// There is at least one higher-priority sprite on the scanline.
				// No more sprites should be visible.
				// However, remaining sprites will still count towards total sprite and sprite dot counts.
				// [Nemesis' Sprite Masking and Overflow Test ROM: Test #5]
				if (!sprite_limit)
					break;
				sprite_mask_active = true;
			}

			// There aren't any higher-priority sprites on the scanline.
			// This sprite will still count towards sprite and sprite dot counts.
		} else {
			// Regular sprite.
			sprite_on_line = true;

			// Check if the sprite is onscreen.
			if (!sprite_mask_active &&
				Sprite_Struct[spr_num].Pos_X < GetHPix() &&
				Sprite_Struct[spr_num].Pos_X_Max >= 0)
			{
				// Sprite is onscreen.
				Sprite_Visible[spr_vis] = spr_num;
				spr_vis++;
			}

			// Set the visible X max.
			Sprite_Struct[spr_num].Pos_X_Max_Vis = Sprite_Struct[spr_num].Pos_X_Max;
		}

		if (sprite_limit) {
			// Check for cell or sprite overflow.
			if (max_cells <= 0) {
				// Cell overflow!
				// Remove the extra cells from the sprite.
				// [Nemesis' Sprite Masking and Overflow Test ROM: Tests #2 and #3]
				// #2 == total sprite dot count; #3 == per-cell dot count.
				// TODO: Verify how Pos_X_Max_Vis should work with regards to H Flip.
				overflow = true;

				// Decrement the displayed number of cells for the sprite.
				Sprite_Struct[spr_num].Pos_X_Max_Vis += (max_cells * 8);
				spr_num++;
				break;
			} else if (max_sprites == 0) {
				// Sprite overflow!
				// [Nemesis' Sprite Masking and Overflow Test ROM: Test #1]
				overflow = true;
				spr_num++;
				break;
			}
		}
	}

	// Update the SpriteDotOverflow value.
	// [Nemesis' Sprite Masking and Overflow Test ROM: Test #6]
	SpriteDotOverflow = (max_cells <= 0);

	if (sprite_limit && overflow) {
		// Sprite overflow. Check if there are any more sprites.
		for (; spr_num < TotalSprites; spr_num++) {
			// Check if the sprite is on the current line.
			if (Sprite_Struct[spr_num].Pos_Y > vdp_line ||
			    Sprite_Struct[spr_num].Pos_Y_Max < vdp_line)
			{
				// Sprite is not on the current line.
				continue;
			}

			// Sprite is on the current line.
			if (--max_sprites < 0) {
				// Sprite overflow!
				// Set the SOVR flag.
				Reg_Status.setBit(VdpStatus::VDP_STATUS_SOVR, true);
				break;
			}
		}
	}

	// Return the number of visible sprites.
	return spr_vis;
}

/**
 * Render a sprite line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
FORCE_INLINE void Vdp::T_Render_Line_Sprite(void)
{
	// Update the sprite masks.
	unsigned int num_spr;
	if (options.spriteLimits)
		num_spr = T_Update_Mask_Sprite<true, interlaced>();
	else
		num_spr = T_Update_Mask_Sprite<false, interlaced>();

	for (unsigned int spr_vis = 0; spr_vis < num_spr; spr_vis++) {
		// Get the sprite number.
		const uint8_t spr_num = Sprite_Visible[spr_vis];

		// Determine the cell and line offsets.
		unsigned int cell_offset = (T_GetLineNumber<interlaced>() - Sprite_Struct[spr_num].Pos_Y);
		unsigned int line_offset;

		if (interlaced) {
			// Interlaced.
			line_offset = (cell_offset & 15);
			cell_offset &= 0x1F0;
		} else {
			// Non-Interlaced.
			line_offset = (cell_offset & 7);
			cell_offset &= 0xF8;
		}

		// Get the Y cell size.
		unsigned int Y_cell_size = Sprite_Struct[spr_num].Size_Y;

		// Get the sprite information.
		// Also, check for swapped sprite layer priority.
		unsigned int spr_info = Sprite_Struct[spr_num].Num_Tile;
		if (VDP_Layers & VdpTypes::VDP_LAYER_SPRITE_SWAP)
			spr_info ^= 0x8000;

		// Get the palette number, multiplied by 16.
		const unsigned int palette = ((spr_info >> 9) & 0x30);

		// Get the pattern number.
		unsigned int tile_num;
		if (interlaced) {
			tile_num = (spr_info & 0x3FF) << 6;	// point on the contents of the pattern
			Y_cell_size <<= 6;	// Size_Y * 64
			cell_offset *= 4;	// Num_Pattern * 64
		} else {
			tile_num = (spr_info & 0x7FF) << 5;	// point on the contents of the pattern
			Y_cell_size <<= 5;	// Size_Y * 32
			cell_offset *= 4;	// Num_Pattern * 32
		}

		// Check for V Flip.
		if (spr_info & 0x1000) {
			// V Flip enabled.
			if (interlaced)
				line_offset ^= 15;
			else
				line_offset ^= 7;

			tile_num += (Y_cell_size - cell_offset);
			if (interlaced) {
				Y_cell_size += 64;
				tile_num += (line_offset * 4);
			} else {
				Y_cell_size += 32;
				tile_num += (line_offset * 4);
			}
		} else {
			// V Flip disabled.
			tile_num += cell_offset;
			if (interlaced) {
				Y_cell_size += 64;
				tile_num += (line_offset * 4);
			} else {
				Y_cell_size += 32;
				tile_num += (line_offset * 4);
			}
		}

		// Check for H Flip.
		register int H_Pos_Min;
		register int H_Pos_Max;

		if (spr_info & 0x800) {
			// H Flip enabled.
			// Check the minimum edge of the sprite.
			H_Pos_Min = Sprite_Struct[spr_num].Pos_X;
			if (H_Pos_Min < -7)
				H_Pos_Min = -7;	// minimum edge = clip screen

			// TODO: Verify how Pos_X_Max_Vis should work with regards to H Flip.
			H_Pos_Max = Sprite_Struct[spr_num].Pos_X_Max_Vis;

			H_Pos_Max -= 7;				// to post the last pattern in first
			while (H_Pos_Max >= GetHPix()) {
				H_Pos_Max -= 8;			// move back to the preceding pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}

			// Draw the sprite.
			if ((VDP_Layers & VdpTypes::VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000)) {
				// High priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8) {
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<true, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			} else {
				// Low priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8) {
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<false, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		} else {
			// H Flip disabled.
			// Check the minimum edge of the sprite.
			H_Pos_Min = Sprite_Struct[spr_num].Pos_X;
			H_Pos_Max = Sprite_Struct[spr_num].Pos_X_Max_Vis;
			if (H_Pos_Max >= GetHPix())
				H_Pos_Max = GetHPix();

			while (H_Pos_Min < -7) {
				H_Pos_Min += 8;			// advance to the next pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}

			// Draw the sprite.
			if ((VDP_Layers & VdpTypes::VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000)) {
				// High priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8)
				{
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<true, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			} else {
				// Low priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8) {
					uint32_t pattern = VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<false, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		}
	}
}

/**
 * Render a line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
FORCE_INLINE void Vdp::T_Render_Line_m5(void)
{
	// Clear the line first.
	memset(&LineBuf, (h_s ? LINEBUF_SHAD_B : 0), sizeof(LineBuf));

	if (VDP_Reg.m5.Set3 & 0x04) {
		// 2-cell VScroll.
		T_Render_Line_Scroll<false, interlaced, true, h_s>(0, GetHCells());	// Scroll B
		T_Render_Line_ScrollA_Window<interlaced, true, h_s>();			// Scroll A
	} else {
		// Full VScroll.
		T_Render_Line_Scroll<false, interlaced, false, h_s>(0, GetHCells());	// Scroll B
		T_Render_Line_ScrollA_Window<interlaced, false, h_s>();			// Scroll A
	}

	T_Render_Line_Sprite<interlaced, h_s>();
}


/**
 * Render the line buffer to the destination surface.
 * @param pixel Type of pixel.
 * @param dest Destination surface.
 * @param md_palette MD palette buffer.
 */
template<typename pixel>
FORCE_INLINE void Vdp::T_Render_LineBuf(pixel *dest, pixel *md_palette)
{
	const LineBuf_t::LineBuf_px_t *src = &LineBuf.px[8];

	// Render the line buffer to the destination surface.
	const int HPixBegin = GetHPixBegin();
	dest += HPixBegin;
	for (int i = H_Cell; i != 0; i--, dest += 8, src += 8) {
		*dest     = md_palette[src->pixel];
		*(dest+1) = md_palette[(src+1)->pixel];
		*(dest+2) = md_palette[(src+2)->pixel];
		*(dest+3) = md_palette[(src+3)->pixel];
		*(dest+4) = md_palette[(src+4)->pixel];
		*(dest+5) = md_palette[(src+5)->pixel];
		*(dest+6) = md_palette[(src+6)->pixel];
		*(dest+7) = md_palette[(src+7)->pixel];
	}

	if (HPixBegin == 0)
		return;

	// Draw the borders.
	// NOTE: S/H is ignored if we're in the border region.

	// Get the border color.
	register const pixel border_color =
		(options.borderColorEmulation ? md_palette[0] : 0);

	// Left border.
	const int HPix = GetHPix();
	dest -= HPixBegin;
	dest -= HPix;
	for (int i = (HPixBegin / 8); i != 0; i--, dest += 8) {
		*dest     = border_color;
		*(dest+1) = border_color;
		*(dest+2) = border_color;
		*(dest+3) = border_color;
		*(dest+4) = border_color;
		*(dest+5) = border_color;
		*(dest+6) = border_color;
		*(dest+7) = border_color;
	}

	// Right border.
	dest += HPix;
	for (int i = (HPixBegin / 8); i != 0; i--, dest += 8) {
		*dest     = border_color;
		*(dest+1) = border_color;
		*(dest+2) = border_color;
		*(dest+3) = border_color;
		*(dest+4) = border_color;
		*(dest+5) = border_color;
		*(dest+6) = border_color;
		*(dest+7) = border_color;
	}
}

/**
 * Apply SMS left-column blanking to the destination surface.
 * @param pixel Type of pixel.
 * @param dest Destination surface.
 * @param border_color Border color.
 */
template<typename pixel>
FORCE_INLINE void Vdp::T_Apply_SMS_LCB(pixel *dest, pixel border_color)
{
	dest += GetHPixBegin();

	*dest     = border_color;
	*(dest+1) = border_color;
	*(dest+2) = border_color;
	*(dest+3) = border_color;
	*(dest+4) = border_color;
	*(dest+5) = border_color;
	*(dest+6) = border_color;
	*(dest+7) = border_color;
}

/**
 * Render a line. (Mode 5)
 */
void Vdp::Render_Line_m5(void)
{
	// Determine what part of the screen we're in.
	bool in_border = false;
	int lineNum = VDP_Lines.currentLine;
	if (lineNum >= VDP_Lines.Border.borderStartBottom &&
	    lineNum <= VDP_Lines.Border.borderEndBottom)
	{
		// Bottom border.
		in_border = true;
	}
	else if (lineNum >= VDP_Lines.Border.borderStartTop &&
	         lineNum <= VDP_Lines.Border.borderEndTop)
	{
		// Top border.
		in_border = true;
		lineNum -= VDP_Lines.Border.borderStartTop;
		lineNum -= VDP_Lines.Border.borderSize;
	}
	else if (VDP_Lines.currentLine >= VDP_Lines.totalVisibleLines) {
		// Off screen.
		return;
	}

	// Determine the starting line in MD_Screen.
	if (Reg_Status.isNtsc() &&
	    (VDP_Reg.m5.Set2 & 0x08) &&
	    options.ntscV30Rolling)
	{
		// NTSC V30 mode. Simulate screen rolling.
		lineNum -= VDP_Lines.NTSC_V30.Offset;

		// Prevent underflow.
		if (lineNum < 0)
			lineNum += 240;
	}
	lineNum += VDP_Lines.Border.borderSize;

	if (in_border && !options.borderColorEmulation) {
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (m_palette.bpp() != VdpPalette::BPP_32) {
			memset(MD_Screen->lineBuf16(lineNum), 0x00,
				(MD_Screen->pxPerLine() * sizeof(uint16_t)));
		} else {
			memset(MD_Screen->lineBuf32(lineNum), 0x00,
				(MD_Screen->pxPerLine() * sizeof(uint32_t)));
		}

		// ...and we're done here.
		return;
	}

	// Check if the VDP is enabled.
	if (!(VDP_Reg.m5.Set2 & 0x40) || in_border) {
		// VDP is disabled, or this is the border region.
		// Clear the line buffer.

		// NOTE: S/H is ignored if the VDP is disabled or if
		// we're in the border region.
		memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));

		// Clear the sprite dot overflow variable.
		SpriteDotOverflow = 0;
	} else {
		// VDP is enabled.

		// Check if sprite structures need to be updated.
		if (Interlaced == VdpTypes::INTERLACED_MODE_2) {
			// Interlaced Mode 2. (2x resolution)
			if (m_updateFlags.VRam)
				T_Make_Sprite_Struct<true, false>();
			else if (m_updateFlags.VRam_Spr)
				T_Make_Sprite_Struct<true, true>();
		} else {
			// Non-Interlaced.
			if (m_updateFlags.VRam)
				T_Make_Sprite_Struct<false, false>();
			else if (m_updateFlags.VRam_Spr)
				T_Make_Sprite_Struct<false, true>();
		}

		// Clear the VRam flags.
		m_updateFlags.VRam = false;
		m_updateFlags.VRam_Spr = false;

		// Determine how to render the image.
		int RenderMode = ((VDP_Reg.m5.Set4 & 0x08) >> 2);		// Shadow/Highlight
		RenderMode |= (Interlaced == VdpTypes::INTERLACED_MODE_2);	// Interlaced.
		switch (RenderMode & 3) {
			case 0:
				// H/S disabled; normal display.
				T_Render_Line_m5<false, false>();
				break;
			case 1:
				// H/S disabled: Interlaced Mode 2.
				T_Render_Line_m5<true, false>();
				break;
			case 2:
				// H/S enabled; normal display.
				T_Render_Line_m5<false, true>();
				break;
			case 3:
				// H/S enabled: Interlaced Mode 2.
				T_Render_Line_m5<true, true>();
				break;
			default:
				// to make gcc shut up
				break;
		}
	}

	// Update the active palette.
	if (!(VDP_Layers & VdpTypes::VDP_LAYER_PALETTE_LOCK)) {
		if (!options.updatePaletteInVBlankOnly || in_border) {
			m_palette.update();
		}
	}

	// Render the image.
	// TODO: Optimize SMS LCB handling. (maybe use Linux's unlikely() macro?)
	if (m_palette.bpp() != VdpPalette::BPP_32) {
		uint16_t *lineBuf16 = MD_Screen->lineBuf16(lineNum);
		T_Render_LineBuf<uint16_t>(lineBuf16, m_palette.m_palActive.u16);

		if (VDP_Reg.m5.Set1 & 0x20) {
			// SMS left-column blanking bit is set.
			// FIXME: Should borderColorEmulation apply here?
			T_Apply_SMS_LCB<uint16_t>(lineBuf16, 
				(options.borderColorEmulation ? m_palette.m_palActive.u16[0] : 0));
		}
	} else {
		uint32_t *lineBuf32 = MD_Screen->lineBuf32(lineNum);
		T_Render_LineBuf<uint32_t>(lineBuf32, m_palette.m_palActive.u32);

		if (VDP_Reg.m5.Set1 & 0x20) {
			// SMS left-column blanking bit is set.
			// FIXME: Should borderColorEmulation apply here?
			T_Apply_SMS_LCB<uint32_t>(lineBuf32, 
				(options.borderColorEmulation ? m_palette.m_palActive.u32[0] : 0));
		}
	}
}

// TODO: 32X stuff.
#if 0
/**
 * Render the 32X line buffer to the destination surface.
 * @param pixel Type of pixel.
 * @param dest Destination surface.
 * @param md_palette MD palette buffer.
 * @param _32X_Rend_Mode 32X rendering mode.
 * @param _32X_palette 32X palette buffer.
 * @param _32X_vdp_cram_adjusted 32X adjusted CRam.
 */
#include "port/timer.h"
template<typename pixel>
static FORCE_INLINE void T_Render_LineBuf_32X(pixel *dest, pixel *md_palette,
			int _32X_Rend_Mode, pixel *_32X_palette, pixel *_32X_vdp_cram_adjusted)
{
	int VRam_Ind = ((_32X_VDP.State & 1) << 16);
	VRam_Ind += _32X_VDP_Ram.u16[VRam_Ind + VDP_Lines.Visible.Current];
	
	// Get the line buffer pointer.
	LineBuf_px_t *lbptr = &LineBuf.px[8];
	
	// Adjust the destination pointer for the horizontal resolution.
	// TODO: Draw horizontal borders, if necessary.
	dest += GetHPixBegin();
	
	// Pixel registers.
	register unsigned int px1, px2;
	
	// Old pixel registers.
	// TODO: Eliminate these!
	unsigned char pixC;
	unsigned short pixS;
	
	// The following 32X games use H32 for the SEGA screen:
	// - Mortal Kombat II
	// - Primal Rage
	// - Sangokushi IV
	
	switch (_32X_Rend_Mode)
	{
		case 0:
		case 4:
		case 8:
		case 12:
			//POST_LINE_32X_M00;
			for (unsigned int px = GetHPix(); px != 0; px -= 4, dest += 4, lbptr += 4)
			{
				*dest = md_palette[lbptr->pixel];
				*(dest+1) = md_palette[(lbptr+1)->pixel];
				*(dest+2) = md_palette[(lbptr+2)->pixel];
				*(dest+3) = md_palette[(lbptr+3)->pixel];
			}
			
			break;
		
		case 1:	// POST_LINE_32X_M01
		case 9: // POST_LINE_32X_SM01
		{
			// TODO: Endianness conversions.
			const uint8_t *src = &_32X_VDP_Ram.u8[VRam_Ind << 1];
			for (unsigned int px = GetHPix(); px != 0; px -= 2, src += 2, dest += 2, lbptr += 2)
			{
				// NOTE: Destination pixels are swapped.
				px1 = *src;
				px2 = *(src+1);
				
				if ((_32X_VDP_CRam[px2] & 0x8000) || !(lbptr->pixel & 0x0F))
					*dest = _32X_vdp_cram_adjusted[px2];
				else
					*dest = md_palette[lbptr->pixel];
				
				if ((_32X_VDP_CRam[px1] & 0x8000) || !((lbptr+1)->pixel & 0x0F))
					*(dest+1) = _32X_vdp_cram_adjusted[px1];
				else
					*(dest+1) = md_palette[(lbptr+1)->pixel];
			}
			break;
		}
		
		case 2:
		case 10:
		{
			//POST_LINE_32X_M01;
			const uint16_t *src = &_32X_VDP_Ram.u16[VRam_Ind];
			for (unsigned int px = GetHPix(); px != 0; px -= 2, src += 2, dest += 2, lbptr += 2)
			{
				// NOTE: Destination pixels are NOT swapped.
				px1 = *src;
				px2 = *(src+1);
				
				if ((px1 & 0x8000) || !(lbptr->pixel & 0x0F))
					*dest = _32X_palette[px1];
				else
					*dest = md_palette[lbptr->pixel];
				
				if ((px2 & 0x8000) || !((lbptr+1)->pixel & 0x0F))
					*(dest+1) = _32X_palette[px2];
				else
					*(dest+1) = md_palette[(lbptr+1)->pixel];
			}
			break;
		}
		
		case 3:
		case 7:
		case 11:
		case 15:
		{
			// POST_LINE_32X_M11
			// This appears to be a form of RLE compression.
			int px = 0;
			int px_end;
			const uint8_t *src = &_32X_VDP_Ram.u8[VRam_Ind << 1];
			while (px < GetHPix())
			{
#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
				px1 = _32X_vdp_cram_adjusted[*src];
				px_end = px + *(src+1);
#else //GSFT_BYTEORDER == GSFT_BIG_ENDIAN
				px1 = _32X_vdp_cram_adjusted[*(src+1)];
				px_end = px + *src;
#endif
				src += 2;
				
				// Make sure it doesn't go out of bounds.
				if (px_end >= GetHPix())
					px_end = (GetHPix() - 1);
				
				for (; px <= px_end; px++)
				{
					dest[px] = px1;
				}
			}
			break;
		}
		
		case 5:
		{
			//POST_LINE_32X_M01_P;
			// TODO: Endianness conversions.
			const uint8_t *src = &_32X_VDP_Ram.u8[VRam_Ind << 1];
			for (unsigned int px = GetHPix(); px != 0; px -= 2, src += 2, dest += 2, lbptr += 2)
			{
				// NOTE: Destination pixels are swapped.
				px1 = *src;
				px2 = *(src+1);
				
				if ((_32X_VDP_CRam[px2] & 0x8000) && (lbptr->pixel & 0x0F))
					*dest = md_palette[lbptr->pixel];
				else
					*dest = _32X_vdp_cram_adjusted[px2];
				
				if ((_32X_VDP_CRam[px1] & 0x8000) && ((lbptr+1)->pixel & 0x0F))
					*(dest+1) = md_palette[(lbptr+1)->pixel];
				else
					*(dest+1) = _32X_vdp_cram_adjusted[px1];
			}
			break;
		}
			
		case 6:
		case 14:
			//POST_LINE_32X_M10_P;
			// TODO: Optimize this!
			for (unsigned int px = GetHPix(); px != 0; px--, dest++, lbptr++)
			{
				pixS = _32X_VDP_Ram.u16[VRam_Ind++];
				
				if (!(pixS & 0x8000) && (lbptr->pixel & 0x0F))
					*dest = _32X_palette[pixS];
				else
					*dest = md_palette[lbptr->pixel];
			}
			break;
		
		case 13:
			//POST_LINE_32X_SM01_P;
			// TODO: Optimize this!
			// TODO: Endianness conversions.
			VRam_Ind *= 2;
			for (unsigned int px = GetHPix(); px != 0; px--, dest++, lbptr++)
			{
				pixC = _32X_VDP_Ram.u8[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				
				if ((pixS & 0x8000) && (lbptr->pixel & 0x0F))
					*dest = md_palette[lbptr->pixel];
				else
					*dest = _32X_vdp_cram_adjusted[pixC];
			}
			break;
		
		default:
			// Invalid.
			break;
	}
}

/**
 * VDP_Render_Line_m5_32X(): Render a line. (Mode 5, 32X)
 */
void VDP_Render_Line_m5_32X(void)
{
	// Determine what part of the screen we're in.
	bool in_border = false;
	if (VDP_Lines.Visible.Current >= -VDP_Lines.Visible.Border_Size &&
	    VDP_Lines.Visible.Current < 0)
	{
		// Top border.
		in_border = true;
	}
	else if (VDP_Lines.Visible.Current >= VDP_Lines.Visible.Total &&
		 VDP_Lines.Visible.Current < (VDP_Lines.Visible.Total + VDP_Lines.Visible.Border_Size))
	{
		// Bottom border.
		in_border = true;
	}
	else if (VDP_Lines.Visible.Current < -VDP_Lines.Visible.Border_Size ||
		 VDP_Lines.Visible.Current >= (VDP_Lines.Visible.Total + VDP_Lines.Visible.Border_Size))
	{
		// Off screen.
		return;
	}
	
	// Determine the starting line in MD_Screen.
	int LineStart = VDP_Lines.Visible.Current;
	if ((CPU_Mode == 0) && (VDP_Reg.m5.Set2 & 0x08) && Video.ntscV30rolling)
	{
		// NTSC V30 mode. Simulate screen rolling.
		LineStart -= VDP_Lines.NTSC_V30.Offset;
		
		// Prevent underflow.
		if (LineStart < 0)
			LineStart += 240;
	}
	LineStart = TAB336[LineStart + VDP_Lines.Visible.Border_Size] + 8;
	
	if (in_border && !Video.borderColorEmulation)
	{
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (bppMD == 32)
			memset(&MD_Screen.u32[LineStart], 0x00, GetHPix()*sizeof(uint32_t));
		else
			memset(&MD_Screen.u16[LineStart], 0x00, GetHPix()*sizeof(uint16_t));
		
		// ...and we're done here.
		return;
	}
	
	// Check if the VDP is enabled.
	if (!(VDP_Reg.m5.Set2 & 0x40) || in_border)
	{
		// VDP is disabled, or this is the border region.
		// Clear the line buffer.
		
		// NOTE: S/H is ignored if the VDP is disabled or if
		// we're in the border region.
		memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));
	}
	else
	{
		// VDP is enabled.
		
		// Check if sprite structures need to be updated.
		if (VDP_Reg.Interlaced.DoubleRes)
		{
			// Interlaced.
			if (VDP_Flags.VRam)
				T_Make_Sprite_Struct<true, false>();
			else if (VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<true, true>();
		}
		else
		{
			// Non-Interlaced.
			if (VDP_Flags.VRam)
				T_Make_Sprite_Struct<false, false>();
			else if (VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<false, true>();
		}
		
		// Clear the VRam flags.
		VDP_Flags.VRam = 0;
		VDP_Flags.VRam_Spr = 0;
		
		// Determine how to render the image.
		const int RenderMode = ((VDP_Reg.m5.Set4 & 0x08) >> 2) | VDP_Reg.Interlaced.DoubleRes;
		switch (RenderMode & 3)
		{
			case 0:
				// H/S disabled; interlaced disabled.
				T_Render_Line_m5<false, false>();
				break;
			case 1:
				// H/S disabled: interlaced enabled.
				T_Render_Line_m5<true, false>();
				break;
			case 2:
				// H/S enabled; interlaced disabled.
				T_Render_Line_m5<false, true>();
				break;
			case 3:
				// H/S enabled: interlaced enabled.
				T_Render_Line_m5<true, true>();
				break;
			default:
				// to make gcc shut up
				break;
		}
	}
	
	// Check if the palette was modified.
	if (VDP_Flags.CRam)
	{
		// Update the palette.
		if (VDP_Reg.m5.Set4 & 0x08)
			VDP_Update_Palette_HS();
		else
			VDP_Update_Palette();
	}
	
	// 32X processing.
	unsigned int eax = (_32X_VDP.Mode);
	unsigned int edx = (eax >> 3) & 0x10;
	unsigned int ebp = (eax >> 11) & 0x20;
	eax &= 0x03;
	edx |= ebp;
	
	// Set the 32X render mode for the 32-bit color C macros.
	int _32X_Rend_Mode = (eax | ((edx >> 2) & 0xFF));
	
	// Render the 32X line.
	if (!in_border)
	{
		if (bppMD != 32)
		{
			T_Render_LineBuf_32X<uint16_t>
						(&MD_Screen.u16[LineStart], MD_Palette.u16,
						_32X_Rend_Mode, _32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
		}
		else
		{
			T_Render_LineBuf_32X<uint32_t>
						(&MD_Screen.u32[LineStart], MD_Palette.u32,
						_32X_Rend_Mode, _32X_Palette.u32, _32X_VDP_CRam_Adjusted.u32);
		}
	}
	else
	{
		// In border. Use standard MD rendering.
		if (bppMD != 32)
			T_Render_LineBuf<uint16_t>(&MD_Screen.u16[LineStart], MD_Palette.u16);
		else
			T_Render_LineBuf<uint32_t>(&MD_Screen.u32[LineStart], MD_Palette.u32);
	}
}
#endif

}
