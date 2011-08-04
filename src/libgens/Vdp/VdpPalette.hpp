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

#ifndef __LIBGENS_MD_VDPPALETTE_HPP__
#define __LIBGENS_MD_VDPPALETTE_HPP__

#include <stdint.h>

#include "VdpTypes.hpp"

// Needed for FORCE_INLINE.
#include "../macros/common.h"

namespace LibGens
{

class VdpPalette
{
	public:
		VdpPalette();
		~VdpPalette();
		
		// Full MD palette.
		union Palette_t
		{
			uint16_t u16[0x1000];
			uint32_t u32[0x1000];
		};
		Palette_t m_palette;
		
		// Active MD palette.
		union MD_Palette_t
		{
			uint16_t u16[0x100];
			uint32_t u32[0x100];
		};
		MD_Palette_t m_palActiveMD;
		
		// Color scale method.
		// TODO: Possibly remove COLSCALE_FULL_HS, since it's incorrect.
		// Normal MD(0xEEE) and highlighted MD(0xEEE) have the same brightness.
		// This was tested by TmEE on hardware. (Genesis 2)
		enum ColorScaleMethod_t
		{
			COLSCALE_RAW = 0,	// Raw colors: 0xEEE -> 0xE0E0E0
			COLSCALE_FULL = 1,	// Full colors: 0xEEE -> 0xFFFFFF
			COLSCALE_FULL_HS = 2,	// Full colors with Highlight/Shadow: 0xEEE -> 0xEEEEEE for highlight
		};
		
		// Color depth.
		enum ColorDepth
		{
			BPP_15,
			BPP_16,
			BPP_32,
			
			BPP_MAX
		};
		
		inline bool isDirty(void) const
			{ return m_dirty; }
		
		/** Properties. **/
		
		inline int contrast(void) const
			{ return m_contrast; }
		void setContrast(int newContrast);
		
		inline int brightness(void) const
			{ return m_brightness; }
		void setBrightness(int newBrightness);
		
		inline bool grayscale(void) const
			{ return m_grayscale; }
		void setGrayscale(bool newGrayscale);
		
		inline bool inverted(void) const
			{ return m_inverted; }
		void setInverted(bool newInverted);
		
		inline ColorScaleMethod_t colorScaleMethod(void) const
			{ return m_colorScaleMethod; }
		void setColorScaleMethod(ColorScaleMethod_t newColorScaleMethod);
		
		inline ColorDepth bpp(void) const
			{ return m_bpp; }
		void setBpp(ColorDepth newBpp);
		
		/**
		 * mdColorMask: Set if color masking is enabled. (Mode 5 only)
		 * False: Full color range is used.
		 * True: Only LSBs are used.
		 */
		bool mdColorMask(void) const;
		void setMdColorMask(bool newMdColorMask);
		
		/**
		 * bgColorIdx: Background color index.
		 */
		int bgColorIdx(void) const;
		void setBgColorIdx(uint8_t newBgColorIdx);
		
		/** Palette manipulation functions. **/
		
		// Palette recalculation functions.
		void recalcFull(void);
		
		// Reset the active palettes.
		void resetActive(void);
		
		// Palette update functions.
		void updateMD(const VdpTypes::CRam_t *cram);
		void updateMD_HS(const VdpTypes::CRam_t *cram);
		
		// TODO
		//static void Adjust_CRam_32X(void);
	
	private:
		int m_contrast;
		int m_brightness;
		bool m_grayscale;
		bool m_inverted;
		ColorScaleMethod_t m_colorScaleMethod;
		ColorDepth m_bpp;
		
		// MD color mask. (Mode 5 only)
		uint16_t m_mdColorMask;
		static const uint16_t MD_COLOR_MASK_FULL;
		static const uint16_t MD_COLOR_MASK_LSB;
		
		// Background color index.
		uint8_t m_bgColorIdx;
		
		// Dirty flag.
		bool m_dirty;
		
		template<int mask>
		static FORCE_INLINE void T_ConstrainColorComponent(int& c);
		
		static FORCE_INLINE int CalcGrayscale(int r, int g, int b);
		static FORCE_INLINE void AdjustContrast(int& r, int& g, int& b, int contrast);
		
		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFullMD(pixel *palMD);
		
		template<bool hs, typename pixel>
		FORCE_INLINE void T_updateMD(pixel *MD_palette,
					const pixel *palette,
					const VdpTypes::CRam_t *cram);
};

/**
 * mdColorMask(): Check if the MD colors are masked.
 * @return True if all but LSBs are masked; false if full color range is available.
 */
inline bool VdpPalette::mdColorMask(void) const
	{ return (m_mdColorMask == MD_COLOR_MASK_LSB); }

/**
 * bgColorIdx: Get the background color index.
 * @return Background color index.
 */
inline int VdpPalette::bgColorIdx(void) const
	{ return m_bgColorIdx; }

}

#endif /* __LIBGENS_MD_VDPPALETTE_HPP__ */
