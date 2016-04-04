/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_m5.cpp: VDP Mode 5 rendering code. (Part of the Vdp class.)     *
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

#include "Vdp.hpp"
#include "VdpTypes.hpp"

// M68K_Mem::ms_Region is needed for region detection.
#include "cpu/M68K_Mem.hpp"

// C includes. (C++ namespace)
#include <cstring>

// ARRAY_SIZE(x)
#include "macros/common.h"

// TODO: Maybe move these to class enum constants?
#define LINEBUF_HIGH_B	0x80	/* Highlighted. */
#define LINEBUF_SHAD_B	0x40	/* Shadowed. */
#define LINEBUF_PRIO_B	0x01	/* High priority. */
#define LINEBUF_SPRSH_B	0x10	/* Sprite pixel is a shadow/highlight operator. */
#define LINEBUF_SPR_B	0x20	/* Sprite pixel. */
#define LINEBUF_WIN_B	0x02	/* Window pixel. */

#define LINEBUF_HIGH_W	0x8080
#define LINEBUF_SHAD_W	0x4040
#define LINEBUF_PRIO_W	0x0100
#define LINEBUF_SPRSH_W	0x1000
#define LINEBUF_SPR_W	0x2000
#define LINEBUF_WIN_W	0x0200

// Tile pixel positions.
#include "libcompat/byteorder.h"

#if SYS_BYTEORDER == SYS_LIL_ENDIAN
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
#else /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
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

// Vdp private class.
#include "Vdp_p.hpp"

namespace LibGens {

/**
 * Get the current line number, adjusted for interlaced display.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @return Line number.
 */
template<bool interlaced>
FORCE_INLINE int VdpPrivate::T_GetLineNumber(void) const
{
	// Get the current line number.
	int vdp_line = q->VDP_Lines.currentLine;

	if (interlaced) {
		// Adjust the VDP line number for Flickering Interlaced display.
		vdp_line *= 2;

		switch (q->options.intRendMode) {
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
 * Put a pixel in background graphics layer 0. (low-priority)
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
FORCE_INLINE void VdpPrivate::T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// Check if this is a transparent pixel.
	if (!(pattern & mask))
		return;

	// Check the layer bits of the current pixel.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	uint8_t layer_bits = LineBuf.px[LineBuf_pixnum].layer;

	// Check if this pixel is masked.
	if (plane && (layer_bits & (LINEBUF_PRIO_B | LINEBUF_WIN_B))) {
		// Scroll A: Either the pixel has priority set,
		// or the pixel is a window pixel.
		return;
	}

	// Shift the pattern data.
	uint8_t pat8 = (pattern >> shift) & 0x0F;

	// Apply palette data.
	pat8 |= palette;

	// If Highlight/Shadow is enabled, adjust the shadow flags.
	if (h_s) {
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
 * Put a pixel in background graphics layer 1. (high-priority)
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
FORCE_INLINE void VdpPrivate::T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return;

	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);

	if (plane) {
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
 * Put a pixel in the sprite layer.
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
FORCE_INLINE uint8_t VdpPrivate::T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return 0;

	// Get the pixel number in the linebuffer.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum + 8);
	uint8_t layer_bits = LineBuf.px[LineBuf_pixnum].layer;

	if (layer_bits & ((LINEBUF_PRIO_B | LINEBUF_SPR_B) - priority)) {
		// Priority bit is set. (TODO: Is that what this means?)
		if (!priority) {
			// Set the sprite bit in the linebuffer.
			LineBuf.px[LineBuf_pixnum].layer |= LINEBUF_SPR_B;
		}

		// Return the original linebuffer priority data.
		return layer_bits;
	} else if (h_s && (layer_bits & LINEBUF_SPRSH_B)) {
		// A sprite shadow/highlight operator has already been applied.
		// This pixel is masked.
		return layer_bits;
	}

	// Shift the pixel and apply the palette.
	px = ((px >> shift) | palette);

	if (h_s) {
		// Shadow/Highlight enabled.
		// NOTE: S/H operators not only mask this sprite,
		// they mask all other sprites as well.
		if (px == 0x3E) {
			// Palette 3, color 14: Highlight. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum] |= (LINEBUF_HIGH_W | LINEBUF_SPRSH_W);
			return 0;
		} else if (px == 0x3F) {
			// Palette 3, color 15: Shadow. (Sprite pixel doesn't show up.)
			LineBuf.u16[LineBuf_pixnum] |= (LINEBUF_SHAD_W | LINEBUF_SPRSH_W);
			return 0;
		}

		// Apply highlight/shadow based on priority.
		if (!priority) {
			// Low priority. Pixel can be normal, shadowed, or highlighted.
			layer_bits &= (LINEBUF_SHAD_B | LINEBUF_HIGH_B);

			if ((px & 0x0F) == 0x0E) {
				// Color 14 in palettes 0-2 are never shadowed.
				layer_bits &= ~LINEBUF_SHAD_B;
			}
		} else {
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
 * Put a line in background graphics layer 0. (low-priority)
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
FORCE_INLINE void VdpPrivate::T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane) {
		// Scroll B.
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLB_LOW))
			return;
	} else {
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLA_LOW))
			return;
	}

	// Don't do anything if the pattern is empty.
	if (pattern == 0)
		return;

	// Put the pixels.
	if (!flip) {
		// No flip.
		T_PutPixel_P0<plane, h_s, 0, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 1, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 2, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 3, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 4, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 5, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 6, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		T_PutPixel_P0<plane, h_s, 7, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
	} else {
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
 * Put a line in background graphics layer 1. (high-priority)
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
FORCE_INLINE void VdpPrivate::T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane) {
		// Scroll B.
		// Clear the line.
		memset(&LineBuf.u16[disp_pixnum], 0x00, 8*2);

		// If ScrollB_Low is disabled, don't do anything.
		if (!(VDP_Layers & VdpTypes::VDP_LAYER_SCROLLB_LOW))
			return;
	} else {
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
	if (!flip) {
		// No flip.
		T_PutPixel_P1<plane, h_s, 0, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 1, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 2, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 3, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 4, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 5, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 6, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		T_PutPixel_P1<plane, h_s, 7, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
	} else {
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
 * Put a line in the sprite layer.
 * @param priority	[in] Sprite priority. (false == low, true == high)
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool priority, bool h_s, bool flip>
FORCE_INLINE void VdpPrivate::T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette)
{
	// Check if the sprite layer is disabled.
	const unsigned int priority_check = (priority
			? VdpTypes::VDP_LAYER_SPRITE_HIGH
			: VdpTypes::VDP_LAYER_SPRITE_LOW);
	if (!(VDP_Layers & priority_check)) {
		// Sprite layer is disabled.
		return;
	}

	// Put the sprite pixels.
	uint8_t status = 0;
	if (!flip) {
		// No flip.
		status |= T_PutPixel_Sprite<priority, h_s, 0, TILE_PX0, TILE_SHIFT0>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 1, TILE_PX1, TILE_SHIFT1>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 2, TILE_PX2, TILE_SHIFT2>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 3, TILE_PX3, TILE_SHIFT3>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 4, TILE_PX4, TILE_SHIFT4>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 5, TILE_PX5, TILE_SHIFT5>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 6, TILE_PX6, TILE_SHIFT6>(disp_pixnum, pattern, palette);
		status |= T_PutPixel_Sprite<priority, h_s, 7, TILE_PX7, TILE_SHIFT7>(disp_pixnum, pattern, palette);
	} else {
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
FORCE_INLINE uint16_t VdpPrivate::T_Get_X_Offset(void)
{
	// NOTE: Multiply by 4 for 16-bit access.
	// * 2 == select A/B; * 2 == 16-bit
	const unsigned int H_Scroll_Offset = (q->VDP_Lines.currentLine & H_Scroll_Mask) * 4;

	if (plane) {
		// Scroll A.
		return H_Scroll_Tbl_Addr_u16(H_Scroll_Offset) & 0x3FF;
	} else {
		// Scroll B.
		return H_Scroll_Tbl_Addr_u16(H_Scroll_Offset + 2) & 0x3FF;
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
FORCE_INLINE unsigned int VdpPrivate::T_Get_Y_Offset(int cell_cur)
{
	// NOTE: Cell offset masking is handled in T_Get_Y_Cell_Offset().
	// We don't need to do it here.
	unsigned int y_offset;

	if (cell_cur < 0 || cell_cur >= 40) {
		// Cell number is invalid.
		// This usually happens if 2-cell VScroll is used
		// at the same time as HScroll.
		if (q->options.vscrollBug) {
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
FORCE_INLINE unsigned int VdpPrivate::T_Get_Y_Cell_Offset(unsigned int y_offset)
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
FORCE_INLINE unsigned int VdpPrivate::T_Get_Y_Fine_Offset(unsigned int y_offset)
{
	// Non-Interlaced: 8x8 cells
	// Interlaced: 8x16 cells
	if (!interlaced)
		return (y_offset & 7);
	else
		return (y_offset & 15);
}

/**
 * Get a nametable word from a scroll plane.
 * H_Scroll_CMul must be initialized correctly.
 * @param plane True for Scroll A; false for Scroll B.
 * @param x X tile number.
 * @param y Y tile number.
 * @return Nametable word.
 */
template<bool plane>
FORCE_INLINE uint16_t VdpPrivate::T_Get_Nametable_Word(unsigned int x, unsigned int y)
{
	// Get the offset.
	// H_Scroll_CMul is the shift value required for the proper vertical offset.
	// NOTE: Multiply by 2 for 16-bit access.
	const unsigned int offset = ((y << H_Scroll_CMul) + x) * 2;

	// Return the pattern information.
	return (plane ? ScrA_Tbl_Addr_u16(offset) : ScrB_Tbl_Addr_u16(offset));
}

/**
 * Get pattern data for a given tile for the current line.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param pattern Pattern info.
 * @param y_fine_offset Y fine offset.
 * @return Pattern data.
 */
template<bool interlaced>
FORCE_INLINE uint32_t VdpPrivate::T_Get_Pattern_Data(uint16_t pattern, unsigned int y_fine_offset)
{
	// Get the tile address.
	unsigned int TileAddr;
	if (interlaced) {
		// FIXME: High bit may be usable for 128 KB mode.
		TileAddr = (pattern & 0x3FF) << 6;
	} else {
		// Non-interlaced, or Interlaced Mode 1.
		TileAddr = (pattern & 0x7FF) << 5;
	}

	if (pattern & 0x1000) {
		// V Flip enabled. Flip the tile vertically.
		if (interlaced) {
			y_fine_offset ^= 15;
		} else {
			y_fine_offset ^= 7;
		}
	}

	// Return the pattern data.
	// FIXME: Rebase to upper 64 KB if necessary. (128 KB VRAM mode)
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
FORCE_INLINE void VdpPrivate::T_Render_Line_Scroll(int cell_start, int cell_length)
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

		// Get the nametable word for the current tile.
		uint16_t nametable_word;
		if (!plane) {
			// Scroll B.
			nametable_word = T_Get_Nametable_Word<plane>(x_cell_offset, y_cell_offset);
		} else {
			// Scroll A. Check if we need to emulate the Left Window bug.
			if (LeftWindowBugCnt <= 0) {
				// Left Window bug doesn't apply or has already been emulated.
				nametable_word = T_Get_Nametable_Word<plane>(x_cell_offset, y_cell_offset);
			} else {
				// Left Window bug applies.
				LeftWindowBugCnt--;
				const unsigned int TmpXCell = ((x_cell_offset + 2) & H_Scroll_CMask);
				nametable_word = T_Get_Nametable_Word<plane>(TmpXCell, y_cell_offset);
			}
		}

		// Get the pattern data for the current tile.
		uint32_t pattern_data = T_Get_Pattern_Data<interlaced>(nametable_word, y_fine_offset);

		// Extract the palette number.
		// Resulting number is palette * 16.
		unsigned int palette = (nametable_word >> 9) & 0x30;

		// Check for swapped Scroll B priority.
		if (VDP_Layers & VdpTypes::VDP_LAYER_SCROLLB_SWAP)
			nametable_word ^= 0x8000;

		// Check for horizontal flip.
		if (nametable_word & 0x0800) {
			// Pattern has H-Flip enabled.
			if (nametable_word & 0x8000)
				T_PutLine_P1<plane, h_s, true>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, true>(disp_pixnum, pattern_data, palette);
		} else {
			// Pattern doesn't have flip enabled.
			if (nametable_word & 0x8000)
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
FORCE_INLINE void VdpPrivate::T_Render_Line_ScrollA_Window(void)
{
	// Cell counts for Scroll A.
	unsigned int ScrA_Start, ScrA_Length;
	int Win_Start, Win_Length = 0;
	
	// Check if the entire line is part of the window.
	// TODO: Verify interlaced operation!
	const unsigned int vdp_cells = (q->VDP_Lines.currentLine >> 3);
	if (VDP_Reg.m5.Win_V_Pos & VDP_REG_M5_WIN_V_DOWN) {
		// Window starts from the bottom.
		if (vdp_cells >= Win_Y_Pos) {
			// Current line is >= starting line.
			// Entire line is part of the window.
			ScrA_Start = 0;
			ScrA_Length = 0;
			Win_Start = 0;
			Win_Length = H_Cell;
		}
	} else if (vdp_cells < Win_Y_Pos) {
		// Current line is < ending line.
		// Entire line is part of the window.
		ScrA_Start = 0;
		ScrA_Length = 0;
		Win_Start = 0;
		Win_Length = H_Cell;
	}

	if (Win_Length == 0) {
		// Determine the cell starting position and length.
		if (VDP_Reg.m5.Win_H_Pos & VDP_REG_M5_WIN_H_RIGT) {
			// Window is right-aligned.
			ScrA_Start = 0;
			ScrA_Length = Win_X_Pos;
			Win_Start = Win_X_Pos;
			Win_Length = (H_Cell - Win_X_Pos);
		} else {
			// Window is left-aligned.
			Win_Start = 0;
			Win_Length = Win_X_Pos;
			ScrA_Start = Win_X_Pos;
			ScrA_Length = (H_Cell - Win_X_Pos);
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
		const uint16_t *Win_Row_Addr = Win_Tbl_Addr_Ptr16((y_cell_offset << H_Win_Shift) * 2) + Win_Start;

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
 * Update the Sprite Line Cache for the next line.
 * Wrapper function to handle interlacing.
 * @param line Current line number, adjusted for IM2.
 */
FORCE_INLINE void VdpPrivate::Update_Sprite_Line_Cache_m5(int line)
{
	unsigned int sovr;
	if (im2_flag) {
		sovr = T_Update_Sprite_Line_Cache_m5<true>(line);
	} else {
		sovr = T_Update_Sprite_Line_Cache_m5<false>(line);
	}

	if (sovr) {
		// Sprite overflow!
		Reg_Status.setBit(VdpStatus::VDP_STATUS_SOVR, true);
	}
}

/**
 * Update the Sprite Line Cache for the next line.
 * @param interlaced If true, using Interlaced Mode 2. (2x res)
 * @param line Current line number, adjusted for IM2.
 * @return VdpStatus::VDP_STATUS_SOVR if sprite limit is exceeded; otherwise, 0.
 */
template<bool interlaced>
unsigned int VdpPrivate::T_Update_Sprite_Line_Cache_m5(int line)
{
	// FIXME: FORCE_INLINE cannot be used here because this function
	// is used in Vdp.cpp. gcc-5.1 fails in release builds due to
	// the function definition not being available there.
	unsigned int ret = 0;
	uint8_t link = 0;

	// Determine the maximum number of sprites.
	// NOTE: Max sprites per frame is always limited
	// due to Sprite Address Table cache masking.
	const uint8_t max_spr_frame = (H_Cell * 2);
	uint8_t max_spr_line;
	if (q->options.spriteLimits) {
		// Sprite limits are enabled:
		// - Max sprites per line:  16 (H32), 20 (H40)
		// - Max sprites per frame: 64 (H32), 80 (H40)
		max_spr_line = (H_Cell / 2);
	} else {
		// Sprite limits are disabled.
		max_spr_line = ARRAY_SIZE(sprLineCache[0]);
	}

	// We're updating the cache for the *next* line.
	int cacheId;
	if (interlaced) {
		if (line < 0) {
			// Offscreen.
			// Set to 0 for even frame, 1 for odd.
			// TODO: Optimize this?
			switch (q->options.intRendMode) {
				case VdpTypes::INTREND_EVEN:
				default:
					line = 0;
					break;
				case VdpTypes::INTREND_ODD:
					line = 1;
					break;
				case VdpTypes::INTREND_FLICKER:
					line = !!(Reg_Status.isOddFrame());
					break;
			}
		} else {
			line += 2;
		}
		cacheId = (line >> 1) & 1;
	} else {
		line++;
		cacheId = line & 1;
	}
	SprLineCache_t *cache = &sprLineCache[cacheId][0];
	uint8_t count = 0;

	/**
	 * The following values are read from the cached
	 * Sprite Attribute Table instead of VRAM:
	 * - Y position
	 * - Sprite size
	 * - Link number
	 */
	const VdpStructs::SprEntry_m5 *spr_SAT = &SprAttrTbl_m5.spr[0];

	// Process up to max_spr_line sprites.
	// (16 in H32, 20 in H40.)
	int total_spr_count = max_spr_frame;
	do {
		// Check the Y position.
		int y = spr_SAT->y;
		if (interlaced) {
			y = (y & 0x3FF) - 256;
		} else {
			y = (y & 0x1FF) - 128;
		}

		if (line >= y) {
			// Calculate the sprite's height.
			const uint8_t sz = spr_SAT->sz;
			int height = (sz & 3);
			if (interlaced) {
				height = (height * 16) + 15;
			} else {
				height = (height * 8) + 7;
			}

			// Check if the bottom of the sprite is in range.
			const int y_max = y + height;	// height is already -1
			if (line <= y_max) {
				// Sprite is in range.
				if (count == max_spr_line) {
					// Sprite overflow!
					ret = VdpStatus::VDP_STATUS_SOVR;
					break;
				}

				// Get the remaining sprite information from VRAM.
				const VdpStructs::SprEntry_m5 *spr_VRam = Spr_Tbl_Addr_PtrM5(link);

				// Save the sprite information in the line cache.
				cache->Pos_X = (spr_VRam->x & 0x1FF) - 128;
				cache->Pos_Y = y;
				// NOTE: Size_? is in units of cells, not pixels.
				cache->Size_X = ((sz >> 2) & 3) + 1;	// 1 more than the original value.
				cache->Size_Y = (sz & 3);		// Exactly the original value.
				// Pos_Y_Max is in units of pixels.
				cache->Pos_Y_Max = y_max;
				// Tile number. (Also includes palette, priority, and flip bits.)
				cache->Num_Tile = spr_VRam->attr;

				// Added a sprite.
				count++;
				cache++;
			}
		}

		// Link field.
		// NOTE: Link field is 7-bit. Usually this won't cause a problem,
		// since most games won't set the high bit.
		// Dino Land incorrectly sets the high bit on some sprites,
		// so we have to mask it off.
		link = spr_SAT->link & 0x7F;
		if (link == 0 || link >= max_spr_frame)
			break;

		// Get the next sprite address in the SAT.
		spr_SAT = &SprAttrTbl_m5.spr[link];
	} while (--total_spr_count);

	// Save the sprite count for the next line.
	sprCountCache[cacheId] = count;

	// Return the SOVR flag.
	return ret;
}

/**
 * Render a sprite line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
FORCE_INLINE void VdpPrivate::T_Render_Line_Sprite(void)
{
	// Current line number, adjusting for Interlaced Mode 2.
	const int line = T_GetLineNumber<interlaced>();

	// Get the sprite line cache for the current line.
	const int cacheId = (interlaced ? (line >> 1) : line) & 1;
	const SprLineCache_t *cache = &sprLineCache[cacheId][0];

	// Pixel count for sprite limit.
	// NOTE: If the user disabled sprite limit, then there's no maximum.
	const unsigned int pixel_count_max =
		(q->options.spriteLimits ? H_Pix : 65536);
	unsigned int pixel_count = 0;

	// Sprite masking.
	// NOTE: Pos_X is screen-relative. Sprite masking is implemented
	// with x == 0, but with screen coordinates, it's x == -128.
	// NOTE 2: If a sprite dot overflow occurred on the previous line,
	// it counts as if this line always has a valid sprite
	bool found_valid_x = sprDotOverflow;	// Found at least one sprite with x > -128.
	bool sprites_masked = false;		// If true, remaining sprites will not be drawn.

	// Process all sprites on this line.
	for (int i = sprCountCache[cacheId]; i > 0; i--, cache++) {
		// Check for a masked sprite.
		// NOTE: Pos_X is screen-relative.
		if (cache->Pos_X > -128) {
			// Found a sprite with x > -128.
			found_valid_x = true;
		} else if (found_valid_x) {
			// We had a sprite with x > -128 already.
			// Mask the rest of the sprites.
			// (NOTE: They still count for sprite dots.)
			sprites_masked = true;
		}

		// Get the X positions.
		int H_Pos_Min = cache->Pos_X;
		int H_Pos_Max = H_Pos_Min + (cache->Size_X * 8) - 1;

		// NOTE: Masked sprites still count towards the sprite dot limit.
		pixel_count += (cache->Size_X * 8);
		if (pixel_count > pixel_count_max) {
			// Sprite dot overflow.
			H_Pos_Max -= (pixel_count - pixel_count_max);
			if (H_Pos_Max < H_Pos_Min)
				break;
		}

		if (sprites_masked) {
			// Sprites are masked.
			// Continue processing sprites,
			// but don't render any.
			continue;
		}

		// Determine the cell and line offsets.
		unsigned int cell_offset = (line - cache->Pos_Y);
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
		unsigned int Y_cell_size = cache->Size_Y;

		// Get the sprite information.
		// Also, check for swapped sprite layer priority.
		uint16_t spr_info = cache->Num_Tile;
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
			tile_num += (Y_cell_size - cell_offset);
			if (interlaced) {
				line_offset ^= 15;
				Y_cell_size += 64;
				tile_num += (line_offset * 4);
			} else {
				line_offset ^= 7;
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
		if (spr_info & 0x800) {
			// H Flip enabled.
			// Check the minimum edge of the sprite.
			if (H_Pos_Min < -7)
				H_Pos_Min = -7;	// minimum edge = clip screen

			H_Pos_Max -= 7;				// to post the last pattern in first
			while (H_Pos_Max >= H_Pix) {
				H_Pos_Max -= 8;			// move back to the preceding pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}

			// Draw the sprite.
			if ((VDP_Layers & VdpTypes::VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000)) {
				// High priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8) {
					uint32_t pattern = Spr_Gen_Addr_u32(tile_num);
					T_PutLine_Sprite<true, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			} else {
				// Low priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8) {
					uint32_t pattern = Spr_Gen_Addr_u32(tile_num);
					T_PutLine_Sprite<false, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		} else {
			// H Flip disabled.
			if (H_Pos_Max >= H_Pix)
				H_Pos_Max = H_Pix;

			while (H_Pos_Min < -7) {
				H_Pos_Min += 8;			// advance to the next pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}

			// Draw the sprite.
			if ((VDP_Layers & VdpTypes::VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000)) {
				// High priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8) {
					uint32_t pattern = Spr_Gen_Addr_u32(tile_num);
					T_PutLine_Sprite<true, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			} else {
				// Low priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8) {
					uint32_t pattern = Spr_Gen_Addr_u32(tile_num);
					T_PutLine_Sprite<false, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		}
	}

	if (pixel_count > pixel_count_max) {
		// Sprite dot overflow.
		sprDotOverflow = true;
	} else {
		// No sprite dot overflow.
		sprDotOverflow = false;
	}
}

/**
 * Render a line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
FORCE_INLINE void VdpPrivate::T_Render_Line_m5(void)
{
	// Clear the line first.
	memset(&LineBuf, (h_s ? LINEBUF_SHAD_B : 0), sizeof(LineBuf));

	if (VDP_Reg.m5.Set3 & VDP_REG_M5_SET3_VSCR) {
		// 2-cell VScroll.
		T_Render_Line_Scroll<false, interlaced, true, h_s>(0, H_Cell);	// Scroll B
		T_Render_Line_ScrollA_Window<interlaced, true, h_s>();		// Scroll A
	} else {
		// Full VScroll.
		T_Render_Line_Scroll<false, interlaced, false, h_s>(0, H_Cell);	// Scroll B
		T_Render_Line_ScrollA_Window<interlaced, false, h_s>();		// Scroll A
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
FORCE_INLINE void VdpPrivate::T_Render_LineBuf(pixel *dest, pixel *md_palette)
{
	const LineBuf_t::LineBuf_px_t *src = &LineBuf.px[8];

	// Render the line buffer to the destination surface.
	dest += H_Pix_Begin;
	const pixel *dest_end = dest + H_Pix;
	for (; dest < dest_end; dest += 8, src += 8) {
		*(dest+0) = md_palette[src->pixel];
		*(dest+1) = md_palette[(src+1)->pixel];
		*(dest+2) = md_palette[(src+2)->pixel];
		*(dest+3) = md_palette[(src+3)->pixel];
		*(dest+4) = md_palette[(src+4)->pixel];
		*(dest+5) = md_palette[(src+5)->pixel];
		*(dest+6) = md_palette[(src+6)->pixel];
		*(dest+7) = md_palette[(src+7)->pixel];
	}

	if (H_Pix_Begin == 0)
		return;

	// Draw the borders.
	// NOTE: S/H is ignored if we're in the border region.

	// Get the border color.
	register const pixel border_color =
		(q->options.borderColorEmulation ? md_palette[0] : 0);

	// Left border.
	dest -= H_Pix_Begin;
	dest -= H_Pix;
	dest_end = dest + H_Pix_Begin;
	for (; dest < dest_end; dest += 8) {
		*(dest+0) = border_color;
		*(dest+1) = border_color;
		*(dest+2) = border_color;
		*(dest+3) = border_color;
		*(dest+4) = border_color;
		*(dest+5) = border_color;
		*(dest+6) = border_color;
		*(dest+7) = border_color;
	}

	// Right border.
	dest += H_Pix;
	dest_end = dest + H_Pix_Begin;
	for (; dest < dest_end; dest += 8) {
		*(dest+0) = border_color;
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
 * FIXME: Should not be post-processing...
 * FIXME: LCB probably doesn't affect sprites. (Check genplus-gx)
 * @param pixel Type of pixel.
 * @param dest Destination surface.
 * @param border_color Border color.
 */
template<typename pixel>
FORCE_INLINE void VdpPrivate::T_Apply_SMS_LCB(pixel *dest, pixel border_color)
{
	dest += H_Pix_Begin;

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
void VdpPrivate::renderLine_m5(void)
{
	// Determine what part of the screen we're in.
	bool in_border = false;
	int lineNum = q->VDP_Lines.currentLine;

	// TODO: This check needs to be optimized.
	if (lineNum == (q->VDP_Lines.totalDisplayLines - 1) &&
	    (VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_DISP))
	{
		// Clear the sprite dot overflow variable.
		// (TODO: Is this correct?)
		sprDotOverflow = false;

		// Line -1, and display is on.
		// Update the sprite line cache.
		Update_Sprite_Line_Cache_m5(-1);
	}

	// Check for borders.
	if (lineNum >= q->VDP_Lines.Border.borderStartBottom &&
	    lineNum <= q->VDP_Lines.Border.borderEndBottom)
	{
		// Bottom border.
		in_border = true;
	}
	else if (lineNum >= q->VDP_Lines.Border.borderStartTop &&
	         lineNum <= q->VDP_Lines.Border.borderEndTop)
	{
		// Top border.
		in_border = true;
		lineNum -= q->VDP_Lines.Border.borderStartTop;
		lineNum -= q->VDP_Lines.Border.borderSize;
	}

	if (!in_border && q->VDP_Lines.currentLine >= q->VDP_Lines.totalVisibleLines) {
		// Off screen.
		return;
	}

	// Determine the starting line in MD_Screen.
	if (Reg_Status.isNtsc() &&
	    (VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_M2) &&
	    q->options.ntscV30Rolling)
	{
		// NTSC V30 mode. Simulate screen rolling.
		lineNum -= q->VDP_Lines.NTSC_V30.Offset;

		// Prevent underflow.
		if (lineNum < 0)
			lineNum += 240;
	}
	lineNum += q->VDP_Lines.Border.borderSize;

	if (in_border && !q->options.borderColorEmulation) {
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (palette.bpp() != MdFb::BPP_32) {
			memset(q->MD_Screen->lineBuf16(lineNum), 0x00,
				(q->MD_Screen->pxPerLine() * sizeof(uint16_t)));
		} else {
			memset(q->MD_Screen->lineBuf32(lineNum), 0x00,
				(q->MD_Screen->pxPerLine() * sizeof(uint32_t)));
		}

		// ...and we're done here.
		return;
	}

	// Check if the VDP is enabled.
	if (!(VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_DISP) || in_border) {
		// VDP is disabled, or this is the border region.
		// Clear the line buffer.

		// NOTE: S/H is ignored if the VDP is disabled or if
		// we're in the border region.
		memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));

		// Clear the sprite dot overflow variable.
		sprDotOverflow = false;
	} else {
		// VDP is enabled.

		// Determine how to render the image.
		int RenderMode = ((VDP_Reg.m5.Set4 & VDP_REG_M5_SET4_STE) >> 2);	// Shadow/Highlight
		RenderMode |= !!im2_flag;						// Interlaced.
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

		// Update the sprite line cache for the next line.
		if (q->VDP_Lines.currentLine < (q->VDP_Lines.totalDisplayLines - 1)) {
			// Update only for visible lines.
			if (im2_flag) {
				Update_Sprite_Line_Cache_m5(T_GetLineNumber<true>());
			} else {
				Update_Sprite_Line_Cache_m5(T_GetLineNumber<false>());
			}
		}
	}

	// Update the active palette.
	// FIXME: If palette is locked and bpp is changed, convert it.
	if (!(VDP_Layers & VdpTypes::VDP_LAYER_PALETTE_LOCK)) {
		if (!q->options.updatePaletteInVBlankOnly || in_border) {
			if (palette.bpp() != q->MD_Screen->bpp())
				palette.setBpp(q->MD_Screen->bpp());
			else
				palette.update();
		}
	}

	// Render the image.
	// TODO: Optimize SMS LCB handling. (maybe use Linux's unlikely() macro?)
	if (q->MD_Screen->bpp() != MdFb::BPP_32) {
		uint16_t *lineBuf16 = q->MD_Screen->lineBuf16(lineNum);
		T_Render_LineBuf<uint16_t>(lineBuf16, palette.m_palActive.u16);

		if (VDP_Reg.m5.Set1 & VDP_REG_M5_SET1_LCB) {
			// SMS left-column blanking bit is set.
			// FIXME: Should borderColorEmulation apply here?
			T_Apply_SMS_LCB<uint16_t>(lineBuf16, 
				(q->options.borderColorEmulation ? palette.m_palActive.u16[0] : 0));
		}
	} else {
		uint32_t *lineBuf32 = q->MD_Screen->lineBuf32(lineNum);
		T_Render_LineBuf<uint32_t>(lineBuf32, palette.m_palActive.u32);

		if (VDP_Reg.m5.Set1 & VDP_REG_M5_SET1_LCB) {
			// SMS left-column blanking bit is set.
			// FIXME: Should borderColorEmulation apply here?
			T_Apply_SMS_LCB<uint32_t>(lineBuf32, 
				(q->options.borderColorEmulation ? palette.m_palActive.u32[0] : 0));
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
	dest += H_Pix_Begin;

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

	switch (_32X_Rend_Mode) {
		case 0:
		case 4:
		case 8:
		case 12:
			//POST_LINE_32X_M00;
			for (unsigned int px = H_Pix; px != 0; px -= 4, dest += 4, lbptr += 4) {
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
			for (unsigned int px = H_Pix; px != 0; px -= 2, src += 2, dest += 2, lbptr += 2) {
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
		case 10: {
			//POST_LINE_32X_M01;
			const uint16_t *src = &_32X_VDP_Ram.u16[VRam_Ind];
			for (unsigned int px = H_Pix; px != 0; px -= 2, src += 2, dest += 2, lbptr += 2) {
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
		case 15: {
			// POST_LINE_32X_M11
			// This appears to be a form of RLE compression.
			int px = 0;
			int px_end;
			const uint8_t *src = &_32X_VDP_Ram.u8[VRam_Ind << 1];
			while (px < H_Pix) {
#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
				px1 = _32X_vdp_cram_adjusted[*src];
				px_end = px + *(src+1);
#else //GSFT_BYTEORDER == GSFT_BIG_ENDIAN
				px1 = _32X_vdp_cram_adjusted[*(src+1)];
				px_end = px + *src;
#endif
				src += 2;

				// Make sure it doesn't go out of bounds.
				if (px_end >= H_Pix)
					px_end = (H_Pix - 1);
				
				for (; px <= px_end; px++) {
					dest[px] = px1;
				}
			}
			break;
		}

		case 5: {
			//POST_LINE_32X_M01_P;
			// TODO: Endianness conversions.
			const uint8_t *src = &_32X_VDP_Ram.u8[VRam_Ind << 1];
			for (unsigned int px = H_Pix; px != 0; px -= 2, src += 2, dest += 2, lbptr += 2) {
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
			for (unsigned int px = H_Pix; px != 0; px--, dest++, lbptr++) {
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
			for (unsigned int px = H_Pix; px != 0; px--, dest++, lbptr++) {
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
 * Render a line. (Mode 5, 32X)
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
	if ((CPU_Mode == 0) && (VDP_Reg.m5.Set2 & 0x08) && Video.ntscV30rolling) {
		// NTSC V30 mode. Simulate screen rolling.
		LineStart -= VDP_Lines.NTSC_V30.Offset;

		// Prevent underflow.
		if (LineStart < 0)
			LineStart += 240;
	}
	LineStart = TAB336[LineStart + VDP_Lines.Visible.Border_Size] + 8;

	if (in_border && !Video.borderColorEmulation) {
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (bppMD == 32)
			memset(&MD_Screen.u32[LineStart], 0x00, (H_Pix * sizeof(uint32_t)));
		else
			memset(&MD_Screen.u16[LineStart], 0x00, (H_Pix * sizeof(uint16_t)));

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
	} else {
		// VDP is enabled.

		// Check if sprite structures need to be updated.
		if (VDP_Reg.Interlaced.DoubleRes) {
			// Interlaced.
			if (VDP_Flags.VRam)
				T_Make_Sprite_Struct<true, false>();
			else if (VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<true, true>();
		} else {
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
		const int RenderMode = ((VDP_Reg.m5.Set4 & VDP_REG_M5_SET4_STE) >> 2) | VDP_Reg.Interlaced.DoubleRes;
		switch (RenderMode & 3) {
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
	if (VDP_Flags.CRam) {
		// Update the palette.
		if (VDP_Reg.m5.Set4 & VDP_REG_M5_SET4_STE)
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
	if (!in_border) {
		if (bppMD != 32) {
			T_Render_LineBuf_32X<uint16_t>
						(&MD_Screen.u16[LineStart], MD_Palette.u16,
						_32X_Rend_Mode, _32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
		} else {
			T_Render_LineBuf_32X<uint32_t>
						(&MD_Screen.u32[LineStart], MD_Palette.u32,
						_32X_Rend_Mode, _32X_Palette.u32, _32X_VDP_CRam_Adjusted.u32);
		}
	} else {
		// In border. Use standard MD rendering.
		if (bppMD != 32)
			T_Render_LineBuf<uint16_t>(&MD_Screen.u16[LineStart], MD_Palette.u16);
		else
			T_Render_LineBuf<uint32_t>(&MD_Screen.u32[LineStart], MD_Palette.u32);
	}
}
#endif

}
