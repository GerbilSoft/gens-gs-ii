/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpPalette.hpp: VDP palette handler.                                    *
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

namespace LibGens {

/**
 * VdpPalette private class.
 */
class VdpPalettePrivate
{
	public:
		VdpPalettePrivate(VdpPalette *q);

	private:
		VdpPalette *const q;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		VdpPalettePrivate(const VdpPalettePrivate &);
		VdpPalettePrivate &operator=(const VdpPalettePrivate &);

	public:
		/** Properties. **/
		VdpPalette::PalMode_t palMode;	// Palette mode.
		uint8_t bgColorIdx;		// Background color index.

		/**
		 * MD color mask. (Mode 5 only)
		 * Used with the Palette Select bit.
		 */
		uint16_t mdColorMask;
		static const uint16_t MD_COLOR_MASK_FULL;
		static const uint16_t MD_COLOR_MASK_LSB;

		/**
		 * Shadow/Highlight enable bit. (Mode 5 only)
		 */
		bool mdShadowHighlight;

		// Full MD/SMS/GG palette.
		union {
			uint16_t u16[0x1000];
			uint32_t u32[0x1000];
		} palFull;

		// Full 32X palette.
		// TODO: Only allocate this if 32X mode is enabled?
		union {
			uint16_t u16[0x10000];
			uint32_t u32[0x10000];
		} palFull32X;

		/** Full palette recalculation functions. **/

		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFull_MD(pixel *palFull);

		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFull_32X(pixel *palFull32X);

		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFull_SMS(pixel *palFull);

		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFull_GG(pixel *palFull);

		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFull_TMS9918(pixel *palFull);

		void recalcFull(void);

	public:
		/** Static functions. **/
		static int FUNC_PURE ClampColorComponent(int mask, int c);
};

// TODO: Maybe just make these #define's instead of class variables?
const uint16_t VdpPalettePrivate::MD_COLOR_MASK_FULL = 0xEEE;
const uint16_t VdpPalettePrivate::MD_COLOR_MASK_LSB = 0x222;

/**
 * VdpPalette private class.
 */
VdpPalettePrivate::VdpPalettePrivate(VdpPalette *q)
	: q(q)
	, palMode(VdpPalette::PALMODE_MD)
	, bgColorIdx(0x00)
	, mdColorMask(MD_COLOR_MASK_FULL)
	, mdShadowHighlight(false)
{ }

/** VdpPalettePrivate: Full palette recalculation functions. **/

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

/** VdpPalettePrivate: Static functions. **/

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

/** VdpPalette class. **/

/**
 * VdpPalette(): Initialize a new VdpPalette object.
 * 
 * NOTE: bpp is initialized to 32 for now because
 * radeong_dri.so is really slow with 16-bit color.
 * TODO: Check with recent Gallium3D updates!
 */
VdpPalette::VdpPalette()
	: d(new VdpPalettePrivate(this))
	, m_bpp(MdFb::BPP_32)
{
	// Set the dirty flags.
	m_dirty.active = true;
	m_dirty.full = true;

	// Reset CRam and other palette variables.
	reset();
}


VdpPalette::~VdpPalette()
{
	delete d;

	// TODO: Other cleanup.
}

/**
 * Reset the palette, including CRam.
 */
void VdpPalette::reset(void)
{
	// Clear CRam.
	memset(&m_cram, 0x00, sizeof(m_cram));

	// Mark the active palette as dirty.
	m_dirty.active = true;
}


/**
 * PAL_PROPERTY_READ(): Property read function macro.
 */
#define PAL_PROPERTY_READ(propType, propName) \
propType VdpPalette::propName(void) const \
	{ return d->propName; }

/**
 * PAL_PROPERTY_WRITE(): Property write function macro.
 */
#define PAL_PROPERTY_WRITE(propName, setPropType, setPropName) \
void VdpPalette::set##setPropName(setPropType new##setPropName) \
{ \
	if (d->propName == (new##setPropName)) \
		return; \
	d->propName = (new##setPropName); \
	m_dirty.full = true; \
}

/**
 * PAL_PROPERTY_WRITE_NOPRIV(): Property write function macro.
 * Used for properties that haven't been moved to VdpPalettePrivate yet.
 */
#define PAL_PROPERTY_WRITE_NOPRIV(propName, setPropType, setPropName) \
void VdpPalette::set##setPropName(setPropType new##setPropName) \
{ \
	if (m_##propName == (new##setPropName)) \
		return; \
	m_##propName = (new##setPropName); \
	m_dirty.full = true; \
}

/** Property write functions. **/
PAL_PROPERTY_WRITE_NOPRIV(bpp, MdFb::ColorDepth, Bpp)

/**
 * Get the palette mode.
 * @return Palette mode.
 */
PAL_PROPERTY_READ(VdpPalette::PalMode_t, palMode);

/**
 * Set the palette mode.
 * @param newPalMode New palette mode.
 */
void VdpPalette::setPalMode(PalMode_t newPalMode)
{
	if (d->palMode == newPalMode)
		return;
	d->palMode = newPalMode;
	
	// Both the full and active palettes must be recalculated.
	m_dirty.full = true;
	m_dirty.active = true;
}


/**
 * Get the background color index.
 * @return Background color index.
 */
PAL_PROPERTY_READ(uint8_t, bgColorIdx)

/**
 * Set the background color index.
 * @param newBgColorIdx New background color index.
 */
void VdpPalette::setBgColorIdx(uint8_t newBgColorIdx)
{
	if (d->bgColorIdx == newBgColorIdx)
		return;
	
	// Mega Drive:
	d->bgColorIdx = (newBgColorIdx & 0x3F);
	
#if 0
	// TODO: On SMS and Game Gear:
	d->bgColorIdx &= 0x0F;
	d->bgColorIdx |= 0x10;
#endif
	
	// TODO: Store the original BG color index as well as the masked version?
	// (and re-mask the original index on mode change)
	
	// Mark the active palette as dirty.
	m_dirty.active = true;
}


/**
 * Get the MD color mask. (Mode 5 only)
 * @return True if all but LSBs are masked; false for normal operation.
 */
bool VdpPalette::mdColorMask(void) const
	{ return (d->mdColorMask == VdpPalettePrivate::MD_COLOR_MASK_LSB); }

/**
 * Set the MD color mask. (Mode 5 only)
 * @param newMdColorMask If true, masks all but LSBs.
 */
void VdpPalette::setMdColorMask(bool newMdColorMask)
{
	const uint16_t newMask = (newMdColorMask
				? VdpPalettePrivate::MD_COLOR_MASK_LSB
				: VdpPalettePrivate::MD_COLOR_MASK_FULL);
	
	if (d->mdColorMask == newMask)
		return;
	d->mdColorMask = newMask;
	
	// Mark both full and active palettes as dirty.
	m_dirty.full = true;
	m_dirty.active = true;
}


/**
 * Get the MD Shadow/Highlight bit. (Mode 5 only)
 * @return True if shadow/highlight is enabled; false otherwise.
 */
PAL_PROPERTY_READ(bool, mdShadowHighlight)

/**
 * Set the MD Shadow/Highlight bit. (Mode 5 only)
 * @param newMdShadowHighlight If true, enables shadow/highlight.
 */
void VdpPalette::setMdShadowHighlight(bool newMdShadowHighlight)
{
	if (d->mdShadowHighlight == newMdShadowHighlight)
		return;
	
	d->mdShadowHighlight = newMdShadowHighlight;
	if (d->mdShadowHighlight) {
		// Shadow/Highlight was just enabled.
		// Active palette needs to be recalculated
		// in order to populate the S/H entries.
		m_dirty.active = true;
	}
}


/** SMS-specific functions. **/


/**
 * Initialize CRam with the SMS TMS9918 palette.
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


/** Active palette recalculation functions. **/


/**
 * Recalculate the active palette. (Mega Drive, Mode 5)
 * @param MD_palette MD color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_MD(pixel *MD_palette,
					const pixel *palette)
{
	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2) {
		const uint16_t color1_raw = (m_cram.u16[i] & d->mdColorMask);
		const uint16_t color2_raw = (m_cram.u16[i + 1] & d->mdColorMask);

		// Get the palette color.
		pixel color1 = palette[color1_raw];
		pixel color2 = palette[color2_raw];

		// Set the new color.
		MD_palette[i]     = color1;
		MD_palette[i + 1] = color2;
	}

	// Update the background color.
	MD_palette[0] = MD_palette[d->bgColorIdx];

	if (d->mdShadowHighlight) {
		// Update the shadow and highlight colors.
		// References:
		// - http://www.tehskeen.com/forums/showpost.php?p=71308&postcount=1077
		// - http://forums.sonicretro.org/index.php?showtopic=17905
		
		// Shadow (64-127) and highlight (128-191) palettes.
		for (int i = 62; i >= 0; i -= 2) {
			uint16_t color1_raw = ((m_cram.u16[i] & d->mdColorMask) >> 1);
			uint16_t color2_raw = ((m_cram.u16[i + 1] & d->mdColorMask) >> 1);

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
		MD_palette[64]  = MD_palette[d->bgColorIdx + 64];	// Shadow color.
		MD_palette[128] = MD_palette[d->bgColorIdx + 128];	// Highlight color.
	}
}


/**
 * Recalculate the active palette. (Sega Master System, Mode 4)
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

	for (int i = color_start; i >= 0; i -= 2) {
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
	SMS_palette[0] = SMS_palette[d->bgColorIdx];
}


/**
 * Recalculate the active palette. (Sega Game Gear, Mode 4 [12-bit RGB])
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

	for (int i = color_start; i >= 0; i -= 2) {
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
	GG_palette[0] = GG_palette[d->bgColorIdx];
}


/**
 * Recalculate the active palette. (TMS9918)
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
	//TMS_palette[0] = TMS_palette[d->bgColorIdx];
}


// TODO: Port to LibGens.
#if 0
/**
 * Adjust the 32X CRam.
 */
template<typename pixel>
static FORCE_INLINE void T_Adjust_CRam_32X(pixel *pal32X, pixel *cramAdjusted32X)
{
	for (int i = 0; i < 0x100; i += 4) {
		cramAdjusted32X[i] = pal32X[_32X_VDP_CRam[i]];
		cramAdjusted32X[i+1] = pal32X[_32X_VDP_CRam[i+1]];
		cramAdjusted32X[i+2] = pal32X[_32X_VDP_CRam[i+2]];
		cramAdjusted32X[i+3] = pal32X[_32X_VDP_CRam[i+3]];
	}
}
#endif


/**
 * Update the active palette.
 */
void VdpPalette::update(void)
{
	if (m_dirty.full)
		d->recalcFull();
	if (!m_dirty.active)
		return;

	if (m_bpp != MdFb::BPP_32) {
		switch (d->palMode) {
			case PALMODE_32X:
				// TODO: Implement T_update_32X().
#if 0
				T_update_32X<uint16_t>(m_palActive32X.u16, m_palFull32X.u16);
#endif
				// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
				// TODO: Add a separate dirty flag for the 32X palette?

			case PALMODE_MD:
			default:
				T_update_MD<uint16_t>(m_palActive.u16, d->palFull.u16);
				break;

			case PALMODE_SMS:
				T_update_SMS<uint16_t>(m_palActive.u16, d->palFull.u16);
				break;

			case PALMODE_GG:
				T_update_GG<uint16_t>(m_palActive.u16, d->palFull.u16);
				break;

			case PALMODE_TMS9918:
				T_update_TMS9918<uint16_t>(m_palActive.u16, d->palFull.u16);
				break;
		}
	} else {
		switch (d->palMode) {
			case PALMODE_32X:
				// TODO: Implement T_update_32X().
#if 0
				T_update_32X<uint32_t>(m_palActive32X.u32, m_palFull32X.u32);
#endif
				// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
				// TODO: Add a separate dirty flag for the 32X palette?

			case PALMODE_MD:
			default:
				T_update_MD<uint32_t>(m_palActive.u32, d->palFull.u32);
				break;

			case PALMODE_SMS:
				T_update_SMS<uint32_t>(m_palActive.u32, d->palFull.u32);
				break;

			case PALMODE_GG:
				T_update_GG<uint32_t>(m_palActive.u32, d->palFull.u32);
				break;

			case PALMODE_TMS9918:
				T_update_TMS9918<uint32_t>(m_palActive.u32, d->palFull.u32);
				break;
		}
	}

	// Clear the active palette dirty bit.
	m_dirty.active = false;
}

// TODO: Port to LibGens: T_update_32X()
#if 0
/**
 * Adjust the 32X CRam.
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
 * Save the CRam.
 * @param state Zomg_CRam_t struct to save to.
 */
void VdpPalette::zomgSaveCRam(Zomg_CRam_t *cram) const
{
	// TODO: Support systems other than MD.
	memcpy(cram->md, m_cram.u16, sizeof(cram->md));
}

/**
 * Load the CRam.
 * @param state Zomg_CRam_t struct to save to.
 */
void VdpPalette::zomgRestoreCRam(const Zomg_CRam_t *cram)
{
	// TODO: Support systems other than MD.
	memcpy(m_cram.u16, cram->md, sizeof(cram->md));
	
	// Mark the active palette as dirty.
	m_dirty.active = true;
}

}
