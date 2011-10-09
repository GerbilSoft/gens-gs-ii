/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_Err_p.hpp: VDP error message class. (Private class.)            *
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

/// NOTE: This MUST be #include'd within the LibGens namespace!
/// DO NOT #include this file outside of LibGens!

#ifndef __LIBGENS_VDP_VDPRENDERR_P_HPP__
#define __LIBGENS_VDP_VDPRENDERR_P_HPP__

#include <stdint.h>

#include "Util/MdFb.hpp"

/**
 * Vdp: Error message rendering.
 * Private class.
 */
class VdpRend_Err_Private
{
	public:
		VdpRend_Err_Private(Vdp *q);
	
	private:
		Vdp *const q;
		
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		VdpRend_Err_Private(const VdpRend_Err_Private &);
		VdpRend_Err_Private &operator=(const VdpRend_Err_Private &);
	
	public:
		static const uint16_t ColorBarsPalette_15[22];
		static const uint16_t ColorBarsPalette_16[22];
		static const uint32_t ColorBarsPalette_32[22];
		
		// VDP mode data.
		unsigned int lastVdpMode;
		int lastHPix;
		int lastVPix;
		VdpPalette::ColorDepth lastBpp;
	
	public:
		// Drawing functions.
		template<typename pixel>
		void T_DrawColorBars(MdFb *fb, const pixel palette[22]);
		
		template<typename pixel>
		void T_DrawColorBars_Border(MdFb *fb, const pixel bg_color);
		
		template<typename pixel, pixel text_color>
		void T_DrawChr(pixel *screen, int chr);
		
	private:
		template<typename pixel, pixel text_color>
		void T_DrawText(MdFb *fb, int x, int y, const char *str);
	
	public:
		template<typename pixel, pixel text_color>
		void T_DrawVDPErrorMessage(MdFb *fb);
};

#endif /* __LIBGENS_VDP_VDPRENDERR_P_HPP__ */
