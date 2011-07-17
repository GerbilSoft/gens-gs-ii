/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_Err.hpp: VDP error message class.                               *
 *                                                                         *
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

#ifndef __LIBGENS_MD_VDPREND_ERR_HPP__
#define __LIBGENS_MD_VDPREND_ERR_HPP__

#include <stdint.h>
#include "VdpPalette.hpp"

namespace LibGens
{

class VdpRend_Err
{
	public:
		static void Render_Line(void);
		static void Update(void);
	
	private:
		VdpRend_Err() { }
		~VdpRend_Err() { }
		
		static const uint16_t ColorBarsPalette_15[22];
		static const uint16_t ColorBarsPalette_16[22];
		static const uint32_t ColorBarsPalette_32[22];
		
		// VDP mode data.
		static unsigned int ms_LastVdpMode;
		static int ms_LastHPix;
		static int ms_LastVPix;
		static VdpPalette::ColorDepth ms_LastBpp;
		
		template<typename pixel>
		static void T_DrawColorBars(pixel *screen, const pixel palette[22]);
		
		template<typename pixel>
		static void T_DrawColorBars_Border(pixel *screen, const pixel bg_color);
		
		template<typename pixel, pixel text_color>
		static void T_DrawChr(pixel *screen, int chr);
		
		template<typename pixel, pixel text_color>
		static void T_DrawText(pixel *screen, int x, int y, const char *str);
		
		template<typename pixel, pixel text_color>
		static void T_DrawVDPErrorMessage(pixel *screen);
};

}

#endif /* __LIBGENS_MD_VDPREND_ERR_HPP__ */
