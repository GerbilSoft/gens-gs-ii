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
 * Recalculate the active palette. (Mega Drive, Mode 4)
 * CRAM is in MD format; however, only the top two bits are relevant.
 * @param palActiveMD Active MD palette. (Must have 0x40 entries!)
 * @param palFullSMS Full SMS palette. (Must have 0x40 entries!)
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_MD_M4(pixel *palActiveMD,
				       const pixel *palFullSMS)
{
#if !defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	// Update all 32 colors.
	static const int color_start = (32 - 2);
#else
	// Process all 64 colors for lulz.
	static const int color_start = (64 - 2);
#endif

	// CRAM format for MD, Mode 4: (TODO: Needs verification.)
	// ---- -BB- -GG- -RR-
	for (int i = color_start; i >= 0; i -= 2) {
		// TODO: Use alternating bytes in SMS CRAM for MD compatibility?
		uint16_t color1_raw = m_cram.u16[i];
		uint16_t color2_raw = m_cram.u16[i + 1];

		// Shift the colors into SMS-compatible values.
		// TODO: Create a lookup table? (Or don't - this mode is rarely used.)
		color1_raw = ((color1_raw & 0x0600) >> 5) |
			     ((color1_raw & 0x0060) >> 3) |
			     ((color1_raw & 0x0006) >> 1);
		color2_raw = ((color2_raw & 0x0600) >> 5) |
			     ((color2_raw & 0x0060) >> 3) |
			     ((color2_raw & 0x0006) >> 1);

		// Get the palette color.
		pixel color1 = palFullSMS[color1_raw];
		pixel color2 = palFullSMS[color2_raw];

		// Set the new color.
		palActiveMD[i]     = color1;
		palActiveMD[i + 1] = color2;
	}

	// Update the background color.
	palActiveMD[0] = palActiveMD[d->maskedBgColorIdx];
}

/**
 * Recalculate the active palette. (Mega Drive, Mode 5)
 * @param palActiveMD Active MD palette. (Must have 0x40 entries!)
 * @param palFullMD Full MD palette. (Must have 0x1000 entries!)
 * @param palFullSMS Full SMS palette. (Must have 0x40 entries!)
 * TODO: Figure out a way to get rid of palFullSMS.
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_MD(pixel *palActiveMD,
				    const pixel *palFullMD,
				    const pixel *palFullSMS)
{
	uint16_t mdColorMask;
	switch (d->m5m4bits & 0x03) {
		case 0:
			// M5=0, M4=0
			// Blank screen.
			memset(palActiveMD, 0, (sizeof(pixel) * 64));
			return;
		case 1:
			// M5=0, M4=1
			// Mode 4: CRAM only has two significant bits.
			T_update_MD_M4(palActiveMD, palFullSMS);
			return;
		case 2:
			// M5=1, M4=0
			// Mode 5, PSEL=0: CRAM masks all but the LSB.
			mdColorMask = 0x222;
			break;
		case 3:
			// M5=1, M4=1
			// Mode 5, PSEL=1: Normal operation.
			mdColorMask = 0xEEE;
			break;
	}

	// Update all 64 colors.
	for (int i = 62; i >= 0; i -= 2) {
		const uint16_t color1_raw = (m_cram.u16[i] & mdColorMask);
		const uint16_t color2_raw = (m_cram.u16[i + 1] & mdColorMask);

		// Get the palette color.
		pixel color1 = palFullMD[color1_raw];
		pixel color2 = palFullMD[color2_raw];

		// Set the new color.
		palActiveMD[i]     = color1;
		palActiveMD[i + 1] = color2;
	}

	// Update the background color.
	palActiveMD[0] = palActiveMD[d->maskedBgColorIdx];

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
			palActiveMD[i + 64]      = palFullMD[color1_raw];
			palActiveMD[i + 1 + 64]  = palFullMD[color2_raw];

			// Highlight color. (1xxx - 0001)
			palActiveMD[i + 128]     = palFullMD[(0x888 | color1_raw) - 0x111];
			palActiveMD[i + 1 + 128] = palFullMD[(0x888 | color2_raw) - 0x111];
		}

		// Copy the normal colors (0-63) to shadow+highlight (192-255).
		// Pixels with both shadow and highlight show up as normal.
		memcpy(&palActiveMD[192], &palActiveMD[0], (sizeof(palActiveMD[0]) * 64));

		// Update the background color for the shadow and highlight palettes.
		palActiveMD[64]  = palActiveMD[d->maskedBgColorIdx + 64];	// Shadow color.
		palActiveMD[128] = palActiveMD[d->maskedBgColorIdx + 128];	// Highlight color.
	}
}

/**
 * Recalculate the active palette. (Sega Master System, Mode 4)
 * TODO: UNTESTED!
 * @param palActiveSMS Active SMS palette. (Must have 0x20 entries!)
 * @param palFullSMS Full SMS palette. (Must have 0x40 entries!)
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_SMS(pixel *palActiveSMS,
				     const pixel *palFullSMS)
{
#if !defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	// Update all 32 colors.
	static const int color_start = (32 - 2);
#else
	// Process all 64 colors for lulz.
	static const int color_start = (64 - 2);
#endif

	/**
	 * TMS9918A palette as used on the SMS1. (6-bit RGB)
	 * Used in VDP modes 0-3.
	 * NOTE: Two copies are included for compatibility.
	 * Source: http://www.smspower.org/maxim/forumstuff/colours.html
	 * Reference: http://www.smspower.org/forums/8224-TMS9918ColorsForSMSVDP
	 */
	static const uint8_t PalTMS9918A_SMS[] = {
		// First palette line.
		// TODO for commit: Color index 0xA is actually 0x05 from the forum post.
		// colours.html also has 0x05 as the color, but incorrectly says 0x04.
		0x00, 0x00, 0x08, 0x0C, 0x10, 0x30, 0x01, 0x3C,
		0x02, 0x03, 0x05, 0x0F, 0x04, 0x33, 0x15, 0x3F,
		// Second palette line.
		0x00, 0x00, 0x08, 0x0C, 0x10, 0x30, 0x01, 0x3C,
		0x02, 0x03, 0x05, 0x0F, 0x04, 0x33, 0x15, 0x3F,
#if defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
		// Third palette line.
		0x00, 0x00, 0x08, 0x0C, 0x10, 0x30, 0x01, 0x3C,
		0x02, 0x03, 0x05, 0x0F, 0x04, 0x33, 0x15, 0x3F,
		// Fourth palette line.
		0x00, 0x00, 0x08, 0x0C, 0x10, 0x30, 0x01, 0x3C,
		0x02, 0x03, 0x05, 0x0F, 0x04, 0x33, 0x15, 0x3F,
#endif
	};

	const uint8_t *cram;
	if (d->m5m4bits & 0x01) {
		// M4 is set. Use SMS CRAM.
		cram = &m_cram.u8[0];
	} else {
		// M4 is not set. Use TMS9918A-equivalent CROM.
		cram = &PalTMS9918A_SMS[0];
	}

	for (int i = color_start; i >= 0; i -= 2) {
		// TODO: Use alternating bytes in SMS CRam for MD compatibility?
		const uint8_t color1_raw = (cram[i] & 0x3F);
		const uint8_t color2_raw = (cram[i + 1] & 0x3F);

		// Get the palette color.
		pixel color1 = palFullSMS[color1_raw];
		pixel color2 = palFullSMS[color2_raw];

		// Set the new color.
		palActiveSMS[i]     = color1;
		palActiveSMS[i + 1] = color2;
	}

	// Update the background color.
	palActiveSMS[0] = palActiveSMS[d->maskedBgColorIdx];
}

/**
 * Recalculate the active palette. (Sega Game Gear, Mode 4 [12-bit RGB])
 * TODO: UNTESTED!
 * @param palActiveGG Active GG palette. (Must have 0x20 entries!)
 * @param palFullGG Full GG palette. (Must have 0x1000 entries!)
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_GG(pixel *palActiveGG,
				    const pixel *palFullGG)
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
		pixel color1 = palFullGG[color1_raw];
		pixel color2 = palFullGG[color2_raw];

		// Set the new color.
		palActiveGG[i]     = color1;
		palActiveGG[i + 1] = color2;
	}

	// Update the background color.
	palActiveGG[0] = palActiveGG[d->maskedBgColorIdx];
}

/**
 * Recalculate the active palette. (TMS9918A)
 * TODO: UNTESTED!
 * @param palActiveTMS Active TMS9918A palette. (Must have 0x20 entries!)
 * @param palFullTMS Full TMS9918A palette. (Must have 0x10 entries!)
 */
template<typename pixel>
FORCE_INLINE void VdpPalette::T_update_TMS9918A(pixel *palActiveTMS,
					  const pixel *palFullTMS)
{
	/**
	 * NOTE: This function doesn't actually recalculate palettes.
	 * It simply copies the full 16-color palette to the active palette twice.
	 * The palette is copied twice for compatibility purposes.
	 */

	// Copy the colors.
	memcpy(&palActiveTMS[0x00], &palFullTMS[0x00], (sizeof(palActiveTMS[0]) * 16));
	memcpy(&palActiveTMS[0x10], &palFullTMS[0x00], (sizeof(palActiveTMS[0]) * 16));
#if defined(DO_FOUR_PALETTE_LINES_IN_ALL_MODES_FOR_LULZ)
	memcpy(&palActiveTMS[0x20], &palFullTMS[0x00], (sizeof(palActiveTMS[0]) * 16));
	memcpy(&palActiveTMS[0x30], &palFullTMS[0x00], (sizeof(palActiveTMS[0]) * 16));
#endif

	// Update the background color.
	// TODO: Verify this.
	palActiveTMS[0] = palActiveTMS[d->maskedBgColorIdx];
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

	// TODO: Add an AND to each switch() for optimization?
	if (m_bpp != MdFb::BPP_32) {
		switch (d->palMode) {
			case PALMODE_32X:
				// TODO: Implement T_update_32X().
				//T_update_32X<uint16_t>(m_palActive32X.u16, m_palFull32X.u16);
				// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
				// TODO: Add a separate dirty flag for the 32X palette?
				// FALLTHROUGH

			case PALMODE_MD:
			default:
				T_update_MD<uint16_t>(m_palActive.u16, d->palFullMD.u16, d->palFullSMS.u16);
				break;

			case PALMODE_SMS:
				T_update_SMS<uint16_t>(m_palActive.u16, d->palFullSMS.u16);
				break;

			case PALMODE_GG:
				T_update_GG<uint16_t>(m_palActive.u16, d->palFullMD.u16);
				break;

			case PALMODE_TMS9918A:
				T_update_TMS9918A<uint16_t>(m_palActive.u16, d->palFullSMS.u16);
				break;
		}
	} else {
		switch (d->palMode) {
			case PALMODE_32X:
				// TODO: Implement T_update_32X().
				//T_update_32X<uint32_t>(m_palActive32X.u32, m_palFull32X.u32);
				// NOTE: 32X falls through to MD, since both 32X and MD palettes must be updated.
				// TODO: Add a separate dirty flag for the 32X palette?
				// FALLTHROUGH

			case PALMODE_MD:
			default:
				T_update_MD<uint32_t>(m_palActive.u32, d->palFullMD.u32, d->palFullSMS.u32);
				break;

			case PALMODE_SMS:
				T_update_SMS<uint32_t>(m_palActive.u32, d->palFullSMS.u32);
				break;

			case PALMODE_GG:
				T_update_GG<uint32_t>(m_palActive.u32, d->palFullMD.u32);
				break;

			case PALMODE_TMS9918A:
				T_update_TMS9918A<uint32_t>(m_palActive.u32, d->palFullSMS.u32);
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
