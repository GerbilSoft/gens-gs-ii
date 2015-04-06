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

#ifndef __LIBGENS_MD_VDPPALETTE_HPP__
#define __LIBGENS_MD_VDPPALETTE_HPP__

#include <stdint.h>

#include "VdpTypes.hpp"
// ColorDepth is in MdFb.
#include "../Util/MdFb.hpp"

// ZOMG CRam struct.
#include "libzomg/zomg_vdp.h"

#include "../macros/common.h"
#include "../Util/byteswap.h"

namespace LibGens {

class VdpPalettePrivate;
class VdpPalette
{
	public:
		VdpPalette();
		~VdpPalette();

		/**
		 * Reset the palette, including CRam.
		 */
		void reset();

		// Active MD palette.
		// TODO: Make this private and add accessors.
		union {
			uint16_t u16[0x100];
			uint32_t u32[0x100];
		} m_palActive;

		/**
		 * Check if the palette is dirty.
		 * @return True if the palette is dirty.
		 */
		bool isDirty(void) const;

		/** CRam functions. **/

		/**
		 * Read 8-bit data from CRam.
		 * @param address CRam address.
		 * @return CRam data. (8-bit)
		 * TODO: Endianness? Assuming host-endian for now.
		 */
		uint8_t readCRam_8(uint8_t address) const;

		/**
		 * Read 16-bit data from CRam.
		 * @param address CRam address. (Must be 16-bit aligned!)
		 * @return CRam data. (16-bit)
		 * TODO: Endianness? Assuming host-endian for now.
		 */
		uint16_t readCRam_16(uint8_t address) const;

		/**
		 * Write 8-bit data to CRam.
		 * @param address CRam address.
		 * @param data CRam data. (8-bit)
		 * TODO: Endianness? Assuming host-endian for now.
		 */
		void writeCRam_8(uint8_t address, uint8_t data);

		/**
		 * Write 16-bit data to CRam.
		 * @param address CRam address. (Must be 16-bit aligned!)
		 * @param data CRam data. (16-bit)
		 * TODO: Endianness? Assuming host-endian for now.
		 */
		void writeCRam_16(uint8_t address, uint16_t data);

		/** Properties. **/

		// Color depth.
		MdFb::ColorDepth bpp(void) const;
		void setBpp(MdFb::ColorDepth newBpp);

		// Palette mode.
		enum PalMode_t {
			// TMS9918A: Hard-coded 16-color palette.
			PALMODE_TMS9918A,

			// Sega Master System: 6-bit CRAM.
			// Uses a hard-coded color ROM when in TMS9918A modes.
			PALMODE_SMS,

			// Sega Game Gear: 12-bit CRAM.
			PALMODE_GG,

			// Sega Mega Drive: 9-bit CRAM.
			// Usage depends on M5/M4 bits:
			// - M5=1,M4=1: 3-bit DAC
			// - M5=1,M4=0: 3-bit DAC, LSB only
			// - M5=0,M4=1: 2-bit DAC, high 2 bits only
			// - M5=0,M4=0: Black screen.
			PALMODE_MD,

			// Sega 32X: MD + 15-bit RGB
			PALMODE_32X,

			PALMODE_MAX
		};
		PalMode_t palMode(void) const;
		void setPalMode(PalMode_t palMode);

		/**
		 * Background color index.
		 */
		uint8_t bgColorIdx(void) const;
		void setBgColorIdx(uint8_t bgColorIdx);

		/**
		 * Set the M5/M4 bits.
		 * Used for Mega Drive and Master System modes.
		 * @param m5m4bits M5/M4 bits. [ x  x  x  x  x  x M5 M4]
		 */
		uint8_t m5m4bits(void) const;
		void setM5M4bits(uint8_t m5m4bits);

		/** MD-specific properties. **/

		/**
		 * Set if the Shadow/Highlight functionality is enabled. (Mode 5 only)
		 * False: Shadow/Highlight is disabled.
		 * True: Shadow/Highlight is enabled.
		 */
		bool mdShadowHighlight(void) const;
		void setMdShadowHighlight(bool newMdShadowHighlight);

		/** SMS-specific functions. **/

		/**
		 * Initialize CRam with the SMS TMS9918A palette.
		 * Only used on Sega Master System!
		 * Palette mode must be set to PALMODE_SMS.
		 */
		void initSegaTMSPalette(void);

		/**
		 * Update the active palette.
		 */
		void update(void);

		/** ZOMG savestate functions. **/
		void zomgSaveCRam(Zomg_CRam_t *cram) const;
		void zomgRestoreCRam(const Zomg_CRam_t *cram);

	private:
		friend class VdpPalettePrivate;
		VdpPalettePrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		VdpPalette(const VdpPalette &);
		VdpPalette &operator=(const VdpPalette &);

		// Color RAM.
		VdpTypes::CRam_t m_cram;
		uint8_t cram_addr_mask;

		// Color depth.
		MdFb::ColorDepth m_bpp;

		/**
		 * Dirty flags.
		 */
		union {
			uint8_t data;
			struct {
				bool active	:1;
				bool full	:1;
				// TODO: Add a separate bit for 32X CRAM.
			};
		} m_dirty;

		/** Active palette recalculation functions. **/

		template<typename pixel>
		FORCE_INLINE void T_update_MD(pixel *MD_palette,
					const pixel *palette);

		template<typename pixel>
		FORCE_INLINE void T_update_SMS(pixel *SMS_palette,
					 const pixel *palette);

		template<typename pixel>
		FORCE_INLINE void T_update_GG(pixel *GG_palette,
					const pixel *palette);

		template<typename pixel>
		FORCE_INLINE void T_update_TMS9918A(pixel *TMS_palette,
					     const pixel *palette);

		// TODO
		//static void Adjust_CRam_32X(void);
};

/**
 * Check if the palette is dirty.
 * @return True if the palette is dirty.
 */
inline bool VdpPalette::isDirty(void) const
	{ return !!(m_dirty.data); }

/** CRam functions. **/

/**
 * Read 8-bit data from CRam.
 * @param address CRam address.
 * @return CRam data. (8-bit)
 * TODO: Endianness? Assuming host-endian for now.
 */
inline uint8_t VdpPalette::readCRam_8(uint8_t address) const
{
	address &= cram_addr_mask;
	return m_cram.u8[address ^ U16DATA_U8_INVERT];
}

/**
 * Read 16-bit data from CRam.
 * @param address CRam address. (Must be 16-bit aligned!)
 * @return CRam data. (16-bit)
 * TODO: Endianness? Assuming host-endian for now.
 */
inline uint16_t VdpPalette::readCRam_16(uint8_t address) const
{
	address &= cram_addr_mask;
	return m_cram.u16[address >> 1];
}

/**
 * Write 8-bit data to CRam.
 * @param address CRam address.
 * @param data CRam data. (8-bit)
 * TODO: Endianness? Assuming host-endian for now.
 */
inline void VdpPalette::writeCRam_8(uint8_t address, uint8_t data)
{
	address &= cram_addr_mask;
	m_cram.u8[address ^ U16DATA_U8_INVERT] = data;
	// TODO: Per-color dirty flag?
	m_dirty.active = true;
}

/**
 * Write 16-bit data to CRam.
 * @param address CRam address. (Must be 16-bit aligned!)
 * @param data CRam data. (16-bit)
 * TODO: Endianness? Assuming host-endian for now.
 */
inline void VdpPalette::writeCRam_16(uint8_t address, uint16_t data)
{
	address &= cram_addr_mask;
	m_cram.u16[address >> 1] = data;
	// TODO: Per-color dirty flag?
	m_dirty.active = true;
}

/** Properties. **/
inline MdFb::ColorDepth VdpPalette::bpp(void) const
	{ return m_bpp; }

}

#endif /* __LIBGENS_MD_VDPPALETTE_HPP__ */
