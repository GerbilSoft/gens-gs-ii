/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpPalette_recalc.hpp: VDP palette handler: Full recalc functions.      *
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

#include "VdpPalette_p.hpp"

namespace LibGens {

/** VdpPalettePrivate **/

/**
 * Recalculate the full palette. (Mega Drive)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * Additionally, the MD color scale method is used to determine how colors are calculated.
 * @param palFull Full palette. (Must have enough space for at least 0x1000 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
void VdpPalettePrivate::T_recalcFull_MD(pixel *palFull)
{
	// MD color components.
	// These values match a Genesis 2, as tested by TmEE.
	static const uint8_t PalComponent_MD[16] =
		{  0,  18,  36,  54,  72,  91, 109, 127,
		 145, 163, 182, 200, 218, 236, 255, 255};

	// Calculate the MD palette.
	for (int i = 0x0000; i < 0x1000; i++) {
		// Scale the color components using the lookup table.
		int r = PalComponent_MD[i & 0x000F];
		int g = PalComponent_MD[(i >> 4) & 0x000F];
		int b = PalComponent_MD[(i >> 8) & 0x000F];

		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);

		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);

		// TODO: Make this configurable?
#if 0	
		if (GMask == 0x3F) {
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
 * Recalculate the full palette. (Sega 32X)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x10000 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalettePrivate::T_recalcFull_32X(pixel *palFull32X)
{
	// Calculate the 32X palette. (first half)
	for (int i = 0x0000; i < 0x8000; i++) {
		// Sega 32X uses 15-bit color.
		// Scale each component by using the following algorithm:
		// - 32X component: abcde
		// - RGB component: abcdeabc
		// Example: 32X 0x15 (10101) -> RGB 0xAD (10101101)
		int r = ((i & 0x001F) << 3);	r |= (r >> 5);
		int g = ((i >> 2) & 0x00F8);	g |= (g >> 5);
		int b = ((i >> 7) & 0x00F8);	b |= (b >> 5);

		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);

		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);

		// TODO: Make this configurable?
#if 0	
		if (GMask == 0x3F) {
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
 * Recalculate the full palette. (Sega Master System)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x40 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalettePrivate::T_recalcFull_SMS(pixel *palFull)
{
	// SMS color components. (EGA palette)
	static const uint8_t PalComponent_SMS[4] = {0x00, 0x55, 0xAA, 0xFF};

	// Calculate the SMS palette.
	for (int i = 0x00; i < 0x40; i++) {
		// Scale the color components using the lookup table.
		int r = PalComponent_SMS[i & 0x03];
		int g = PalComponent_SMS[(i >> 2) & 0x03];
		int b = PalComponent_SMS[(i >> 4) & 0x03];

		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);
		
		// TODO: Make this configurable?
#if 0	
		if (GMask == 0x3F) {
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
 * Recalculate the full palette. (Game Gear)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x1000 entries!)
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalettePrivate::T_recalcFull_GG(pixel *palFull)
{
	// Calculate the SMS palette.
	for (int i = 0x0000; i < 0x1000; i++) {
		// Game Gear uses 12-bit color.
		// Scale each component by using the same 4 bits for each nybble.
		int r = (i & 0x000F);		r |= (r << 4);
		int g = ((i >> 4) & 0x000F);	g |= (g << 4);
		int b = ((i >> 8) & 0x000F);	b |= (b << 4);

		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);

		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);

		// TODO: Make this configurable?
#if 0	
		if (GMask == 0x3F) {
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
 * Recalculate the full palette. (TMS9918)
 * This applies brightness, contrast, grayscale, and inverted palette settings.
 * TODO: UNTESTED!
 * @param palFull Full palette. (Must have enough space for at least 0x20 entries!)
 * NOTE: The palette is 16 colors, but we're saving it twice for compatibility purposes.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
FORCE_INLINE void VdpPalettePrivate::T_recalcFull_TMS9918(pixel *palFull)
{
	/**
	 * PalTMS9918_Analog[] TMS9918 analog palette, in 32-bit RGB.
	 * Source: http://www.smspower.org/maxim/forumstuff/colours.html
	 * Reference: http://www.smspower.org/forums/viewtopic.php?t=8224
	 */
	// TODO: Byteswapping on big-endian?
	struct { uint8_t a; uint8_t r; uint8_t g; uint8_t b; }
	PalTMS9918_Analog[16] = {
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

	// Calculate the TMS9918 palette.
	for (int i = 0; i < 16; i++) {
		// TMS9918 uses analog color circuitry.
		// We're using close approximations of the colors as 32-bit RGB.
		// Source: http://www.smspower.org/maxim/forumstuff/colours.html
		int r = PalTMS9918_Analog[i].r;
		int g = PalTMS9918_Analog[i].g;
		int b = PalTMS9918_Analog[i].b;

		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Clamp the color components.
		r = VdpPalettePrivate::ClampColorComponent(RMask, r);
		g = VdpPalettePrivate::ClampColorComponent(GMask, g);
		b = VdpPalettePrivate::ClampColorComponent(BMask, b);

		// TODO: Make this configurable?
#if 0	
		if (GMask == 0x3F) {
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
 * Recalculate the full VDP palette.
 */
void VdpPalettePrivate::recalcFull(void)
{
	switch (q->m_bpp) {
		case MdFb::BPP_15:
			switch (this->palMode) {
				case VdpPalette::PALMODE_32X:
					T_recalcFull_32X<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(palFull32X.u16);
					// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
					// TODO: Add a separate dirty flag for the 32X palette?
				case VdpPalette::PALMODE_MD:
				default:
					T_recalcFull_MD<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(palFull.u16);
					break;
				case VdpPalette::PALMODE_SMS:
					T_recalcFull_SMS<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(palFull.u16);
					break;
				case VdpPalette::PALMODE_GG:
					T_recalcFull_GG<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(palFull.u16);
					break;
				case VdpPalette::PALMODE_TMS9918:
					T_recalcFull_TMS9918<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(palFull.u16);
					break;
			}
			break;

		case MdFb::BPP_16:
			switch (this->palMode) {
				case VdpPalette::PALMODE_32X:
					T_recalcFull_32X<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(palFull32X.u16);
					// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
					// TODO: Add a separate dirty flag for the 32X palette?
				case VdpPalette::PALMODE_MD:
				default:
					T_recalcFull_MD<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(palFull.u16);
					break;
				case VdpPalette::PALMODE_SMS:
					T_recalcFull_SMS<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(palFull.u16);
					break;
				case VdpPalette::PALMODE_GG:
					T_recalcFull_GG<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(palFull.u16);
					break;
				case VdpPalette::PALMODE_TMS9918:
					T_recalcFull_TMS9918<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(palFull.u16);
					break;
			}
			break;

		case MdFb::BPP_32:
		default:
			switch (this->palMode) {
				case VdpPalette::PALMODE_32X:
					T_recalcFull_32X<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(palFull32X.u32);
					// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
					// TODO: Add a separate dirty flag for the 32X palette?
				case VdpPalette::PALMODE_MD:
				default:
					T_recalcFull_MD<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(palFull.u32);
					break;
				case VdpPalette::PALMODE_SMS:
					T_recalcFull_SMS<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(palFull.u32);
					break;
				case VdpPalette::PALMODE_GG:
					T_recalcFull_GG<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(palFull.u32);
					break;
				case VdpPalette::PALMODE_TMS9918:
					T_recalcFull_TMS9918<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(palFull.u32);
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
	q->m_dirty.full = false;
	q->m_dirty.active = true;
}

}
