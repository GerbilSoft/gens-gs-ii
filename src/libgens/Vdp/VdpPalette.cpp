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

#ifdef _WIN32
#include <windows.h>
#endif /* _WIN32 */

namespace LibGens {

/** VdpPalettePrivate **/

VdpPalettePrivate::VdpPalettePrivate(VdpPalette *q)
	: q(q)
	, palMode(VdpPalette::PALMODE_MD)
	, bgColorIdx(0)
	, maskedBgColorIdx(0)
	, m5m4bits(0)
	, mdShadowHighlight(false)
	, isAppOs(false)
{
#ifdef _WIN32
	// Check for an app-based OS.
	// TODO: More comprehensive Win8 check.
	// (Use function from KERNEL32 directly?)
	OSVERSIONINFOA versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	BOOL ret = GetVersionExA(&versionInfo);
	if (ret) {
		if (versionInfo.dwMajorVersion > 6 ||
		    (versionInfo.dwMajorVersion == 6 &&
		     versionInfo.dwMinorVersion >= 2))
		{
			// App-based OS detected.
			// Use 16 colors to "fit in".
			isAppOs = true;
		}
	}
#endif /* _WIN32 */
}

/**
 * Apply masking to the background color index.
 */
void VdpPalettePrivate::applyBgColorIdxMask(void)
{
	uint8_t masked = 0;
	switch (palMode) {
		case VdpPalette::PALMODE_TMS9918A:
			masked = (bgColorIdx & 0xF);
			break;

		case VdpPalette::PALMODE_SMS:
		case VdpPalette::PALMODE_GG:
			// SMS/GG always uses the second palette
			// for the background color.
			masked = (bgColorIdx & 0xF);
			masked |= 0x10;
			break;

		case VdpPalette::PALMODE_MD:
		case VdpPalette::PALMODE_32X:
		default:
			// Mask value depends on m5m4bits.
			if (m5m4bits & 0x02) {
				// Mode 5. All 64 colors are usable.
				masked = (bgColorIdx & 0x3F);
			} else {
				// Mode 4. Only the second palette is usable.
				masked = (bgColorIdx & 0xF);
				masked |= 0x10;
			}
			break;
	}

	if (maskedBgColorIdx != masked) {
		// Masked background color index has changed.
		// TODO: Only update the background color?
		maskedBgColorIdx = masked;
		q->m_dirty.active = true;
	}
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
	, cram_addr_mask(0x7F)
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
 * @param palMode New palette mode.
 */
void VdpPalette::setPalMode(PalMode_t palMode)
{
	if (d->palMode == palMode)
		return;
	d->palMode = palMode;
	// TODO: May need other changes...

	// Both the full and active palettes must be recalculated.
	d->applyBgColorIdxMask();
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
 * @param bgColorIdx New background color index.
 */
void VdpPalette::setBgColorIdx(uint8_t bgColorIdx)
{
	if (d->bgColorIdx == bgColorIdx)
		return;
	d->bgColorIdx = bgColorIdx;
	d->applyBgColorIdxMask();
}

/**
 * Get the M5/M4 bits.
 * Used for Mega Drive and Master System modes.
 * @return M5/M4 bits. [ x  x  x  x  x  x M5 M4]
 */
PAL_PROPERTY_READ(uint8_t, m5m4bits)

/**
 * Set the M5/M4 bits.
 * Used for Mega Drive and Master System modes.
 * @param m5m4bits M5/M4 bits. [ x  x  x  x  x  x M5 M4]
 */
void VdpPalette::setM5M4bits(uint8_t m5m4bits)
{
	if (d->m5m4bits == m5m4bits)
		return;
	d->m5m4bits = m5m4bits;

	if (d->palMode == PALMODE_SMS || d->palMode == PALMODE_MD) {
		// Mode bits have changed.
		// Recalculate the active palette.
		// NOTE: Game Gear does not have a hard-coded TMS palette.
		d->applyBgColorIdxMask();
		m_dirty.active = true;
	}
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
