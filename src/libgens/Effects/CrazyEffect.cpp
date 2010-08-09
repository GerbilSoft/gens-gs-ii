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
#include "MD/VdpRend.hpp"

// C includes.
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Color component masks for 32-bit color.
#include "Util/byteswap.h"
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define CRAZY_MASK32_R 0x00F80000
#define CRAZY_MASK32_G 0x0000F800
#define CRAZY_MASK32_B 0x000000F8
#define CRAZY_ADD32_R  0x00080000
#define CRAZY_ADD32_G  0x00000800
#define CRAZY_ADD32_B  0x00000008
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
#define CRAZY_MASK32_R 0x0000F800
#define CRAZY_MASK32_G 0x00F80000
#define CRAZY_MASK32_B 0xF8000000
#define CRAZY_ADD32_R  0x00000800
#define CRAZY_ADD32_G  0x00080000
#define CRAZY_ADD32_B  0x08000000
#endif

#include <stdio.h>
#include "Util/Timing.hpp"

namespace LibGens
{

/**
 * T_DoCrazyEffect(): Do the "Crazy" effect.
 * @param colorMask Color mask.
 */
template<typename pixel, pixel Rmask, pixel Gmask, pixel Bmask,
		  pixel Radd, pixel Gadd, pixel Badd>
inline void CrazyEffect::T_DoCrazyEffect(ColorMask colorMask, pixel *screen)
{
	if (colorMask == CM_BLACK)
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
		
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
		if (sizeof(pixel) == 4)
		{
			// The Blue channel occupies the high byte in 32-bit color
			// on big-endian CPUs, so we have to right-shift the colors
			// before we add them together
			// (The low byte is the unused alpha channel.)
			RB = ((pl & RBmask) >> 1) + ((pp & RBmask) >> 1);
		}
		else
#else /* GENS_BYTEORDER == GENS_LIL_ENDIAN */
		RB = ((pl & RBmask) + (pp & RBmask)) >> 1;
#endif
		G = ((pl & Gmask) + (pp & Gmask)) >> 1;
		
		if (colorMask & CM_RED)
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
		
		if (colorMask & CM_GREEN)
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
		
		if (colorMask & CM_BLUE)
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
 * DoCrazyEffect(): Do the "Crazy" effect.
 * @param colorMask Color mask.
 */
void CrazyEffect::DoCrazyEffect(ColorMask colorMask)
{
	switch (VdpRend::Bpp)
	{
		case VdpRend::BPP_15:
			T_DoCrazyEffect<uint16_t, 0x7C00, 0x03E0, 0x001F,
					0x0400, 0x0020, 0x0001>(colorMask, VdpRend::MD_Screen.u16);
			break;
		
		case VdpRend::BPP_16:
			T_DoCrazyEffect<uint16_t, 0xF800, 0x07C0, 0x001F,
					0x0800, 0x0040, 0x0001>(colorMask, VdpRend::MD_Screen.u16);
			break;
		
		case VdpRend::BPP_32:
		default:
			T_DoCrazyEffect<uint32_t, CRAZY_MASK32_R, CRAZY_MASK32_G, CRAZY_MASK32_B,
					CRAZY_ADD32_R, CRAZY_ADD32_G, CRAZY_ADD32_B>
					(colorMask, VdpRend::MD_Screen.u32);
			break;
	}
}

}
