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

// TODO: Remove Vdp::UpdateFlags dependency.
#include "Vdp.hpp"
#include "VdpTypes.hpp"

#include "Util/byteswap.h"

// C includes.
#include <math.h>
#include <stdint.h>
#include <string.h>

namespace LibGens
{

/** Static member initialization. **/
const uint16_t VdpPalette::MD_COLOR_MASK_FULL = 0xEEE;
const uint16_t VdpPalette::MD_COLOR_MASK_LSB = 0x222;

/**
 * VdpPalette(): Initialize a new VdpPalette object.
 * 
 * NOTE: bpp is initialized to 32 for now because
 * radeong_dri.so is really slow with 16-bit color.
 * TODO: Check with recent Gallium3D updates!
 */
VdpPalette::VdpPalette()
	: m_contrast(100)
	, m_brightness(100)
	, m_grayscale(false)
	, m_inverted(false)
	, m_colorScaleMethod(COLSCALE_FULL)
	, m_bpp(BPP_32)
	, m_mdColorMask(MD_COLOR_MASK_FULL)
	, m_dirty(true)
{
	// Reset the active palettes.
	resetActive();
}


VdpPalette::~VdpPalette()
{
	// TODO
}


/**
 * PAL_PROPERTY_WRITE(): Property write function macro..
 */
#define PAL_PROPERTY_WRITE(propName, setPropType, setPropName) \
void VdpPalette::set##setPropName(setPropType new##setPropName) \
{ \
	if (m_##propName == (new##setPropName)) \
		return; \
	m_##propName = (new##setPropName); \
	m_dirty = true; \
}


/** Property write functions. **/
PAL_PROPERTY_WRITE(contrast, int, Contrast)
PAL_PROPERTY_WRITE(brightness, int, Brightness)
PAL_PROPERTY_WRITE(grayscale, bool, Grayscale)
PAL_PROPERTY_WRITE(inverted, bool, Inverted)
PAL_PROPERTY_WRITE(colorScaleMethod, ColorScaleMethod_t, ColorScaleMethod)
PAL_PROPERTY_WRITE(bpp, ColorDepth, Bpp)


/**
 * setMdColorMask(): Set the MD color mask. (Mode 5 only)
 * @param newMdColorMask If true, masks all but LSBs.
 */
void VdpPalette::setMdColorMask(bool newMdColorMask)
{
	const uint16_t newMask = (newMdColorMask
				? MD_COLOR_MASK_LSB
				: MD_COLOR_MASK_FULL);
	
	if (m_mdColorMask == newMask)
		return;
	m_mdColorMask = newMask;
	m_dirty = true;
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
	switch (m_colorScaleMethod)
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
		if (m_colorScaleMethod != COLSCALE_RAW)
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
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Constrain the color components.
		T_ConstrainColorComponent<RMask>(r);
		T_ConstrainColorComponent<GMask>(g);
		T_ConstrainColorComponent<BMask>(b);
		
		if (m_inverted)
		{
			// Invert the color components.
			r ^= RMask;
			g ^= GMask;
			b ^= BMask;
		}
		
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
	Vdp::MarkCRamDirty();
	
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
FORCE_INLINE void VdpPalette::T_updateMD(pixel *MD_palette,
					const pixel *palette,
					const VdpTypes::CRam_t *cram)
{
	// TODO: Figure out a better way to handle this.
	if (Vdp::VDP_Layers & VdpTypes::VDP_LAYER_PALETTE_LOCK)
		return;
	
	// NOTE: We can't clear the CRam flag in the VDP class here.
	// This function is called by the VDP class, so the VDP class
	// can clear the flag itself.
	
	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2)
	{
		uint16_t color1_raw = cram->u16[i] & m_mdColorMask;
		uint16_t color2_raw = cram->u16[i + 1] & m_mdColorMask;
		
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
	unsigned int BG_Color = (Vdp::VDP_Reg.m5.BG_Color & 0x3F);
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
void VdpPalette::updateMD(const VdpTypes::CRam_t *cram)
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
void VdpPalette::updateMD_HS(const VdpTypes::CRam_t *cram)
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

