/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * CrazyEffect.cpp: "Crazy" effect.                                        *
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

/**
 * NOTE: The video effects here are applied to MD_Screen[].
 */

#include "CrazyEffect.hpp"
#include "MD/Vdp.hpp"

// C includes.
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Color component masks for 32-bit color.
#include "Util/byteswap.h"
#define CRAZY_MASK32_R 0x00F80000
#define CRAZY_MASK32_G 0x0000F800
#define CRAZY_MASK32_B 0x000000F8
#define CRAZY_ADD32_R  0x00080000
#define CRAZY_ADD32_G  0x00000800
#define CRAZY_ADD32_B  0x00000008

namespace LibGens
{

CrazyEffect::CrazyEffect()
{
	// Initialize the color mask to CM_WHITE.
	m_colorMask = CM_WHITE;
}

/**
 * T_doCrazyEffect(): Do the "Crazy" effect.
 * @param screen MD screen.
 */
template<typename pixel, pixel Rmask, pixel Gmask, pixel Bmask,
		  pixel Radd, pixel Gadd, pixel Badd>
inline void CrazyEffect::T_doCrazyEffect(pixel *screen)
{
	if (m_colorMask == CM_BLACK)
	{
		// Intro effect color is black.
		// Simply clear the screen.
		memset(screen, 0x00, 336*240*sizeof(pixel));
		return;
	}
	
	const pixel RBmask = (Rmask | Bmask);
	pixel r = 0, g = 0, b = 0;
	pixel RB, G;
	
	pixel *pix = &screen[336*240 - 1];
	pixel *prev_l = pix - 336;
	pixel *prev_p = pix - 1;
	
	// TODO: Unroll the last-line/last-pixel code.
	for (unsigned int i = 336*240; i != 0; i--)
	{
		pixel pl, pp;
		pl = (prev_l >= screen ? *prev_l : 0);
		pp = (prev_p >= screen ? *prev_p : 0);
		
		// Separate RB and G components.
		RB = ((pl & RBmask) + (pp & RBmask)) >> 1;
		G = ((pl & Gmask) + (pp & Gmask)) >> 1;
		
		if (m_colorMask & CM_RED)
		{
			// Red channel.
			r = RB & Rmask;
			if ((rand() & 0x7FFF) > 0x2C00)
			{
				if ((Rmask - Radd) <= r)
					r = Rmask;
				else
					r += Radd;
			}
			else
			{
				if (Radd >= r)
					r = 0;
				else
					r -= Radd;
			}
		}
		
		if (m_colorMask & CM_GREEN)
		{
			// Green channel.
			g = G & Gmask;
			if ((rand() & 0x7FFF) > 0x2C00)
			{
				if ((Gmask - Gadd) <= g)
					g = Gmask;
				else
					g += Gadd;
			}
			else
			{
				if (Gadd >= g)
					g = 0;
				else
					g -= Gadd;
			}
		}
		
		if (m_colorMask & CM_BLUE)
		{
			// Blue channel.
			b = RB & Bmask;
			if ((rand() & 0x7FFF) > 0x2C00)
			{
				if ((Bmask - Badd) <= b)
					b = Bmask;
				else
					b += Badd;
			}
			else
			{
				if (Badd >= b)
					b = 0;
				else
					b -= Badd;
			}
		}
		
		*pix = r | g | b;
		
		// Next pixels.
		prev_l--;
		prev_p--;
		pix--;
	}
}


/**
 * run(): Run the "Crazy" effect.
 */
void CrazyEffect::run(void)
{
	switch (Vdp::m_palette.bpp())
	{
		case VdpPalette::BPP_15:
			T_doCrazyEffect<uint16_t, 0x7C00, 0x03E0, 0x001F,
					0x0400, 0x0020, 0x0001>(Vdp::MD_Screen.fb16());
			break;
		
		case VdpPalette::BPP_16:
			T_doCrazyEffect<uint16_t, 0xF800, 0x07C0, 0x001F,
					0x0800, 0x0040, 0x0001>(Vdp::MD_Screen.fb16());
			break;
		
		case VdpPalette::BPP_32:
		default:
			T_doCrazyEffect<uint32_t, CRAZY_MASK32_R, CRAZY_MASK32_G, CRAZY_MASK32_B,
					CRAZY_ADD32_R, CRAZY_ADD32_G, CRAZY_ADD32_B>
					(Vdp::MD_Screen.fb32());
			break;
	}
}

}
