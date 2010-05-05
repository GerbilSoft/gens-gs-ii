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
		// Full MD palette.
		union Palette_t
		{
			uint16_t u16[0x1000];
			uint32_t u32[0x1000];
		};
		static Palette_t Palette;
		
		static int Contrast;
		static int Brightness;
		static bool Grayscale;
		static bool InvertColor;
		
		// Color scale method.
		enum ColorScaleMethod_t
		{
			COLSCALE_RAW = 0,	// Raw colors: 0xEEE -> 0xE0E0E0
			COLSCALE_FULL = 1,	// Full colors: 0xEEE -> 0xFFFFFF
			COLSCALE_FULL_HS = 2,	// Full colors with Highlight/Shadow: 0xEEE -> 0xEEEEEE for highlight
		};
		static ColorScaleMethod_t ColorScaleMethod;
		
		static void Recalc(void);
		
		// TODO
		static void Adjust_CRam_32X(void);
	
	protected:
		// TODO: Port FORCE_INLINE from Gens/GS.
		#define FORCE_INLINE inline
		
		template<int mask>
		static FORCE_INLINE void T_ConstrainColorComponent(int& c);
		
		static FORCE_INLINE int CalcGrayscale(int r, int g, int b);
		static FORCE_INLINE void AdjustContrast(int& r, int& g, int& b, int contrast);
		
		template<typename pixel,
			int RBits, int GBits, int BBits,
			int RMask, int GMask, int BMask>
		static FORCE_INLINE void T_Recalc_MD(pixel *palMD);
	
	private:
		VdpPalette() { }
		~VdpPalette() { }
};

}

#endif /* __LIBGENS_MD_VDPPALETTE_HPP__ */
