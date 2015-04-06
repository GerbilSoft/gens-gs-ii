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
#include "VdpPalette_p.hpp"

namespace LibGens {

/** VdpPalettePrivate **/

VdpPalettePrivate::VdpPalettePrivate(VdpPalette *q)
	: q(q)
	, palMode(VdpPalette::PALMODE_MD)
	, bgColorIdx(0x00)
	, mdColorMask(MD_COLOR_MASK_FULL)
	, mdShadowHighlight(false)
{ }

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
 * Initialize a new VdpPalette object.
 * 
 * NOTE: bpp is initialized to 32 for now because
 * radeong_dri.so is really slow with 16-bit color.
 * TODO: Check with recent Gallium3D updates!
 */
VdpPalette::VdpPalette()
	: d(new VdpPalettePrivate(this))
	, m_bpp(MdFb::BPP_32)
	, cram_addr_mask(0x7F)
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
