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

#ifndef __LIBGENS_MD_VDPPALETTE_HPP__
#define __LIBGENS_MD_VDPPALETTE_HPP__

#include <stdint.h>

#include "VdpTypes.hpp"

// ZOMG CRam struct.
#include "libzomg/zomg_vdp.h"

// Needed for FORCE_INLINE.
#include "../macros/common.h"

namespace LibGens
{

class VdpPalettePrivate;

class VdpPalette
{
	public:
		VdpPalette();
		~VdpPalette();
		
		/**
		 * reset(): Reset the palette, including CRam.
		 */
		void reset();
		
		// Active MD palette.
		// TODO: Make this private and add accessors.
		union ActivePalette_t
		{
			uint16_t u16[0x100];
			uint32_t u32[0x100];
		};
		ActivePalette_t m_palActive;
		
		/**
		 * isDirty(): Check if the palette is dirty.
		 * @return True if the palette is dirty.
		 */
		bool isDirty(void) const;
		
		/**
		 * setMode(): Set the palette mode.
		 * TODO: Implement mode settings. For now, this just forces a palette recalculation.
		 */
		void setMode(void) __attribute__ ((deprecated));
		
		/** CRam functions. **/
		
		/**
		 * readCRam_8(): Read 8-bit data from CRam.
		 * @param address CRam address.
		 * @return CRam data. (8-bit)
		 */
		uint8_t readCRam_8(uint8_t address) const;
		
		/**
		 * readCRam_16(): Read 16-bit data from CRam.
		 * @param address CRam address. (Must be 16-bit aligned!)
		 * @return CRam data. (16-bit)
		 * TODO: Endianness? Assuming host-endian for now.
		 */
		uint16_t readCRam_16(uint8_t address) const;
		
		/**
		 * writeCRam_8(): Write 8-bit data to CRam.
		 * @param address CRam address.
		 * @param data CRam data. (8-bit)
		 */
		void writeCRam_8(uint8_t address, uint8_t data);
		
		/**
		 * writeCRam_16(): Write 16-bit data to CRam.
		 * @param address CRam address. (Must be 16-bit aligned!)
		 * @param data CRam data. (16-bit)
		 * TODO: Endianness? Assuming host-endian for now.
		 */
		void writeCRam_16(uint8_t address, uint16_t data);
		
		/** Properties. **/
		
		int contrast(void) const;
		void setContrast(int newContrast);
		
		int brightness(void) const;
		void setBrightness(int newBrightness);
		
		bool grayscale(void) const;
		void setGrayscale(bool newGrayscale);
		
		bool inverted(void) const;
		void setInverted(bool newInverted);
		
		/**
		 * Color scale method.
		 * TODO: Possibly remove COLSCALE_FULL_SH, since it's incorrect.
		 * Normal MD(0xEEE) and highlighted MD(0xEEE) have the same brightness.
		 * This was tested by TmEE on hardware. (Genesis 2)
		 */
		enum ColorScaleMethod_t
		{
			COLSCALE_RAW = 0,	// Raw colors: 0xEEE -> 0xE0E0E0
			COLSCALE_FULL = 1,	// Full colors: 0xEEE -> 0xFFFFFF
			COLSCALE_FULL_SH = 2,	// Full colors with Shadow/Highlight: 0xEEE -> 0xEEEEEE for highlight
		};
		ColorScaleMethod_t colorScaleMethod(void) const;
		void setColorScaleMethod(ColorScaleMethod_t newColorScaleMethod);
		
		// Color depth.
		enum ColorDepth
		{
			// RGB color modes.
			BPP_15,
			BPP_16,
			BPP_32,
			
			BPP_MAX
		};
		ColorDepth bpp(void) const;
		void setBpp(ColorDepth newBpp);
		
		// Palette mode.
		enum PalMode_t
		{
			PALMODE_MD,
			PALMODE_32X,
			PALMODE_SMS,
			PALMODE_GG,
			PALMODE_TMS9918,
			
			PALMODE_MAX
		};
		PalMode_t palMode(void) const;
		void setPalMode(PalMode_t newPalMode);
		
		/**
		 * bgColorIdx: Background color index.
		 */
		int bgColorIdx(void) const;
		void setBgColorIdx(uint8_t newBgColorIdx);
		
		/** MD-specific properties. **/
		
		/**
		 * mdColorMask: Set if color masking is enabled. (Mode 5 only)
		 * False: Full color range is used.
		 * True: Only LSBs are used.
		 */
		bool mdColorMask(void) const;
		void setMdColorMask(bool newMdColorMask);
		
		/**
		 * mdShadowHighlight(): Set if the Shadow/Highlight functionality is enabled. (Mode 5 only)
		 * False: Shadow/Highlight is disabled.
		 * True: Shadow/Highlight is enabled.
		 */
		bool mdShadowHighlight(void) const;
		void setMdShadowHighlight(bool newMdShadowHighlight);
		
		/** SMS-specific functions. **/
		
		/**
		 * initSegaTMSPalette(): Initialize CRam with the SMS TMS9918 palette.
		 * Only used on Sega Master System!
		 * Palette mode must be set to PALMODE_SMS.
		 */
		void initSegaTMSPalette(void);
		
		/** Palette recalculation functions. **/
		void update(void);
		
		// TODO
		//static void Adjust_CRam_32X(void);
		
		/** ZOMG savestate functions. **/
		void zomgSaveCRam(Zomg_CRam_t *cram);
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
		
		/** Properties. **/
		int m_contrast;
		int m_brightness;
		bool m_grayscale;
		bool m_inverted;
		ColorScaleMethod_t m_colorScaleMethod;
		ColorDepth m_bpp;
		
		// Palette mode.
		PalMode_t m_palMode;
		
		// Background color index.
		uint8_t m_bgColorIdx;
		
		/**
		 * MD color mask. (Mode 5 only)
		 * Used with the Palette Select bit.
		 */
		uint16_t m_mdColorMask;
		static const uint16_t MD_COLOR_MASK_FULL;
		static const uint16_t MD_COLOR_MASK_LSB;
		
		/**
		 * Shadow/Highlight enable bit. (Mode 5 only)
		 */
		bool m_mdShadowHighlight;
		
		/**
		 * Dirty flags.
		 */
		union PalDirty_t
		{
			uint8_t data;
			struct
			{
				bool active	:1;
				bool full	:1;
			};
		};
		PalDirty_t m_dirty;
		
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
		FORCE_INLINE void T_update_TMS9918(pixel *TMS_palette,
					const pixel *palette);
};

/**
 * isDirty(): Check if the palette is dirty.
 * @return True if the palette is dirty.
 */
inline bool VdpPalette::isDirty(void) const
	{ return !!(m_dirty.data); }

/**
 * setMode(): Set the palette mode.
 * TODO: Implement mode settings. For now, this just forces a palette recalculation.
 */
inline void VdpPalette::setMode(void)
	{ m_dirty.full = true; }


/** CRam functions. **/

/**
 * readCRam_8(): Read 8-bit data from CRam.
 * @param address CRam address.
 * @return CRam data. (8-bit)
 */
inline uint8_t VdpPalette::readCRam_8(uint8_t address) const
{
	// TODO: Apply address masking based on system ID.
	return m_cram.u8[address & 0x7F];
}

/**
 * readCRam_16(): Read 16-bit data from CRam.
 * @param address CRam address. (Must be 16-bit aligned!)
 * @return CRam data. (16-bit)
 * TODO: Endianness? Assuming host-endian for now.
 */
inline uint16_t VdpPalette::readCRam_16(uint8_t address) const
{
	// TODO: Apply address masking based on system ID.
	return m_cram.u16[(address & 0x7F) >> 1];
}

/**
 * writeCRam_8(): Write 8-bit data to CRam.
 * @param address CRam address.
 * @param data CRam data. (8-bit)
 */
inline void VdpPalette::writeCRam_8(uint8_t address, uint8_t data)
{
	// TODO: Apply address masking based on system ID.
	m_cram.u8[address & 0x7F] = data;
	m_dirty.active = true;
}

/**
 * writeCRam_16(): Write 16-bit data to CRam.
 * @param address CRam address. (Must be 16-bit aligned!)
 * @param data CRam data. (16-bit)
 * TODO: Endianness? Assuming host-endian for now.
 */
inline void VdpPalette::writeCRam_16(uint8_t address, uint16_t data)
{
	// TODO: Apply address masking based on system ID.
	m_cram.u16[(address & 0x7F) >> 1] = data;
	m_dirty.active = true;
}


/** Properties. **/
inline int VdpPalette::contrast(void) const
	{ return m_contrast; }
inline int VdpPalette::brightness(void) const
	{ return m_brightness; }
inline bool VdpPalette::grayscale(void) const
	{ return m_grayscale; }
inline bool VdpPalette::inverted(void) const
	{ return m_inverted; }

inline VdpPalette::ColorScaleMethod_t VdpPalette::colorScaleMethod(void) const
	{ return m_colorScaleMethod; }

inline VdpPalette::ColorDepth VdpPalette::bpp(void) const
	{ return m_bpp; }

inline VdpPalette::PalMode_t VdpPalette::palMode(void) const
	{ return m_palMode; }
inline int VdpPalette::bgColorIdx(void) const
	{ return m_bgColorIdx; }
inline bool VdpPalette::mdColorMask(void) const
	{ return (m_mdColorMask == MD_COLOR_MASK_LSB); }

}

#endif /* __LIBGENS_MD_VDPPALETTE_HPP__ */
