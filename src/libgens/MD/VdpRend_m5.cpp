/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_m5.cpp: VDP rendering class. (Mode 5)                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "VdpRend_m5.hpp"

/** Static member initialization. **/
#include "VdpRend_m5_static.hpp"

#include "VdpRend.hpp"
#include "VdpIo.hpp"
#include "TAB336.h"

// C includes.
#include <string.h>

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
 * VdpRend_m5::T_GetLineNumber(): Get the current line number, adjusted for interlaced display.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @return Line number.
 */
template<bool interlaced>
FORCE_INLINE int VdpRend_m5::T_GetLineNumber(void)
{
	// Get the current line number.
	int vdp_line = VdpIo::VdpIo::VDP_Lines.Visible.Current;
	
	if (interlaced)
	{
		// Adjust the VDP line number for Flickering Interlaced display.
		vdp_line *= 2;
		
		switch (IntRend_Mode)
		{
			case INTREND_EVEN:
			default:
				// Even lines only.
				// Don't do anything.
				break;
			
			case INTREND_ODD:
				// Odd lines only.
				vdp_line++;
				break;
			
			case INTREND_FLICKER:
				// Flickering Interlaced mode.
				if (VdpIo::VdpIo::VDP_Status & 0x0010)
					vdp_line++;
				break;
		}
	}
	
	return vdp_line;
}

/**
 * VdpRend_m5::T_PutPixel_P0(): Put a pixel in background graphics layer 0.
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
FORCE_INLINE void VdpRend_m5::T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	if (!(pattern & mask))
		return;
	
	// Check the layer bits of the current pixel.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	uint8_t layer_bits = VdpRend::LineBuf.px[LineBuf_pixnum].layer;
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
	
	// If Highlight/Shadow is enabled, mark this pixel as shadow.
	if (h_s)
		pat8 |= LINEBUF_SHAD_B;
	
	// Write the new pixel to the line buffer.
	VdpRend::LineBuf.px[LineBuf_pixnum].pixel = pat8;
}


/**
 * VdpRend_m5::T_PutPixel_P1(): Put a pixel in background graphics layer 1.
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
FORCE_INLINE void VdpRend_m5::T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return;
	
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum);
	
	if (plane)
	{
		// Scroll A: If the pixel is a Window pixel, don't do anything.
		if (VdpRend::LineBuf.px[LineBuf_pixnum].layer & LINEBUF_WIN_B)
			return;
	}
	
	// Shift the pixel.
	px >>= shift;
	
	// Update the pixel:
	// - Add palette information.
	// - Mark the pixel as priority.
	// - Save it to the linebuffer.
	px |= palette | LINEBUF_PRIO_W;
	VdpRend::LineBuf.u16[LineBuf_pixnum] = (uint16_t)px;
}


/**
 * VdpRend_m5::T_PutPixel_Sprite(): Put a pixel in the sprite layer.
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
FORCE_INLINE uint8_t VdpRend_m5::T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette)
{
	// TODO: Convert mask and shift to template parameters.
	
	// Check if this is a transparent pixel.
	unsigned int px = (pattern & mask);
	if (px == 0)
		return 0;
	
	// Get the pixel number in the linebuffer.
	const unsigned int LineBuf_pixnum = (disp_pixnum + pat_pixnum + 8);
	
	// TODO: Endianness conversions.
	uint8_t layer_bits = VdpRend::LineBuf.px[LineBuf_pixnum].layer;
	
	if (layer_bits & (LINEBUF_PRIO_B + LINEBUF_SPR_B - priority))
	{
		// Priority bit is set. (TODO: Is that what this means?)
		if (!priority)
		{
			// Set the sprite bit in the linebuffer.
			VdpRend::LineBuf.px[LineBuf_pixnum].layer |= LINEBUF_SPR_B;
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
			VdpRend::LineBuf.u16[LineBuf_pixnum] |= LINEBUF_HIGH_W;
			return 0;
		}
		else if (px == 0x3F)
		{
			// Palette 3, color 15: Shadow. (Sprite pixel doesn't show up.)
			VdpRend::LineBuf.u16[LineBuf_pixnum] |= LINEBUF_SHAD_W;
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
	VdpRend::LineBuf.u16[LineBuf_pixnum] = px;
	
	return 0;
}


#define LINEBUF_HIGH_D	0x80808080
#define LINEBUF_SHAD_D	0x40404040
#define LINEBUF_PRIO_D	0x01000100
#define LINEBUF_SPR_D	0x20002000
#define LINEBUF_WIN_D	0x02000200


/**
 * VdpRend_m5::T_PutLine_P0(): Put a line in background graphics layer 0.
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
FORCE_INLINE void VdpRend_m5::T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane)
	{
		// Scroll B.
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SCROLLB_LOW))
			return;
	}
	else
	{
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SCROLLA_LOW))
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
 * VdpRend_m5::T_PutLine_P1(): Put a line in background graphics layer 1.
 * @param plane		[in] True for Scroll A; false for Scroll B.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool plane, bool h_s, bool flip>
FORCE_INLINE void VdpRend_m5::T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette)
{
	if (!plane)
	{
		// Scroll B.
		// Clear the line.
		memset(&VdpRend::LineBuf.u16[disp_pixnum], 0x00, 8*2);
		
		// If ScrollB_Low is disabled, don't do anything.
		if (!(VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SCROLLB_LOW))
			return;
	}
	else
	{
		// Scroll A.
		// If ScrollA Low is disabled. don't do anything.
		if (!(VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SCROLLA_LOW))
			return;
		
		// AND the linebuffer with ~LINEBUF_SHAD_W.
		// TODO: Optimize this to use 32-bit operations instead of 16-bit.
		VdpRend::LineBuf.u16[disp_pixnum]   &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+1] &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+2] &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+3] &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+4] &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+5] &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+6] &= ~LINEBUF_SHAD_W;
		VdpRend::LineBuf.u16[disp_pixnum+7] &= ~LINEBUF_SHAD_W;
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
 * VdpRend_m5::T_PutLine_Sprite(): Put a line in the sprite layer.
 * @param priority	[in] Sprite priority. (false == low, true == high)
 * @param h_s		[in] Highlight/Shadow enable.
 * @param flip		[in] True to flip the line horizontally.
 * @param disp_pixnum	[in] Display pixel nmber.
 * @param pattern	[in] Pattern data.
 * @param palette	[in] Palette number * 16.
 */
template<bool priority, bool h_s, bool flip>
FORCE_INLINE void VdpRend_m5::T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette)
{
	// Check if the sprite layer is disabled.
	if (!(VdpRend::VDP_Layers & (priority ? VdpRend::VDP_LAYER_SPRITE_HIGH : VdpRend::VDP_LAYER_SPRITE_LOW)))
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
	VdpIo::VDP_Status |= (status & 0x20);
}


/**
 * VdpRend_m5::T_Get_X_Offset(): Get the X offset for the line. (Horizontal Scroll Table)
 * @param plane True for Scroll A; false for Scroll B.
 * @return X offset.
 */
template<bool plane>
FORCE_INLINE uint16_t VdpRend_m5::T_Get_X_Offset(void)
{
	const unsigned int H_Scroll_Offset = (VdpIo::VDP_Lines.Visible.Current & VdpIo::H_Scroll_Mask) * 2;
	
	if (plane)
	{
		// Scroll A.
		return VdpIo::H_Scroll_Addr[H_Scroll_Offset];
	}
	else
	{
		// Scroll B.
		return VdpIo::H_Scroll_Addr[H_Scroll_Offset + 1];
	}
}


/**
 * VdpRend_m5::T_Update_Y_Offset(): Update the Y offset.
 * @param plane True for Scroll A; false for Scroll B.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param cell_cur Current X cell number.
 * @return Y offset.
 */
template<bool plane, bool interlaced>
FORCE_INLINE unsigned int VdpRend_m5::T_Update_Y_Offset(int cell_cur)
{
	if ((cell_cur & 0xFF80) || (cell_cur < 0))
	{
		// Cell number is invalid.
		return 0;
	}
	
	// Mask off odd columns.
	cell_cur &= ~1;
	
	// Get the vertical scroll offset.
	unsigned int VScroll_Offset;
	if (plane)
	{
		// Scroll A.
		VScroll_Offset = VdpIo::VSRam.u16[cell_cur];
	}
	else
	{
		// Scroll B.
		VScroll_Offset = VdpIo::VSRam.u16[cell_cur + 1];
	}
	
	// Add the current line number to the VScroll offset.
	VScroll_Offset += T_GetLineNumber<interlaced>();
	
	if (interlaced)
	{
		// Interlaced mode.
		Y_FineOffset = (VScroll_Offset & 15);
		
		// Get the V Cell offset and prevent it from overflowing.
		VScroll_Offset = (VScroll_Offset >> 4) & VdpIo::V_Scroll_CMask;
	}
	else
	{
		// Non-Interlaced mode.
		Y_FineOffset = (VScroll_Offset & 7);
		
		// Get the V Cell offset and prevent it from overflowing.
		VScroll_Offset = (VScroll_Offset >> 3) & VdpIo::V_Scroll_CMask;
	}
	
	return VScroll_Offset;
}


/**
 * VdpRend_m5::T_Get_Pattern_Info(): Get pattern info from a scroll plane.
 * H_Scroll_CMul must be initialized correctly.
 * @param plane True for Scroll A; false for Scroll B.
 * @param x X tile number.
 * @param y Y tile number.
 * @return Pattern info.
 */
template<bool plane>
FORCE_INLINE uint16_t VdpRend_m5::T_Get_Pattern_Info(unsigned int x, unsigned int y)
{
	// Get the offset.
	// H_Scroll_CMul is the shift value required for the proper vertical offset.
	unsigned int offset = (y << VdpIo::H_Scroll_CMul) + x;
	
	// Return the pattern information.
	return (plane ? VdpIo::ScrA_Addr[offset] : VdpIo::ScrB_Addr[offset]);
}


/**
 * VdpRend_m5::T_Get_Pattern_Data(): Get pattern data for a given tile for the current line.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param pattern Pattern info.
 * @return Pattern data.
 */
template<bool interlaced>
FORCE_INLINE unsigned int VdpRend_m5::T_Get_Pattern_Data(uint16_t pattern)
{
	// Vertical offset.
	unsigned int V_Offset = Y_FineOffset;
	
	// Get the tile address.
	unsigned int TileAddr;
	if (interlaced)
		TileAddr = (pattern & 0x3FF) << 6;
	else
		TileAddr = (pattern & 0x7FF) << 5;
	
	if (pattern & 0x1000)
	{
		// V Flip enabled. Flip the tile vertically.
		if (interlaced)
			V_Offset ^= 15;
		else
			V_Offset ^= 7;
	}
	
	// Return the pattern data.
	return VdpIo::VRam.u32[(TileAddr + (V_Offset * 4)) >> 2];
}


/**
 * VdpRend_m5::T_Render_Line_Scroll(): Render a scroll line.
 * @param plane		[in] True for Scroll A / Window; false for Scroll B.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 * @param cell_start	[in] (Scroll A) First cell to draw.
 * @param cell_length	[in] (Scroll A) Number of cells to draw.
 */
template<bool plane, bool interlaced, bool vscroll, bool h_s>
FORCE_INLINE void VdpRend_m5::T_Render_Line_Scroll(int cell_start, int cell_length)
{
	// Get the horizontal scroll offset. (cell and fine offset)
	unsigned int X_offset_cell = T_Get_X_Offset<plane>() & 0x3FF;
	
	// Drawing will start at the fine cell offset.
	// VdpRend::LineBuf.u16[X_offset_cell & 7]
	unsigned int disp_pixnum = (X_offset_cell & 7);
	
	// Determine if we should apply the Left Window bug.
	int LeftWindowBugCnt = 0;	// Left Window bug counter.
	if (plane && (cell_start != 0))
	{
		// Determine the value for the Left Window bug counter.
		// First tile: Counter should be 2.
		// Second tile: Counter should be 1.
		LeftWindowBugCnt = ((X_offset_cell & 8) ? 2 : 1);
	}
	
	if (plane)
	{
		// Adjust for the cell starting position.
		const int cell_start_px = (cell_start << 3);
		X_offset_cell -= cell_start_px;
		disp_pixnum += cell_start_px;
	}
	
	// Get the correct cell offset:
	// - Invert the cell position.
	// - Right-shift by 3 for the cell number.
	// - AND with the horizontal scrolling cell mask to prevent overflow.
	X_offset_cell = (((X_offset_cell ^ 0x3FF) >> 3) & VdpIo::H_Scroll_CMask);
	
	// VSRam cell number.
	int VSRam_Cell = ((X_offset_cell & 1) - 2);
	
	// Initialize the Y offset.
	unsigned int Y_offset_cell;				// Y offset. (in cells)
	if (!vscroll)
	{
		// Full vertical scrolling.
		// Initialize the Y offset here.
		Y_offset_cell = T_Update_Y_Offset<plane, interlaced>(VSRam_Cell + 2);
	}
	
	// Loop through the cells.
	for (int x = (plane ? cell_length : VdpIo::H_Cell);
	     x >= 0; x--, VSRam_Cell++)
	{
		if (vscroll)
		{
			// 2-cell vertical scrolling.
			// Update the Y offset.
			Y_offset_cell = T_Update_Y_Offset<plane, interlaced>(VSRam_Cell);
		}
		
		// Get the pattern info for the current tile.
		uint16_t pattern_info;
		if (!plane)
		{
			// Scroll B.
			pattern_info = T_Get_Pattern_Info<plane>(X_offset_cell, Y_offset_cell);
		}
		else
		{
			// Scroll A. Check if we need to emulate the Left Window bug.
			if (LeftWindowBugCnt <= 0)
			{
				// Left Window bug doesn't apply or has already been emulated.
				pattern_info = T_Get_Pattern_Info<plane>(X_offset_cell, Y_offset_cell);
			}
			else
			{
				// Left Window bug applies.
				LeftWindowBugCnt--;
				const unsigned int TmpXCell = ((X_offset_cell + 2) & VdpIo::H_Scroll_CMask);
				pattern_info = T_Get_Pattern_Info<plane>(TmpXCell, Y_offset_cell);
			}
		}
		
		// Get the pattern data for the current tile.
		uint32_t pattern_data = T_Get_Pattern_Data<interlaced>(pattern_info);
		
		// Extract the palette number.
		// Resulting number is palette * 16.
		unsigned int palette = (pattern_info >> 9) & 0x30;
		
		// Check for swapped Scroll B priority.
		if (VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SCROLLB_SWAP)
			pattern_info ^= 0x8000;
		
		// Check for horizontal flip.
		if (pattern_info & 0x0800)
		{
			// Pattern has H-Flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<plane, h_s, true>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, true>(disp_pixnum, pattern_data, palette);
		}
		else
		{
			// Pattern doesn't have flip enabled.
			if (pattern_info & 0x8000)
				T_PutLine_P1<plane, h_s, false>(disp_pixnum, pattern_data, palette);
			else
				T_PutLine_P0<plane, h_s, false>(disp_pixnum, pattern_data, palette);
		}
		
		// Go to the next H cell.
		X_offset_cell = (X_offset_cell + 1) & VdpIo::H_Scroll_CMask;
		
		// Go to the next pattern.
		disp_pixnum += 8;
	}
}


/**
 * VdpRend_m5::T_Render_Line_ScrollA(): Render a line for Scroll A / Window.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param vscroll	[in] True for 2-cell mode; false for full scroll.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool vscroll, bool h_s>
FORCE_INLINE void VdpRend_m5::T_Render_Line_ScrollA(void)
{
	// Cell counts for Scroll A.
	int ScrA_Start, ScrA_Length;
	int Win_Start, Win_Length = 0;
	
	// Check if the entire line is part of the window.
	// TODO: Verify interlaced operation!
	const int vdp_cells = (VdpIo::VDP_Lines.Visible.Current >> 3);
	if (VdpIo::VDP_Reg.m5.Win_V_Pos & 0x80)
	{
		// Window starts from the bottom.
		if (vdp_cells >= VdpIo::Win_Y_Pos)
		{
			// Current line is >= starting line.
			// Entire line is part of the window.
			ScrA_Start = 0;
			ScrA_Length = 0;
			Win_Start = 0;
			Win_Length = VdpIo::H_Cell;
		}
	}
	else if (vdp_cells < VdpIo::Win_Y_Pos)
	{
		// Current line is < ending line.
		// Entire line is part of the window.
		ScrA_Start = 0;
		ScrA_Length = 0;
		Win_Start = 0;
		Win_Length = VdpIo::H_Cell;
	}
	
	if (Win_Length == 0)
	{
		// Determine the cell starting position and length.
		if (VdpIo::VDP_Reg.m5.Win_H_Pos & 0x80)
		{
			// Window is right-aligned.
			ScrA_Start = 0;
			ScrA_Length = VdpIo::Win_X_Pos;
			Win_Start = VdpIo::Win_X_Pos;
			Win_Length = (VdpIo::H_Cell - VdpIo::Win_X_Pos);
		}
		else
		{
			// Window is left-aligned.
			Win_Start = 0;
			Win_Length = VdpIo::Win_X_Pos;
			ScrA_Start = VdpIo::Win_X_Pos;
			ScrA_Length = (VdpIo::H_Cell - VdpIo::Win_X_Pos);
		}
	}
	
	if (Win_Length > 0)
	{
		// Draw the window.
		
		// Drawing will start at the first window cell.
		// (Window is not scrollable.)
		unsigned int disp_pixnum = (Win_Start * 8) + 8;
		
		// Get the cell offsets.
		// TODO: Is this affected by interlaced mode?
		unsigned int X_offset_cell = Win_Start;
		
		// Calculate the fine offsets.
		const int vdp_line = T_GetLineNumber<interlaced>();
		if (interlaced)
			Y_FineOffset = (vdp_line & 15);
		else
			Y_FineOffset = (vdp_line & 7);
		
		// Window row start address.
		const unsigned int Y_offset_cell = (VdpIo::VDP_Lines.Visible.Current / 8);
		const uint16_t *Win_Row_Addr = &VdpIo::Win_Addr[Y_offset_cell << VdpIo::H_Win_Shift];
		
		// Loop through the cells.
		for (int x = Win_Length; x > 0; x--)
		{
			// Get the pattern info and data for the current tile.
			register uint16_t pattern_info = *Win_Row_Addr++;
			uint32_t pattern_data = T_Get_Pattern_Data<interlaced>(pattern_info);
			
			// Extract the palette number.
			// Resulting number is palette * 16.
			unsigned int palette = (pattern_info >> 9) & 0x30;
			
			// Check for swapped Scroll A priority.
			if (VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SCROLLA_SWAP)
				pattern_info ^= 0x8000;
			
			// Check for horizontal flip.
			if (pattern_info & 0x0800)
			{
				// Pattern has H-Flip enabled.
				if (pattern_info & 0x8000)
					T_PutLine_P1<true, h_s, true>(disp_pixnum, pattern_data, palette);
				else
					T_PutLine_P0<true, h_s, true>(disp_pixnum, pattern_data, palette);
			}
			else
			{
				// Pattern doesn't have flip enabled.
				if (pattern_info & 0x8000)
					T_PutLine_P1<true, h_s, false>(disp_pixnum, pattern_data, palette);
				else
					T_PutLine_P0<true, h_s, false>(disp_pixnum, pattern_data, palette);
			}
			
			// Go to the next H cell.
			X_offset_cell++;
			
			// Go to the next pattern.
			disp_pixnum += 8;
		}
		
		// Mark window pixels.
		// TODO: Do this in the Window drawing code!
		if (ScrA_Length > 0)
		{
			const int StartPx = ((Win_Start * 8) + 8) / 2;
			const int EndPx = StartPx + ((Win_Length * 8) / 2);
			
			for (int x = StartPx; x < EndPx; x++)
				VdpRend::LineBuf.u32[x] |= LINEBUF_WIN_D;
		}
	}
	
	if (ScrA_Length > 0)
	{
		// Draw the scroll area.
		T_Render_Line_Scroll<true, interlaced, vscroll, h_s>(ScrA_Start, ScrA_Length);
	}
}


/**
 * VdpRend_m5::T_Make_Sprite_Struct(): Fill Sprite_Struct[] with information from the Sprite Attribute Table.
 * @param interlaced If true, using Interlaced Mode 2. (2x res)
 * @param partial If true, only do a partial update. (X pos, X size)
 */
template<bool interlaced, bool partial>
FORCE_INLINE void VdpRend_m5::T_Make_Sprite_Struct(void)
{
	uint16_t *CurSpr = VdpIo::Spr_Addr;
	unsigned int spr_num = 0;
	unsigned int link;
	
	// H40 allows 80 sprites; H32 allows 64 sprites.
	// Essentially, it's (H_Cell * 2).
	// [Nemesis' Sprite Masking and Overflow Test ROM: Test #9]
	const unsigned int max_spr = (VdpIo::H_Cell * 2);
	
	do
	{
		// Sprite position.
		VdpRend::Sprite_Struct[spr_num].Pos_X = (CurSpr[3] & 0x1FF) - 128;
		if (!partial)
		{
			if (interlaced)
			{
				// Interlaced mode. Y position is 11-bit.
				VdpRend::Sprite_Struct[spr_num].Pos_Y = (CurSpr[0] & 0x3FF) - 256;
			}
			else
			{
				// Non-Interlaced mode. Y position is 10-bit.
				VdpRend::Sprite_Struct[spr_num].Pos_Y = (CurSpr[0] & 0x1FF) - 128;
			}
		}
		
		// Sprite size.
		uint8_t sz = (CurSpr[1] >> 8);
		VdpRend::Sprite_Struct[spr_num].Size_X = ((sz >> 2) & 3) + 1;	// 1 more than the original value.
		if (!partial)
			VdpRend::Sprite_Struct[spr_num].Size_Y = sz & 3;		// Exactly the original value.
		
		// Determine the maximum positions.
		VdpRend::Sprite_Struct[spr_num].Pos_X_Max =
				VdpRend::Sprite_Struct[spr_num].Pos_X +
				((VdpRend::Sprite_Struct[spr_num].Size_X * 8) - 1);
		
		if (!partial)
		{
			if (interlaced)
			{
				// Interlaced mode. Cells are 8x16.
				VdpRend::Sprite_Struct[spr_num].Pos_Y_Max =
						VdpRend::Sprite_Struct[spr_num].Pos_Y +
						((VdpRend::Sprite_Struct[spr_num].Size_Y * 16) + 15);
			}
			else
			{
				// Non-Interlaced mode. Cells are 8x8.
				VdpRend::Sprite_Struct[spr_num].Pos_Y_Max =
						VdpRend::Sprite_Struct[spr_num].Pos_Y +
						((VdpRend::Sprite_Struct[spr_num].Size_Y * 8) + 7);
			}
			
			// Tile number. (Also includes palette, priority, and flip bits.)
			VdpRend::Sprite_Struct[spr_num].Num_Tile = CurSpr[2];
		}
		
		// Link number.
		link = (CurSpr[1] & 0xFF);
		
		// Increment the sprite number.
		spr_num++;
		if (link == 0)
			break;
		
		// Go to the next sprite.
		CurSpr = VdpIo::Spr_Addr + (link * (8>>1));
		
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
 * VdpRend_m5::T_Update_Mask_Sprite(): Update Sprite_Visible[] using sprite masking.
 * @param sprite_limit If true, emulates sprite limits.
 * @param interlaced If true, uses interlaced mode.
 * @return Number of visible sprites.
 */
template<bool sprite_limit, bool interlaced>
FORCE_INLINE unsigned int VdpRend_m5::T_Update_Mask_Sprite(void)
{
	// If Sprite Limit is on, the following limits are enforced: (H32/H40)
	// - Maximum sprite dots per line: 256/320
	// - Maximum sprites per line: 16/20
	int max_cells = VdpIo::H_Cell;
	int max_sprites = (VdpIo::H_Cell / 2);
	
	bool overflow = false;
	
	// sprite_on_line is set if at least one sprite is on the scanline
	// that is not a sprite mask (x == 0). Sprite masks are only effective
	// if there is at least one higher-priority sprite on the scanline.
	// Thus, if a sprite mask is the first sprite on the scanline it is ignored.
	// However, if the previous line had a sprite dot overflow, it is *not*
	// ignored, so it is processed as a regular mask.
	bool sprite_on_line = (bool)VdpIo::SpriteDotOverflow;

	// sprite_mask_active is set if a sprite mask is preventing
	// remaining sprites from showing up on the scanline.
	// Those sprites still count towards total sprite and sprite dot counts.
	bool sprite_mask_active = false;

	unsigned int spr_num = 0;	// Current sprite number in Sprite_Struct[].
	unsigned int spr_vis = 0;	// Current visible sprite in Sprite_Visible[].
	
	// Get the current line number.
	const int vdp_line = T_GetLineNumber<interlaced>();
	
	// Search for all sprites visible on the current scanline.
	for (; spr_num < TotalSprites; spr_num++)
	{
		if (VdpRend::Sprite_Struct[spr_num].Pos_Y > vdp_line ||
		    VdpRend::Sprite_Struct[spr_num].Pos_Y_Max < vdp_line)
		{
			// Sprite is not on the current line.
			continue;
		}
		
		if (sprite_limit)
		{
			// Sprite limit is enabled.
			// Decrement the maximum cell and sprite counters.
			max_cells -= VdpRend::Sprite_Struct[spr_num].Size_X;
			max_sprites--;
		}
		
		// Check for sprite masking.
		if (VdpRend::Sprite_Struct[spr_num].Pos_X == -128)
		{
			// Sprite mask.
			if (sprite_on_line)
			{
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
		}
		else
		{
			// Regular sprite.
			sprite_on_line = true;
			
			// Check if the sprite is onscreen.
			if (!sprite_mask_active &&
				VdpRend::Sprite_Struct[spr_num].Pos_X < VdpIo::H_Pix &&
				VdpRend::Sprite_Struct[spr_num].Pos_X_Max >= 0)
			{
				// Sprite is onscreen.
				VdpRend::Sprite_Visible[spr_vis] = spr_num;
				spr_vis++;
			}
			
			// Set the visible X max.
			VdpRend::Sprite_Struct[spr_num].Pos_X_Max_Vis = VdpRend::Sprite_Struct[spr_num].Pos_X_Max;
		}
		
		if (sprite_limit)
		{
			// Check for cell or sprite overflow.
			if (max_cells <= 0)
			{
				// Cell overflow!
				// Remove the extra cells from the sprite.
				// [Nemesis' Sprite Masking and Overflow Test ROM: Tests #2 and #3]
				// #2 == total sprite dot count; #3 == per-cell dot count.
				// TODO: Verify how Pos_X_Max_Vis should work with regards to H Flip.
				overflow = true;
				
				// Decrement the displayed number of cells for the sprite.
				VdpRend::Sprite_Struct[spr_num].Pos_X_Max_Vis += (max_cells * 8);
				spr_num++;
				break;
			}
			else if (max_sprites == 0)
			{
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
	VdpIo::SpriteDotOverflow = (max_cells <= 0);
	
	if (sprite_limit && overflow)
	{
		// Sprite overflow. Check if there are any more sprites.
		for (; spr_num < TotalSprites; spr_num++)
		{
			// Check if the sprite is on the current line.
			if (VdpRend::Sprite_Struct[spr_num].Pos_Y > vdp_line ||
			    VdpRend::Sprite_Struct[spr_num].Pos_Y_Max < vdp_line)
			{
				// Sprite is not on the current line.
				continue;
			}
			
			// Sprite is on the current line.
			if (--max_sprites < 0)
			{
				// Sprite overflow!
				// Set the SOVR flag.
				VdpIo::VDP_Status |= 0x40;
				break;
			}
		}
	}
	
	// Return the number of visible sprites.
	return spr_vis;
}


/**
 * VdpRend_m5::T_Render_Line_Sprite(): Render a sprite line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
FORCE_INLINE void VdpRend_m5::T_Render_Line_Sprite(void)
{
	// Update the sprite masks.
	unsigned int num_spr;
	if (VdpRend::Sprite_Limits)
		num_spr = T_Update_Mask_Sprite<true, interlaced>();
	else
		num_spr = T_Update_Mask_Sprite<false, interlaced>();
	
	for (unsigned int spr_vis = 0; spr_vis < num_spr; spr_vis++)
	{
		// Get the sprite number. (TODO: Eliminate the sizeof division.)
		unsigned int spr_num = VdpRend::Sprite_Visible[spr_vis];
		
		// Determine the cell and line offsets.
		unsigned int cell_offset = (T_GetLineNumber<interlaced>() - VdpRend::Sprite_Struct[spr_num].Pos_Y);
		unsigned int line_offset;
		
		if (interlaced)
		{
			// Interlaced.
			line_offset = (cell_offset & 15);
			cell_offset &= 0x1F0;
		}
		else
		{
			// Non-Interlaced.
			line_offset = (cell_offset & 7);
			cell_offset &= 0xF8;
		}
		
		// Get the Y cell size.
		unsigned int Y_cell_size = VdpRend::Sprite_Struct[spr_num].Size_Y;
		
		// Get the sprite information.
		// Also, check for swapped sprite layer priority.
		unsigned int spr_info = VdpRend::Sprite_Struct[spr_num].Num_Tile;
		if (VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SPRITE_SWAP)
			spr_info ^= 0x8000;
		
		// Get the palette number, multiplied by 16.
		const unsigned int palette = ((spr_info >> 9) & 0x30);
		
		// Get the pattern number.
		unsigned int tile_num;
		if (interlaced)
		{
			tile_num = (spr_info & 0x3FF) << 6;	// point on the contents of the pattern
			Y_cell_size <<= 6;	// Size_Y * 64
			cell_offset *= 4;	// Num_Pattern * 64
		}
		else
		{
			tile_num = (spr_info & 0x7FF) << 5;	// point on the contents of the pattern
			Y_cell_size <<= 5;	// Size_Y * 32
			cell_offset *= 4;	// Num_Pattern * 32
		}
		
		// Check for V Flip.
		if (spr_info & 0x1000)
		{
			// V Flip enabled.
			if (interlaced)
				line_offset ^= 15;
			else
				line_offset ^= 7;
			
			tile_num += (Y_cell_size - cell_offset);
			if (interlaced)
			{
				Y_cell_size += 64;
				tile_num += (line_offset * 4);
			}
			else
			{
				Y_cell_size += 32;
				tile_num += (line_offset * 4);
			}
		}
		else
		{
			// V Flip disabled.
			tile_num += cell_offset;
			if (interlaced)
			{
				Y_cell_size += 64;
				tile_num += (line_offset * 4);
			}
			else
			{
				Y_cell_size += 32;
				tile_num += (line_offset * 4);
			}
		}
		
		// Check for H Flip.
		register int H_Pos_Min;
		register int H_Pos_Max;
		
		if (spr_info & 0x800)
		{
			// H Flip enabled.
			// Check the minimum edge of the sprite.
			H_Pos_Min = VdpRend::Sprite_Struct[spr_num].Pos_X;
			if (H_Pos_Min < -7)
				H_Pos_Min = -7;	// minimum edge = clip screen
			
			// TODO: Verify how Pos_X_Max_Vis should work with regards to H Flip.
			H_Pos_Max = VdpRend::Sprite_Struct[spr_num].Pos_X_Max_Vis;
			
			H_Pos_Max -= 7;				// to post the last pattern in first
			while (H_Pos_Max >= VdpIo::H_Pix)
			{
				H_Pos_Max -= 8;			// move back to the preceding pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}
			
			// Draw the sprite.
			if ((VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000))
			{
				// High priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8)
				{
					uint32_t pattern = VdpIo::VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<true, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
			else
			{
				// Low priority.
				for (; H_Pos_Max >= H_Pos_Min; H_Pos_Max -= 8)
				{
					uint32_t pattern = VdpIo::VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<false, h_s, true>(H_Pos_Max, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		}
		else
		{
			// H Flip disabled.
			// Check the minimum edge of the sprite.
			H_Pos_Min = VdpRend::Sprite_Struct[spr_num].Pos_X;
			H_Pos_Max = VdpRend::Sprite_Struct[spr_num].Pos_X_Max_Vis;
			if (H_Pos_Max >= VdpIo::H_Pix)
				H_Pos_Max = VdpIo::H_Pix;
			
			while (H_Pos_Min < -7)
			{
				H_Pos_Min += 8;			// advance to the next pattern (screen)
				tile_num += Y_cell_size;	// go to the next pattern (VRam)
			}
			
			// Draw the sprite.
			if ((VdpRend::VDP_Layers & VdpRend::VDP_LAYER_SPRITE_ALWAYSONTOP) || (spr_info & 0x8000))
			{
				// High priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8)
				{
					uint32_t pattern = VdpIo::VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<true, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
			else
			{
				// Low priority.
				for (; H_Pos_Min < H_Pos_Max; H_Pos_Min += 8)
				{
					uint32_t pattern = VdpIo::VRam.u32[tile_num >> 2];
					T_PutLine_Sprite<false, h_s, false>(H_Pos_Min, pattern, palette);
					tile_num += Y_cell_size;
				}
			}
		}
	}
}


/**
 * T_Render_Line_m5(): Render a line.
 * @param interlaced	[in] True for interlaced; false for non-interlaced.
 * @param h_s		[in] Highlight/Shadow enable.
 */
template<bool interlaced, bool h_s>
FORCE_INLINE void VdpRend_m5::T_Render_Line_m5(void)
{
	// Clear the line first.
	memset(&VdpRend::LineBuf, (h_s ? LINEBUF_SHAD_B : 0), sizeof(VdpRend::LineBuf));
	
	if (VdpIo::VDP_Reg.m5.Set3 & 0x04)
	{
		// 2-cell VScroll.
		T_Render_Line_Scroll<false, interlaced, true, h_s>(0, 0);	// Scroll B
		T_Render_Line_ScrollA<interlaced, true, h_s>();			// Scroll A
	}
	else
	{
		// Full VScroll.
		T_Render_Line_Scroll<false, interlaced, false, h_s>(0, 0);	// Scroll B
		T_Render_Line_ScrollA<interlaced, false, h_s>();		// Scroll A
	}
	
	T_Render_Line_Sprite<interlaced, h_s>();
}


/**
 * VdpRend_m5::T_Render_LineBuf(): Render the line buffer to the destination surface.
 * @param pixel Type of pixel.
 * @param dest Destination surface.
 * @param md_palette MD palette buffer.
 */
template<typename pixel>
FORCE_INLINE void VdpRend_m5::T_Render_LineBuf(pixel *dest, pixel *md_palette)
{
	const VdpRend::LineBuf_t::LineBuf_px_t *src = &VdpRend::LineBuf.px[8];
	
	// Render the line buffer to the destination surface.
	dest += VdpIo::H_Pix_Begin;
	for (unsigned int i = ((160 - VdpIo::H_Pix_Begin) / 4);
	     i != 0; i--, dest += 8, src += 8)
	{
		// TODO: Endianness conversions.
		*dest     = md_palette[src->pixel];
		*(dest+1) = md_palette[(src+1)->pixel];
		*(dest+2) = md_palette[(src+2)->pixel];
		*(dest+3) = md_palette[(src+3)->pixel];
		*(dest+4) = md_palette[(src+4)->pixel];
		*(dest+5) = md_palette[(src+5)->pixel];
		*(dest+6) = md_palette[(src+6)->pixel];
		*(dest+7) = md_palette[(src+7)->pixel];
	}
	
	if (VdpIo::H_Pix_Begin == 0)
		return;
	
	// Draw the borders.
	// NOTE: S/H is ignored if we're in the border region.
	
	// Get the border color.
	// TODO: Add the "borderColorEmulation" variable somewhere. (LibGens)
#if 0
	register const pixel border_color = (Video.borderColorEmulation ? md_palette[0] : 0);
#else
	register const pixel border_color = md_palette[0];
#endif
	
	// Left border.
	dest -= VdpIo::H_Pix_Begin;
	dest -= VdpIo::H_Pix;
	for (unsigned int i = (VdpIo::H_Pix_Begin / 8); i != 0; i--, dest += 8)
	{
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
	dest += VdpIo::H_Pix;
	for (unsigned int i = (VdpIo::H_Pix_Begin / 8); i != 0; i--, dest += 8)
	{
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
 * VdpRend_m5::Render_Line(): Render a line. (Mode 5)
 */
void VdpRend_m5::Render_Line(void)
{
	// Determine what part of the screen we're in.
	bool in_border = false;
	if (VdpIo::VDP_Lines.Visible.Current >= -VdpIo::VDP_Lines.Visible.Border_Size &&
	    VdpIo::VDP_Lines.Visible.Current < 0)
	{
		// Top border.
		in_border = true;
	}
	else if (VdpIo::VDP_Lines.Visible.Current >= VdpIo::VDP_Lines.Visible.Total &&
		 VdpIo::VDP_Lines.Visible.Current < (VdpIo::VDP_Lines.Visible.Total + VdpIo::VDP_Lines.Visible.Border_Size))
	{
		// Bottom border.
		in_border = true;
	}
	else if (VdpIo::VDP_Lines.Visible.Current < -VdpIo::VDP_Lines.Visible.Border_Size ||
		 VdpIo::VDP_Lines.Visible.Current >= (VdpIo::VDP_Lines.Visible.Total + VdpIo::VDP_Lines.Visible.Border_Size))
	{
		// Off screen.
		return;
	}
	
	// Determine the starting line in MD_Screen.
	int LineStart = VdpIo::VDP_Lines.Visible.Current;
	// TODO: Reimplement CPU_Mode and NTSC V30 scrolling. (LibGens)
#if 0
	if ((CPU_Mode == 0) && (VdpIo::m5.Set2 & 0x08) && Video.ntscV30rolling)
	{
		// NTSC V30 mode. Simulate screen rolling.
		LineStart -= VdpIo::VDP_Lines.NTSC_V30.Offset;
		
		// Prevent underflow.
		if (LineStart < 0)
			LineStart += 240;
	}
#endif
	LineStart = TAB336[LineStart + VdpIo::VDP_Lines.Visible.Border_Size] + 8;
	
	// TODO: Reimplement the borderColorEmulation option. (LibGens)
#if 0
	if (in_border && !Video.borderColorEmulation)
	{
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (bppMD == 32)
			memset(&MD_Screen.u32[LineStart], 0x00, 320*sizeof(uint32_t));
		else
			memset(&MD_Screen.u16[LineStart], 0x00, 320*sizeof(uint16_t));
		
		// ...and we're done here.
		return;
	}
#endif
	
	// Check if the VDP is enabled.
	bool VDP_Enabled = false;
	if (!in_border)
	{
		// HACK: There's a minor issue with the SegaCD firmware.
		// The firmware turns off the VDP after the last line,
		// which causes the entire screen to disappear if paused.
		// TODO: Don't rerun the VDP drawing functions when paused!
		
		if (VdpIo::VDP_Reg.m5.Set2 & 0x40)
			VDP_Enabled = true;
		// TODO: Reimplement Settings.Active and Settings.Paused. (LibGens)
#if 0
		else if ((!Settings.Active || Settings.Paused) && VdpIo::HasVisibleLines)
			VDP_Enabled = true;
#endif
	}
	
	if (!VDP_Enabled)
	{
		// VDP is disabled, or this is the border region.
		// Clear the line buffer.
		
		// NOTE: S/H is ignored if the VDP is disabled or if
		// we're in the border region.
		memset(VdpRend::LineBuf.u8, 0x00, sizeof(VdpRend::LineBuf.u8));

		// Clear the sprite dot overflow variable.
		VdpIo::SpriteDotOverflow = 0;
	}
	else
	{
		// VDP is enabled.
		
		// HACK: There's a minor issue with the SegaCD firmware.
		// The firmware turns off the VDP after the last line,
		// which causes the entire screen to disappear if paused.
		// TODO: Don't rerun the VDP drawing functions when paused!
		VdpIo::HasVisibleLines = 1;
		
		// Check if sprite structures need to be updated.
		if (VdpIo::Interlaced.DoubleRes)
		{
			// Interlaced.
			if (VdpIo::VDP_Flags.VRam)
				T_Make_Sprite_Struct<true, false>();
			else if (VdpIo::VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<true, true>();
		}
		else
		{
			// Non-Interlaced.
			if (VdpIo::VDP_Flags.VRam)
				T_Make_Sprite_Struct<false, false>();
			else if (VdpIo::VDP_Flags.VRam_Spr)
				T_Make_Sprite_Struct<false, true>();
		}
		
		// Clear the VRam flags.
		VdpIo::VDP_Flags.VRam = 0;
		VdpIo::VDP_Flags.VRam_Spr = 0;
		
		// Determine how to render the image.
		const int RenderMode = ((VdpIo::VDP_Reg.m5.Set4 & 0x8) >> 2) | VdpIo::Interlaced.DoubleRes;
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
	if (VdpIo::VDP_Flags.CRam)
	{
		// Update the palette.
		if (VdpIo::VDP_Reg.m5.Set4 & 0x08)
			VdpRend::m_palette.updateMD_HS(&VdpIo::CRam);
		else
			VdpRend::m_palette.updateMD(&VdpIo::CRam);
	}
	
	// Render the image.
	if (VdpRend::m_palette.bpp() != VdpPalette::BPP_32)
		T_Render_LineBuf<uint16_t>(&VdpRend::MD_Screen.u16[LineStart], VdpRend::m_palette.m_palActiveMD.u16);
	else
		T_Render_LineBuf<uint32_t>(&VdpRend::MD_Screen.u32[LineStart], VdpRend::m_palette.m_palActiveMD.u32);
}


// TODO: 32X stuff.
#if 0
/**
 * T_Render_LineBuf_32X(): Render the 32X line buffer to the destination surface.
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
	VRam_Ind += _32X_VDP_Ram.u16[VRam_Ind + VdpIo::VDP_Lines.Visible.Current];
	
	// Get the line buffer pointer.
	VdpRend::LineBuf_px_t *lbptr = &VdpRend::LineBuf.px[8];
	
	// Adjust the destination pointer for the horizontal resolution.
	// TODO: Draw horizontal borders, if necessary.
	dest += VDP_Reg.H_Pix_Begin;
	
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
			for (unsigned int px = VDP_Reg.H_Pix; px != 0; px -= 4, dest += 4, lbptr += 4)
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
			for (unsigned int px = VDP_Reg.H_Pix; px != 0; px -= 2, src += 2, dest += 2, lbptr += 2)
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
			for (unsigned int px = VDP_Reg.H_Pix; px != 0; px -= 2, src += 2, dest += 2, lbptr += 2)
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
			while (px < VDP_Reg.H_Pix)
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
				if (px_end >= VDP_Reg.H_Pix)
					px_end = (VDP_Reg.H_Pix - 1);
				
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
			for (unsigned int px = VDP_Reg.H_Pix; px != 0; px -= 2, src += 2, dest += 2, lbptr += 2)
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
			for (unsigned int px = VDP_Reg.H_Pix; px != 0; px--, dest++, lbptr++)
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
			for (unsigned int px = VDP_Reg.H_Pix; px != 0; px--, dest++, lbptr++)
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
	if (VdpIo::VDP_Lines.Visible.Current >= -VdpIo::VDP_Lines.Visible.Border_Size &&
	    VdpIo::VDP_Lines.Visible.Current < 0)
	{
		// Top border.
		in_border = true;
	}
	else if (VdpIo::VDP_Lines.Visible.Current >= VdpIo::VDP_Lines.Visible.Total &&
		 VdpIo::VDP_Lines.Visible.Current < (VdpIo::VDP_Lines.Visible.Total + VdpIo::VDP_Lines.Visible.Border_Size))
	{
		// Bottom border.
		in_border = true;
	}
	else if (VdpIo::VDP_Lines.Visible.Current < -VdpIo::VDP_Lines.Visible.Border_Size ||
		 VdpIo::VDP_Lines.Visible.Current >= (VdpIo::VDP_Lines.Visible.Total + VdpIo::VDP_Lines.Visible.Border_Size))
	{
		// Off screen.
		return;
	}
	
	// Determine the starting line in MD_Screen.
	int LineStart = VdpIo::VDP_Lines.Visible.Current;
	if ((CPU_Mode == 0) && (VDP_Reg.m5.Set2 & 0x08) && Video.ntscV30rolling)
	{
		// NTSC V30 mode. Simulate screen rolling.
		LineStart -= VdpIo::VDP_Lines.NTSC_V30.Offset;
		
		// Prevent underflow.
		if (LineStart < 0)
			LineStart += 240;
	}
	LineStart = TAB336[LineStart + VdpIo::VDP_Lines.Visible.Border_Size] + 8;
	
	if (in_border && !Video.borderColorEmulation)
	{
		// We're in the border area, but border color emulation is disabled.
		// Clear the border area.
		// TODO: Only clear this if the option changes or V/H mode changes.
		if (bppMD == 32)
			memset(&MD_Screen.u32[LineStart], 0x00, VDP_Reg.H_Pix*sizeof(uint32_t));
		else
			memset(&MD_Screen.u16[LineStart], 0x00, VDP_Reg.H_Pix*sizeof(uint16_t));
		
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
		memset(VdpRend::LineBuf.u8, 0x00, sizeof(VdpRend::LineBuf.u8));
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
		const int RenderMode = ((VDP_Reg.m5.Set4 & 0x8) >> 2) | VDP_Reg.Interlaced.DoubleRes;
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
