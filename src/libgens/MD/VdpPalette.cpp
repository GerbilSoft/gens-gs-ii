/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpPalette.hpp: VDP palette handler.                                    *
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

#include "VdpPalette.hpp"

#include "VdpRend.hpp"
#include "VdpIo.hpp"

#include "Util/byteswap.h"

// C includes.
#include <math.h>
#include <stdint.h>
#include <string.h>

namespace LibGens
{

VdpPalette::VdpPalette()
{
	// Set defaults.
	// NOTE: bpp is initialized to 32 for now because
	// radeong_dri.so is really slow with 16-bit color.
	m_contrast = 100;
	m_brightness = 100;
	m_grayscale = false;
	m_invertColor = false;
	m_csm = COLSCALE_FULL;
	m_bpp = BPP_32;
	m_dirty = true;
	
	// TODO: Should we recalculate the palette now or wait?
	recalcFull();
	
	// Reset the active palettes.
	resetActive();
}


VdpPalette::~VdpPalette()
{
	// TODO
}


/**
 * T_ConstrainColorComponent(): Constrains a color component.
 * @param mask Color component mask. (max value)
 * @param c Color component to constrain.
 */
template<int mask>
FORCE_INLINE void VdpPalette::T_ConstrainColorComponent(int& c)
{
	if (c < 0)
		c = 0;
	else if (c > mask)
		c = mask;
}


/**
 * CalcGrayscale(): Calculate grayscale color values.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return Grayscale value.
 */
FORCE_INLINE int VdpPalette::CalcGrayscale(int r, int g, int b)
{
	// Convert the color components to grayscale.
	// Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
	// Source: http://en.wikipedia.org/wiki/YCbCr
	r = lrint((double)r * 0.299);
	g = lrint((double)g * 0.587);
	b = lrint((double)b * 0.114);
	return (r + g + b);
}


/**
 * AdjustContrast(): Adjust the contrast of the specified color.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 */
FORCE_INLINE void VdpPalette::AdjustContrast(int& r, int& g, int& b, int contrast)
{
	if (contrast == 100)
		return;
	
	r = (r * contrast) / 100;
	g = (g * contrast) / 100;
	b = (b * contrast) / 100;
}


/**
 * T_recalcFullMD(): Recalculates the full MD palette for brightness, contrast, and various effects.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalette::T_recalcFullMD(pixel *palFull)
{
	int r, g, b;
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)

	const int brightness = (m_brightness - 100);
	
	int mdComponentScale;
	switch (m_csm)
	{
		case COLSCALE_RAW:
			mdComponentScale = 0;
			break;
		case COLSCALE_FULL:
			mdComponentScale = 0xE0;
			break;
		case COLSCALE_FULL_HS:
		default:
			mdComponentScale = 0xF0;
			break;
	}
	
	// Calculate the MD palette.
	for (unsigned int i = 0x0000; i < 0x1000; i++)
	{
		// Process using 8-bit color components.
		r = (i & 0x000F) << 4;
		g = (i & 0x00F0);
		b = (i >> 4) & 0xF0;
		
		// Scale the colors to full RGB.
		if (m_csm != COLSCALE_RAW)
		{
			r = (r * 0xFF) / mdComponentScale;
			g = (g * 0xFF) / mdComponentScale;
			b = (b * 0xFF) / mdComponentScale;
		}
		
		// Adjust brightness.
		if (brightness != 0)
		{
			r += brightness;
			g += brightness;
			b += brightness;
		}
		
		// Adjust contrast.
		AdjustContrast(r, g, b, m_contrast);
		
		if (m_grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = CalcGrayscale(r, g, b);
		}
		
		if (m_invertColor)
		{
			// Invert the color components.
			r ^= 0xFF;
			g ^= 0xFF;
			b ^= 0xFF;
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Constrain the color components.
		T_ConstrainColorComponent<RMask>(r);
		T_ConstrainColorComponent<GMask>(g);
		T_ConstrainColorComponent<BMask>(b);
		
		// TODO: Make this configurable?
#if 0	
		if (GMask == 0x3F)
		{
			// 16-bit color. (RGB565)
			// Mask off the LSB of the green component.
			g &= ~1;
		}
#endif
		
		// Create the color.
		palFull[i] = (r << (BBits + GBits)) |
			     (g << (BBits)) |
			     (b);
		
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
		if (sizeof(pixel) == 4)
		{
			// HACK: Mac OS X on PowerPC uses BGRA format for 32-bit color.
			// The default palette calculation ends up using RGBA (or ARGB?).
			// (15-bit and 16-bit color appear to be fine...)
			// TODO: Check Linux/PPC.
			palFull[i] = le32_to_cpu(palFull[i]);
		}
#endif
	}
}


// TODO: Port to LibGens.
#if 0
/**
 * T_Adjust_CRam_32X(): Adjust the 32X CRam.
 */
template<typename pixel>
static FORCE_INLINE void T_Adjust_CRam_32X(pixel *pal32X, pixel *cramAdjusted32X)
{
	for (int i = 0; i < 0x100; i += 4)
	{
		cramAdjusted32X[i] = pal32X[_32X_VDP_CRam[i]];
		cramAdjusted32X[i+1] = pal32X[_32X_VDP_CRam[i+1]];
		cramAdjusted32X[i+2] = pal32X[_32X_VDP_CRam[i+2]];
		cramAdjusted32X[i+3] = pal32X[_32X_VDP_CRam[i+3]];
	}
}
#endif


// TODO: Port to LibGens.
#if 0
/**
 * T_Recalculate_Palette_32X(): Recalculates the 32X palette for brightness, contrast, and various effects.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
static inline void T_Recalculate_Palette_32X(pixel *pal32X, pixel *cramAdjusted32X)
{
	int r, g, b;
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)

	const int brightness = (Brightness - 100);
	const int contrast = Contrast;
	
	// Calculate the 32X palette.
	for (unsigned int i = 0; i < 0x10000; i++)
	{
		// Process using 8-bit color components.
		r = (i & 0x1F) << 3;
		g = (i >> 2) & 0xF8;
		b = (i >> 7) & 0xF8;
		
		// Scale the colors to full RGB.
		if (ColorScaleMethod != COLSCALE_RAW)
		{
			r = (r * 0xFF) / 0xF8;
			g = (g * 0xFF) / 0xF8;
			b = (b * 0xFF) / 0xF8;
		}
		
		// Adjust brightness.
		if (brightness != 0)
		{
			r += brightness;
			g += brightness;
			b += brightness;
		}
		
		// Adjust contrast.
		AdjustContrast(r, g, b, contrast);
		
		if (Grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = CalcGrayscale(r, g, b);
		}
		
		if (InvertColor)
		{
			// Invert the color components.
			r ^= 0xFF;
			g ^= 0xFF;
			b ^= 0xFF;
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Constrain the color components.
		T_ConstrainColorComponent<RMask>(r);
		T_ConstrainColorComponent<GMask>(g);
		T_ConstrainColorComponent<BMask>(b);
		
		if (GMask == 0x3F)
		{
			// 16-bit color. (RGB565)
			// Mask off the LSB of the green component.
			g &= ~1;
		}
		
		// Create the color.
		pal32X[i] = (r << (GBits + BBits)) |
			    (g << (BBits)) |
			    (b);
	}
	
	// Adjust the 32X VDP CRam.
	T_Adjust_CRam_32X<pixel>(pal32X, cramAdjusted32X);
}
#endif


/**
 * recalcFull(): Recalculate the full VDP palette.
 */
void VdpPalette::recalcFull(void)
{
	switch (m_bpp)
	{
		case BPP_15:
			T_recalcFullMD<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(m_palette.u16);
#if 0
			// TODO: Port to LibGens.
			T_Recalculate_Palette_32X<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>
					(_32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
#endif
			break;
		
		case BPP_16:
			T_recalcFullMD<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(m_palette.u16);
#if 0
			// TODO: Port to LibGens.
			T_Recalculate_Palette_32X<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>
					(_32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
#endif
			break;
		
		case BPP_32:
		default:
			T_recalcFullMD<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(m_palette.u32);
#if 0
			// TODO: Port to LibGens.
			T_Recalculate_Palette_32X<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>
					(_32X_Palette.u32, _32X_VDP_CRam_Adjusted.u32);
#endif
			break;
	}
	
	// Set the CRam flag to force a palette update.
	// TODO: This is being done from a class instance...
	// Figure out a better way to handle this.
	VdpIo::VDP_Flags.CRam = 1;
	
	// TODO: Do_VDP_Only() / Do_32X_VDP_Only() if paused.
	
	// Force a wakeup.
	// TODO: Port to LibGens.
#if 0
	GensUI::wakeup();
#endif
	
	// Clear the dirty bit.
	m_dirty = false;
}


/**
 * resetActive(): Reset the active palettes.
 */
void VdpPalette::resetActive(void)
{
	memset(&m_palActiveMD, 0x00, sizeof(m_palActiveMD));
}


/**
 * T_updateMD(): MD VDP palette update function.
 * @param hs If true, updates highlight/shadow.
 * @param MD_palette MD color palette.
 * @param palette Full color palette.
 * @param cram CRam.
 */
template<bool hs, typename pixel>
FORCE_INLINE void VdpPalette::T_updateMD(pixel *MD_palette, const pixel *palette, const VdpIo::VDP_CRam_t *cram)
{
	// TODO: Figure out a better way to handle this.
	if (VdpRend::VDP_Layers & VdpRend::VDP_LAYER_PALETTE_LOCK)
		return;
	
	// Clear the CRam flag, since the palette is being updated.
	// TODO: Don't do this from a class instance?
	VdpIo::VDP_Flags.CRam = 0;
	
	// Color mask. Depends on VDP register 0, bit 2 (Palette Select).
	// If set, allows full MD palette.
	// If clear, only allows the LSB of each color component.
	// TODO: Figure out a better way to handle this. (class instance etc)
	const uint16_t color_mask = (VdpIo::VDP_Reg.m5.Set1 & 0x04) ? 0x0EEE : 0x0222;
	
	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2)
	{
		uint16_t color1_raw = cram->u16[i] & color_mask;
		uint16_t color2_raw = cram->u16[i + 1] & color_mask;
		
		// Get the palette color.
		pixel color1 = palette[color1_raw];
		pixel color2 = palette[color2_raw];
		
		// Set the new color.
		MD_palette[i]     = color1;
		MD_palette[i + 1] = color2;
		
		if (hs)
		{
			// Update the highlight and shadow colors.
			// References:
			// - http://www.tehskeen.com/forums/showpost.php?p=71308&postcount=1077
			// - http://forums.sonicretro.org/index.php?showtopic=17905
			
			// Normal color. (xxx0)
			MD_palette[i + 192]	= color1;
			MD_palette[i + 1 + 192]	= color2;
			
			color1_raw >>= 1;
			color2_raw >>= 1;
			
			// Shadow color. (0xxx)
			MD_palette[i + 64]	= palette[color1_raw];
			MD_palette[i + 1 + 64]	= palette[color2_raw];
			
			// Highlight color. (1xxx - 0001)
			MD_palette[i + 128]	= palette[(0x888 | color1_raw) - 0x111];
			MD_palette[i + 1 + 128]	= palette[(0x888 | color2_raw) - 0x111];
		}
	}
	
	// Update the background color.
	unsigned int BG_Color = (VdpIo::VDP_Reg.m5.BG_Color & 0x3F);
	MD_palette[0] = MD_palette[BG_Color];
	
	if (hs)
	{
		// Update the background color for highlight and shadow.
		
		// Normal color.
		MD_palette[192] = MD_palette[BG_Color];
		
		// Shadow color.
		MD_palette[64] = MD_palette[BG_Color + 64];
		
		// Highlight color.
		MD_palette[128] = MD_palette[BG_Color + 128];
	}
}


/**
 * updateMD(): Update the active MD palette.
 * @param cram MD CRam.
 */
void VdpPalette::updateMD(const VdpIo::VDP_CRam_t *cram)
{
	if (m_dirty)
		recalcFull();
	
	if (m_bpp != BPP_32)
		T_updateMD<false, uint16_t>(m_palActiveMD.u16, m_palette.u16, cram);
	else
		T_updateMD<false, uint32_t>(m_palActiveMD.u32, m_palette.u32, cram);
}


/**
 * updateMD(): Update the active MD palette, including shadow/highlight.
 * @param cram MD CRam.
 */
void VdpPalette::updateMD_HS(const VdpIo::VDP_CRam_t *cram)
{
	if (m_dirty)
		recalcFull();
	
	if (m_bpp != BPP_32)
		T_updateMD<true, uint16_t>(m_palActiveMD.u16, m_palette.u16, cram);
	else
		T_updateMD<true, uint32_t>(m_palActiveMD.u32, m_palette.u32, cram);
}


// TODO: Port to LibGens.
#if 0
/**
 * Adjust_CRam_32X(): Adjust the 32X CRam.
 */
void Adjust_CRam_32X(void)
{
	if (bppMD != 32)
		T_Adjust_CRam_32X<uint16_t>(_32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
	else
		T_Adjust_CRam_32X<uint32_t>(_32X_Palette.u32, _32X_VDP_CRam_Adjusted.u32);
}
#endif

}
