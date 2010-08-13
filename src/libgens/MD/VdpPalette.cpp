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

namespace LibGens
{

/** Static member initialization. **/
VdpPalette::Palette_t VdpPalette::Palette;
int VdpPalette::Contrast = 100;
int VdpPalette::Brightness = 100;
bool VdpPalette::Grayscale = false;
bool VdpPalette::InvertColor = false;
VdpPalette::ColorScaleMethod_t VdpPalette::ColorScaleMethod = VdpPalette::COLSCALE_FULL;


/**
 * VdpPalette::T_ConstrainColorComponent(): Constrains a color component.
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
 * VdpPalette::CalcGrayscale(): Calculate grayscale color values.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return Grayscale value.
 */
FORCE_INLINE int VdpPalette::CalcGrayscale(int r, int g, int b)
{
	// Standard grayscale computation: Y = R*0.30 + G*0.59 + B*0.11
	r = lrint((double)r * 0.30);
	g = lrint((double)g * 0.59);
	b = lrint((double)b * 0.11);
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
 * T_Recalc_MD(): Recalculates the MD palette for brightness, contrast, and various effects.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalette::T_Recalc_MD(pixel *palMD)
{
	int r, g, b;
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)

	const int brightness = (Brightness - 100);
	const int contrast = Contrast;
	
	int mdComponentScale;
	switch (ColorScaleMethod)
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
		if (ColorScaleMethod != COLSCALE_RAW)
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
		palMD[i] = (r << (BBits + GBits)) |
			   (g << (BBits)) |
			   (b);
		
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
		if (sizeof(pixel) == 4)
		{
			// HACK: Mac OS X on PowerPC uses BGRA format for 32-bit color.
			// The default palette calculation ends up using RGBA (or ARGB?).
			// (15-bit and 16-bit color appear to be fine...)
			// TODO: Check Linux/PPC.
			palMD[i] = le32_to_cpu(palMD[i]);
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
 * VdpPalette::Recalc(): Recalculate the VDP palette.
 */
void VdpPalette::Recalc(void)
{
	switch (VdpRend::Bpp)
	{
		case VdpRend::BPP_15:
			T_Recalc_MD<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(Palette.u16);
#if 0
			// TODO: Port to LibGens.
			T_Recalculate_Palette_32X<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>
					(_32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
#endif
			break;
		
		case VdpRend::BPP_16:
			T_Recalc_MD<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(Palette.u16);
#if 0
			// TODO: Port to LibGens.
			T_Recalculate_Palette_32X<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>
					(_32X_Palette.u16, _32X_VDP_CRam_Adjusted.u16);
#endif
			break;
		
		case VdpRend::BPP_32:
		default:
			T_Recalc_MD<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(Palette.u32);
#if 0
			// TODO: Port to LibGens.
			T_Recalculate_Palette_32X<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>
					(_32X_Palette.u32, _32X_VDP_CRam_Adjusted.u32);
#endif
			break;
	}
	
	// Set the CRam flag to force a palette update.
	VdpIo::VDP_Flags.CRam = 1;
	
	// TODO: Do_VDP_Only() / Do_32X_VDP_Only() if paused.
	
	// Force a wakeup.
	// TODO: Port to LibGens.
#if 0
	GensUI::wakeup();
#endif
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
