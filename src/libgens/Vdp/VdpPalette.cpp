/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpPalette.hpp: VDP palette handler.                                    *
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

#include "VdpPalette.hpp"

// FOR TESTING ONLY: Uncomment this #define to enable support for 4 palette lines in all modes.
// This should not be enabled in release builds!
//#define DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ

// TODO: Remove Vdp::UpdateFlags dependency.
#include "Vdp.hpp"
#include "VdpTypes.hpp"

#include "Util/byteswap.h"
#include "macros/common.h"

// C includes.
#include <math.h>
#include <string.h>

namespace LibGens
{

/**
 * VdpPalette private class.
 */
class VdpPalettePrivate
{
	public:
		static int FUNC_PURE ClampColorComponent(int mask, int c);
		static int FUNC_PURE CalcGrayscale(int r, int g, int b);
		static void FUNC_PURE AdjustContrast(int& r, int& g, int& b, int contrast);
};

/**
 * Clamp a color component to [0, mask].
 * @param mask Color component mask. (max value)
 * @param c Color component to clamp.
 * @return Clamped color component.
 */
int FUNC_PURE VdpPalettePrivate::ClampColorComponent(int mask, int c)
{
	if (c < 0)
		return 0;
	else if (c > mask)
		return mask;
	return c;
}

/**
 * Calculate grayscale color values.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return Grayscale value.
 */
int FUNC_PURE VdpPalettePrivate::CalcGrayscale(int r, int g, int b)
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
void FUNC_PURE VdpPalettePrivate::AdjustContrast(int& r, int& g, int& b, int contrast)
{
	if (contrast == 100)
		return;
	
	r = (r * contrast) / 100;
	g = (g * contrast) / 100;
	b = (b * contrast) / 100;
}

/** VdpPalette class. **/

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
	, m_palMode(PALMODE_MD)
	, m_bgColorIdx(0x00)
	, m_mdColorMask(MD_COLOR_MASK_FULL)
	, m_mdShadowHighlight(false)
{
	// Set the dirty flags.
	m_dirty.active = true;
	m_dirty.full = true;
	
	// Reset CRam and other palette variables.
	reset();
}


VdpPalette::~VdpPalette()
{
	// TODO
}


/**
 * reset(): Reset the palette, including CRam.
 */
void VdpPalette::reset(void)
{
	// Clear CRam.
	memset(&m_cram, 0x00, sizeof(m_cram));
	
	// Mark the active palette as dirty.
	m_dirty.active = true;
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
	m_dirty.full = true; \
}


/** Property write functions. **/
PAL_PROPERTY_WRITE(contrast, int, Contrast)
PAL_PROPERTY_WRITE(brightness, int, Brightness)
PAL_PROPERTY_WRITE(grayscale, bool, Grayscale)
PAL_PROPERTY_WRITE(inverted, bool, Inverted)
PAL_PROPERTY_WRITE(colorScaleMethod, ColorScaleMethod_t, ColorScaleMethod)
PAL_PROPERTY_WRITE(bpp, ColorDepth, Bpp)


/**
 * setPalMode(): Set the palette mode.
 * @param newPalMode New palette mode.
 */
void VdpPalette::setPalMode(PalMode_t newPalMode)
{
	if (m_palMode == newPalMode)
		return;
	m_palMode = newPalMode;
	
	// Both the full and active palettes must be recalculated.
	m_dirty.full = true;
	m_dirty.active = true;
}


/**
 * setBgColorIdx(): Set the background color index.
 * @param newBgColorIdx New background color index.
 */
void VdpPalette::setBgColorIdx(uint8_t newBgColorIdx)
{
	if (m_bgColorIdx == newBgColorIdx)
		return;
	
	// Mega Drive:
	m_bgColorIdx = (newBgColorIdx & 0x3F);
	
	// TODO: On SMS and Game Gear:
#if 0
	m_bgColorIdx &= 0x0F;
	m_bgColorIdx |= 0x10;
#endif
	
	// Mark the active palette as dirty.
	m_dirty.active = true;
}


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
	
	// Mark both full and active palettes as dirty.
	m_dirty.full = true;
	m_dirty.active = true;
}


/**
 * setMdShadowHighlight(): Set the MD Shadow/Highlight bit. (Mode 5 only)
 * @param newMdShadowHighlight If true, enables shadow/highlight.
 */
void VdpPalette::setMdShadowHighlight(bool newMdShadowHighlight)
{
	if (m_mdShadowHighlight == newMdShadowHighlight)
		return;
	
	m_mdShadowHighlight = newMdShadowHighlight;
	if (m_mdShadowHighlight)
	{
		// Shadow/Highlight was just enabled.
		// Active palette needs to be recalculated
		// in order to populate the S/H entries.
		m_dirty.active = true;
	}
}


/** SMS-specific functions. **/


/**
 * initSegaTMSPalette(): Initialize CRam with the SMS TMS9918 palette.
 * Only used on Sega Master System!
 * Palette mode must be set to PALMODE_SMS.
 * TODO: UNTESTED!
 */
void VdpPalette::initSegaTMSPalette(void)
{
	/**
	 * PalTMS9918_SMS[]: TMS9918 palette as used on the SMS. (6-bit RGB)
	 * Used in SMS backwards-compatibility mode.
	 * VdpPalette should be set to SMS mode to use this palette.
	 * Source: http://www.smspower.org/maxim/forumstuff/colours.html
	 * Reference: http://www.smspower.org/forums/viewtopic.php?t=8224
	 */
	static const uint8_t PalTMS9918_SMS[16] =
		{0x00, 0x00, 0x08, 0x0C, 0x10, 0x30, 0x01, 0x3C,
		 0x02, 0x03, 0x04, 0x0F, 0x04, 0x33, 0x15, 0x3F};
	 
	// TODO: Verify that palette mode is set to PALMODE_SMS.
	// TODO: Implement multiple palette modes.
	// TODO: Use alternating bytes in SMS CRam for MD compatibility?
	
	// Copy PalTMS9918_SMS to both SMS palettes in CRam.
	memcpy(&m_cram.u8[0x00], PalTMS9918_SMS, sizeof(PalTMS9918_SMS));
	memcpy(&m_cram.u8[0x10], PalTMS9918_SMS, sizeof(PalTMS9918_SMS));
#if defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	memcpy(&m_cram.u8[0x20], PalTMS9918_SMS, sizeof(PalTMS9918_SMS));
	memcpy(&m_cram.u8[0x30], PalTMS9918_SMS, sizeof(PalTMS9918_SMS));
#endif
	
	// Palette is dirty.
	m_dirty.active = true;
}


/** Palette update functions. **/


/** Full palette recalculation functions. **/


/**
 * T_recalcFull_MD(): Recalculate the full palette. (Mega Drive)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * Additionally, the MD color scale method is used to determine how colors are calculated.
 * @param palFull Full palette. (Must have enough space for at least 0x1000 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
void VdpPalette::T_recalcFull_MD(pixel *palFull)
{
	// MD color components.
	static const uint8_t PalComponent_MD_Raw[16] =
		{0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		 0xE0, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0};
	static const uint8_t PalComponent_MD_Full[16] =
		{  0,  18,  36,  54,  72,  91, 109, 127,
		 145, 163, 182, 200, 218, 236, 255, 255};
	static const uint8_t PalComponent_MD_Full_SH[16] =
		{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	
	const uint8_t *md_components;
	switch (m_colorScaleMethod)
	{
		case COLSCALE_RAW:
			md_components = &PalComponent_MD_Raw[0];
			break;
		case COLSCALE_FULL:
		default: // Matches Genesis 2; tested by TmEE.
			md_components = &PalComponent_MD_Full[0];
			break;
		case COLSCALE_FULL_SH:
			md_components = &PalComponent_MD_Full_SH[0];
			break;
	}
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)
	const int brightness = (m_brightness - 100);
	
	// Calculate the MD palette.
	for (int i = 0x0000; i < 0x1000; i++)
	{
		// Scale the color components using the lookup table.
		int r = md_components[i & 0x000F];
		int g = md_components[(i >> 4) & 0x000F];
		int b = md_components[(i >> 8) & 0x000F];
		
		// Adjust brightness.
		r += brightness;
		g += brightness;
		b += brightness;
		
		// Adjust contrast.
		VdpPalettePrivate::AdjustContrast(r, g, b, m_contrast);
		
		if (m_grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = VdpPalettePrivate::CalcGrayscale(r, g, b);
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);
		
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
		
		// Combine the color components.
		palFull[i] = (r << (BBits + GBits)) |
			     (g << (BBits)) |
			     (b);
	}
}


/**
 * T_recalcFull_32X(): Recalculate the full palette. (Sega 32X)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x10000 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalette::T_recalcFull_32X(pixel *palFull32X)
{
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)
	const int brightness = (m_brightness - 100);
	
	// Calculate the 32X palette. (first half)
	for (int i = 0x0000; i < 0x8000; i++)
	{
		// Sega 32X uses 15-bit color.
		// Scale each component by using the following algorithm:
		// - 32X component: abcde
		// - RGB component: abcdeabc
		// Example: 32X 0x15 (10101) -> RGB 0xAD (10101101)
		int r = ((i & 0x001F) << 3);	r |= (r >> 5);
		int g = ((i >> 2) & 0x00F8);	g |= (g >> 5);
		int b = ((i >> 7) & 0x00F8);	b |= (b >> 5);
		
		// Adjust brightness.
		r += brightness;
		g += brightness;
		b += brightness;
		
		// Adjust contrast.
		VdpPalettePrivate::AdjustContrast(r, g, b, m_contrast);
		
		if (m_grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = VdpPalettePrivate::CalcGrayscale(r, g, b);
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);
		
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
		
		// Combine the color components.
		palFull32X[i] = (r << (BBits + GBits)) |
				(g << (BBits)) |
				(b);
	}
	
	// Copy the palette from the first half of palFull32X to the second half.
	// TODO: Is it better to do this, or should we just mask palette entries by 0x7FFF?
	memcpy(&palFull32X[0x8000], &palFull32X[0], (0x8000 * sizeof(palFull32X[0])));
	
	// TODO: Port to LibGens.
	// TODO: Move this to T_update_32X()?
#if 0
	// Adjust the 32X VDP CRam.
	T_Adjust_CRam_32X<pixel>(pal32X, cramAdjusted32X);
#endif
}


/**
 * T_recalcFull_SMS(): Recalculate the full palette. (Sega Master System)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x40 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalette::T_recalcFull_SMS(pixel *palFull)
{
	// SMS color components. (EGA palette)
	static const uint8_t PalComponent_SMS[4] = {0x00, 0x55, 0xAA, 0xFF};
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)
	const int brightness = (m_brightness - 100);
	
	// Calculate the SMS palette.
	for (int i = 0x00; i < 0x3F; i++)
	{
		// Scale the color components using the lookup table.
		int r = PalComponent_SMS[i & 0x03];
		int g = PalComponent_SMS[(i >> 2) & 0x03];
		int b = PalComponent_SMS[(i >> 4) & 0x03];
		
		// Adjust brightness.
		r += brightness;
		g += brightness;
		b += brightness;
		
		// Adjust contrast.
		VdpPalettePrivate::AdjustContrast(r, g, b, m_contrast);
		
		if (m_grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = VdpPalettePrivate::CalcGrayscale(r, g, b);
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);
		
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
		
		// Combine the color components.
		palFull[i] = (r << (BBits + GBits)) |
			     (g << (BBits)) |
			     (b);
	}
}


/**
 * T_recalcFull_GG(): Recalculate the full palette. (Game Gear)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x1000 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalette::T_recalcFull_GG(pixel *palFull)
{
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)
	const int brightness = (m_brightness - 100);
	
	// Calculate the SMS palette.
	for (int i = 0x00; i < 0x1000; i++)
	{
		// Game Gear uses 12-bit color.
		// Scale each component by using the same 4 bits for each nybble.
		int r = (i & 0x000F);		r |= (r << 4);
		int g = ((i >> 4) & 0x000F);	g |= (g << 4);
		int b = ((i >> 8) & 0x000F);	b |= (b << 4);
		
		// Adjust brightness.
		r += brightness;
		g += brightness;
		b += brightness;
		
		// Adjust contrast.
		VdpPalettePrivate::AdjustContrast(r, g, b, m_contrast);
		
		if (m_grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = VdpPalettePrivate::CalcGrayscale(r, g, b);
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);
		
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
		
		// Combine the color components.
		palFull[i] = (r << (BBits + GBits)) |
			     (g << (BBits)) |
			     (b);
	}
}


/**
 * T_recalcFull_TMS9918(): Recalculate the full palette. (TMS9918)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x20 entries!)
 * NOTE: The palette is 16 colors, but we're saving it twice for compatibility purposes.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalette::T_recalcFull_TMS9918(pixel *palFull)
{
	/**
	 * PalTMS9918_Analog[] TMS9918 analog palette, in 32-bit RGB.
	 * Source: http://www.smspower.org/maxim/forumstuff/colours.html
	 * Reference: http://www.smspower.org/forums/viewtopic.php?t=8224
	 */
	// TODO: Byteswapping on big-endian?
	struct PalRGB_t { uint8_t a; uint8_t r; uint8_t g; uint8_t b; };
	static const PalRGB_t PalTMS9918_Analog[16] =
	{
		{0x00, 0x00, 0x00, 0x00},	// 0: Transparent
		{0x00, 0x00, 0x00, 0x00},	// 1: Black
		{0x00, 0x47, 0xB7, 0x3B},	// 2: Medium Green
		{0x00, 0x7C, 0xCF, 0x6F},	// 3: Light Green
		{0x00, 0x5D, 0x4E, 0xFF},	// 4: Dark Blue
		{0x00, 0x80, 0x72, 0xFF},	// 5: Light Blue
		{0x00, 0xB6, 0x62, 0x47},	// 6: Dark Red
		{0x00, 0x5D, 0xC8, 0xED},	// 7: Cyan
		{0x00, 0xD7, 0x68, 0x48},	// 8: Medium Red
		{0x00, 0xFB, 0x8F, 0x6C},	// 9: Light Red
		{0x00, 0xC3, 0xCD, 0x41},	// A: Dark Yellow
		{0x00, 0xD3, 0xDA, 0x76},	// B: Light Yellow
		{0x00, 0x3E, 0x9F, 0x2F},	// C: Dark Green
		{0x00, 0xB6, 0x64, 0xC7},	// D: Magenta
		{0x00, 0xCC, 0xCC, 0xCC},	// E: Gray
		{0x00, 0xFF, 0xFF, 0xFF},	// F: White
	};
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness == 100)
	// Normal contrast:   (  Contrast == 100)
	const int brightness = (m_brightness - 100);
	
	// Calculate the TMS9918 palette.
	for (int i = 0; i < 16; i++)
	{
		// TMS9918 uses analog color circuitry.
		// We're using close approximations of the colors as 32-bit RGB.
		// Source: http://www.smspower.org/maxim/forumstuff/colours.html
		int r = PalTMS9918_Analog[i].r;
		int g = PalTMS9918_Analog[i].g;
		int b = PalTMS9918_Analog[i].b;
		
		// Adjust brightness.
		r += brightness;
		g += brightness;
		b += brightness;
		
		// Adjust contrast.
		VdpPalettePrivate::AdjustContrast(r, g, b, m_contrast);
		
		if (m_grayscale)
		{
			// Convert the color to grayscale.
			r = g = b = VdpPalettePrivate::CalcGrayscale(r, g, b);
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);
		
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
		
		// Combine the color components.
		palFull[i] = (r << (BBits + GBits)) |
			     (g << (BBits)) |
			     (b);
	}
	
	// Copy the TMS9918 palette to the second SMS palettes.
	memcpy(&palFull[0x10], &palFull[0x00], (sizeof(palFull[0]) * 16));
}


/**
 * recalcFull(): Recalculate the full VDP palette.
 */
void VdpPalette::recalcFull(void)
{
	switch (m_bpp)
	{
		case BPP_15:
			switch (m_palMode)
			{
				case PALMODE_32X:
					T_recalcFull_32X<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(m_palFull32X.u16);
					// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
					// TODO: Add a separate dirty flag for the 32X palette?
				case PALMODE_MD:
				default:
					T_recalcFull_MD<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(m_palFull.u16);
					break;
				case PALMODE_SMS:
					T_recalcFull_SMS<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(m_palFull.u16);
					break;
				case PALMODE_GG:
					T_recalcFull_GG<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(m_palFull.u16);
					break;
				case PALMODE_TMS9918:
					T_recalcFull_TMS9918<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(m_palFull.u16);
					break;
			}
			break;
		
		case BPP_16:
			switch (m_palMode)
			{
				case PALMODE_32X:
					T_recalcFull_32X<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(m_palFull32X.u16);
					// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
					// TODO: Add a separate dirty flag for the 32X palette?
				case PALMODE_MD:
				default:
					T_recalcFull_MD<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(m_palFull.u16);
					break;
				case PALMODE_SMS:
					T_recalcFull_SMS<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(m_palFull.u16);
					break;
				case PALMODE_GG:
					T_recalcFull_GG<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(m_palFull.u16);
					break;
				case PALMODE_TMS9918:
					T_recalcFull_TMS9918<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(m_palFull.u16);
					break;
			}
			break;
		
		case BPP_32:
		default:
			switch (m_palMode)
			{
				case PALMODE_32X:
					T_recalcFull_32X<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(m_palFull32X.u32);
					// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
					// TODO: Add a separate dirty flag for the 32X palette?
				case PALMODE_MD:
				default:
					T_recalcFull_MD<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(m_palFull.u32);
					break;
				case PALMODE_SMS:
					T_recalcFull_SMS<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(m_palFull.u32);
					break;
				case PALMODE_GG:
					T_recalcFull_GG<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(m_palFull.u32);
					break;
				case PALMODE_TMS9918:
					T_recalcFull_TMS9918<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(m_palFull.u32);
					break;
			}
			break;
	}
	
	// TODO: Do_VDP_Only() / Do_32X_VDP_Only() if paused.
	
	// Force a wakeup.
	// TODO: Port to LibGens.
#if 0
	GensUI::wakeup();
#endif
	
	// Full palette is recalculated.
	// Active palette needs to be recalculated.
	m_dirty.full = false;
	m_dirty.active = true;
}


/** Active palette recalculation functions. **/


/**
 * T_update_MD(): Recalculate the active palette. (Mega Drive, Mode 5)
 * @param MD_palette MD color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_MD(pixel *MD_palette,
					const pixel *palette)
{
	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2)
	{
		const uint16_t color1_raw = (m_cram.u16[i] & m_mdColorMask);
		const uint16_t color2_raw = (m_cram.u16[i + 1] & m_mdColorMask);
		
		// Get the palette color.
		pixel color1 = palette[color1_raw];
		pixel color2 = palette[color2_raw];
		
		// Set the new color.
		MD_palette[i]     = color1;
		MD_palette[i + 1] = color2;
	}
	
	// Update the background color.
	MD_palette[0] = MD_palette[m_bgColorIdx];
	
	if (m_mdShadowHighlight)
	{
		// Update the shadow and highlight colors.
		// References:
		// - http://www.tehskeen.com/forums/showpost.php?p=71308&postcount=1077
		// - http://forums.sonicretro.org/index.php?showtopic=17905
		
		// Shadow (64-127) and highlight (128-191) palettes.
		for (int i = 62; i >= 0; i -= 2)
		{
			uint16_t color1_raw = ((m_cram.u16[i] & m_mdColorMask) >> 1);
			uint16_t color2_raw = ((m_cram.u16[i + 1] & m_mdColorMask) >> 1);
			
			// Shadow color. (0xxx)
			MD_palette[i + 64]	= palette[color1_raw];
			MD_palette[i + 1 + 64]	= palette[color2_raw];
			
			// Highlight color. (1xxx - 0001)
			MD_palette[i + 128]	= palette[(0x888 | color1_raw) - 0x111];
			MD_palette[i + 1 + 128]	= palette[(0x888 | color2_raw) - 0x111];
		}
		
		// Copy the normal colors (0-63) to shadow+highlight (192-255).
		// Pixels with both shadow and highlight show up as normal.
		memcpy(&MD_palette[192], &MD_palette[0], (sizeof(MD_palette[0]) * 64));
		
		// Update the background color for the shadow and highlight palettes.
		MD_palette[64]  = MD_palette[m_bgColorIdx + 64];	// Shadow color.
		MD_palette[128] = MD_palette[m_bgColorIdx + 128];	// Highlight color.
	}
}


/**
 * T_update_SMS(): Recalculate the active palette. (Sega Master System, Mode 4)
 * TODO: UNTESTED!
 * @param SMS_palette SMS color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_SMS(pixel *SMS_palette,
					const pixel *palette)
{
#if !defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	// Update all 32 colors.
	static const int color_start = (32 - 2);
#else
	// Process all 64 colors for lulz.
	static const int color_start = (64 - 2);
#endif
	
	for (int i = color_start; i >= 0; i -= 2)
	{
		// TODO: Use alternating bytes in SMS CRam for MD compatibility?
		const uint8_t color1_raw = (m_cram.u8[i] & 0x3F);
		const uint8_t color2_raw = (m_cram.u8[i + 1] & 0x3F);
		
		// Get the palette color.
		pixel color1 = palette[color1_raw];
		pixel color2 = palette[color2_raw];
		
		// Set the new color.
		SMS_palette[i]     = color1;
		SMS_palette[i + 1] = color2;
	}
	
	// Update the background color.
	SMS_palette[0] = SMS_palette[m_bgColorIdx];
}


/**
 * T_update_GG(): Recalculate the active palette. (Sega Game Gear, Mode 4 [12-bit RGB])
 * TODO: UNTESTED!
 * @param GG_palette Game Gear color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_GG(pixel *GG_palette,
					const pixel *palette)
{
#if !defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	// Update all 32 colors.
	static const int color_start = (32 - 2);
#else
	// Process all 64 colors for lulz.
	static const int color_start = (64 - 2);
#endif
	
	for (int i = color_start; i >= 0; i -= 2)
	{
		const uint16_t color1_raw = (m_cram.u16[i] & 0xFFF);
		const uint16_t color2_raw = (m_cram.u16[i + 1] & 0xFFF);
		
		// Get the palette color.
		pixel color1 = palette[color1_raw];
		pixel color2 = palette[color2_raw];
		
		// Set the new color.
		GG_palette[i]     = color1;
		GG_palette[i + 1] = color2;
	}
	
	// Update the background color.
	GG_palette[0] = GG_palette[m_bgColorIdx];
}


/**
 * T_update_TMS9918(): Recalculate the active palette. (TMS9918)
 * TODO: UNTESTED!
 * @param GG_palette Game Gear color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_TMS9918(pixel *TMS_palette,
					const pixel *palette)
{
	/**
	 * NOTE: This function doesn't actually recalculate palettes.
	 * It simply copies the full 16-color palette to the active palette twice.
	 * The palette is copied twice for compatibility purposes.
	 */
	
	// Copy the colors.
	memcpy(&TMS_palette[0x00], &palette[0x00], (sizeof(TMS_palette[0]) * 16));
	memcpy(&TMS_palette[0x10], &palette[0x00], (sizeof(TMS_palette[0]) * 16));
#if defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	memcpy(&TMS_palette[0x20], &palette[0x00], (sizeof(TMS_palette[0]) * 16));
	memcpy(&TMS_palette[0x30], &palette[0x00], (sizeof(TMS_palette[0]) * 16));
#endif
	
	// Update the background color.
	// TODO: How is the background color handled in TMS9918 modes?
	//TMS_palette[0] = TMS_palette[m_bgColorIdx];
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


/**
 * update(): Update the active palette.
 */
void VdpPalette::update(void)
{
	if (m_dirty.full)
		recalcFull();
	if (!m_dirty.active)
		return;
	
	if (m_bpp != BPP_32)
	{
		switch (m_palMode)
		{
			case PALMODE_32X:
				// TODO: Implement T_update_32X().
#if 0
				T_update_32X<uint16_t>(m_palActive32X.u16, m_palFull32X.u16);
#endif
				// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
				// TODO: Add a separate dirty flag for the 32X palette?
			
			case PALMODE_MD:
			default:
				T_update_MD<uint16_t>(m_palActive.u16, m_palFull.u16);
				break;
			
			case PALMODE_SMS:
				T_update_SMS<uint16_t>(m_palActive.u16, m_palFull.u16);
				break;
			
			case PALMODE_GG:
				T_update_GG<uint16_t>(m_palActive.u16, m_palFull.u16);
				break;
			
			case PALMODE_TMS9918:
				T_update_TMS9918<uint16_t>(m_palActive.u16, m_palFull.u16);
				break;
		}
	}
	else
	{
		switch (m_palMode)
		{
			case PALMODE_32X:
				// TODO: Implement T_update_32X().
#if 0
				T_update_32X<uint32_t>(m_palActive32X.u32, m_palFull32X.u32);
#endif
				// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
				// TODO: Add a separate dirty flag for the 32X palette?
			
			case PALMODE_MD:
			default:
				T_update_MD<uint32_t>(m_palActive.u32, m_palFull.u32);
				break;
			
			case PALMODE_SMS:
				T_update_SMS<uint32_t>(m_palActive.u32, m_palFull.u32);
				break;
			
			case PALMODE_GG:
				T_update_GG<uint32_t>(m_palActive.u32, m_palFull.u32);
				break;
			
			case PALMODE_TMS9918:
				T_update_TMS9918<uint32_t>(m_palActive.u32, m_palFull.u32);
				break;
		}
	}
	
	// Clear the active palette dirty bit.
	m_dirty.active = false;
}


// TODO: Port to LibGens: T_update_32X()
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


/** ZOMG savestate functions. **/


/**
 * zomgSaveCRam(): Save the CRam.
 * @param state Zomg_PsgSave_t struct to save to.
 */
void VdpPalette::zomgSaveCRam(Zomg_CRam_t *cram)
{
	// TODO: Support systems other than MD.
	memcpy(cram->md, m_cram.u16, sizeof(cram->md));
}


void VdpPalette::zomgRestoreCRam(const Zomg_CRam_t *cram)
{
	// TODO: Support systems other than MD.
	memcpy(m_cram.u16, cram->md, sizeof(cram->md));
	
	// Mark the active palette as dirty.
	m_dirty.active = true;
}

}
