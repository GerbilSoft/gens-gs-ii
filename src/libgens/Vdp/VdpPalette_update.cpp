/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpPalette_update.hpp: VDP palette handler: Update functions.           *
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
#include "VdpPalette_p.hpp"

// FOR TESTING ONLY: Uncomment this #define to enable support for 4 palette lines in all modes.
// This should not be enabled in release builds!
//#define DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ

namespace LibGens {

/**
 * Recalculate the active palette. (Mega Drive, Mode 5)
 * @param MD_palette MD color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_MD(pixel *MD_palette,
					const pixel *palette)
{
	// Check the M4 bit to determine the color mask.
	const uint16_t mdColorMask = (d->m5m4bits & 1 ? 0xEEE : 0x222);

	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2) {
		const uint16_t color1_raw = (m_cram.u16[i] & mdColorMask);
		const uint16_t color2_raw = (m_cram.u16[i + 1] & mdColorMask);

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
			uint16_t color1_raw = ((m_cram.u16[i] & mdColorMask) >> 1);
			uint16_t color2_raw = ((m_cram.u16[i + 1] & mdColorMask) >> 1);

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
 * Recalculate the active palette. (TMS9918A)
 * TODO: UNTESTED!
 * @param GG_palette Game Gear color palette.
 * @param palette Full color palette.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_TMS9918A(pixel *TMS_palette,
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
	// TODO: How is the background color handled in TMS9918A modes?
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

			case PALMODE_TMS9918A:
				T_update_TMS9918A<uint16_t>(m_palActive.u16, d->palFull.u16);
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

			case PALMODE_TMS9918A:
				T_update_TMS9918A<uint32_t>(m_palActive.u32, d->palFull.u32);
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

}
