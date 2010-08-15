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
		
		bool isDirty(void) const { return m_dirty; }
		
		// Properties.
		int contrast(void) const { return m_contrast; }
		void setContrast(int newContrast)
		{
			if (newContrast == m_contrast)
				return;
			m_contrast = newContrast;
			m_dirty = true;
		}
		
		int brightness(void) const { return m_brightness; }
		void setBrightness(int newBrightness)
		{
			if (newBrightness == m_brightness)
				return;
			m_brightness = newBrightness;
			m_dirty = true;
		}
		
		bool grayscale(void) const { return m_grayscale; }
		void setGrayscale(bool newGrayscale)
		{
			if (newGrayscale == m_grayscale)
				return;
			m_grayscale = newGrayscale;
			m_dirty = true;
		}
		
		bool invertColor(void) const { return m_invertColor; }
		void setInvertColor(bool newInvertColor)
		{
			if (newInvertColor == m_invertColor)
				return;
			m_invertColor = newInvertColor;
			m_dirty = true;
		}
		
		ColorDepth bpp(void) const { return m_bpp; }
		void setBpp(ColorDepth newBpp)
		{
			if (m_bpp == newBpp)
				return;
			m_bpp = newBpp;
			m_dirty = true;
		}
		
		// Palette recalculation functions.
		void recalcFull(void);
		void recalcMD(void);
		
		// TODO
		//static void Adjust_CRam_32X(void);
	
	protected:
		int m_contrast;
		int m_brightness;
		bool m_grayscale;
		bool m_invertColor;
		ColorScaleMethod_t m_csm;
		ColorDepth m_bpp;
		
		// Dirty flag.
		bool m_dirty;
		
		// TODO: Port FORCE_INLINE from Gens/GS.
		#define FORCE_INLINE inline
		
		template<int mask>
		static FORCE_INLINE void T_ConstrainColorComponent(int& c);
		
		static FORCE_INLINE int CalcGrayscale(int r, int g, int b);
		static FORCE_INLINE void AdjustContrast(int& r, int& g, int& b, int contrast);
		
		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		FORCE_INLINE void T_recalcFullMD(pixel *palMD);
};

}

#endif /* __LIBGENS_MD_VDPPALETTE_HPP__ */
