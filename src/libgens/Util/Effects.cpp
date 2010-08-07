/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Effects.hpp: Special Video Effects. (Software Rendering)                *
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

#include "Effects.hpp"
#include "MD/VdpRend.hpp"

// C includes.
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


namespace LibGens
{

/**
 * T_DoCrazyEffect(): Do the "Crazy" effect.
 * @param colorMask Color mask.
 */
template<typename pixel, pixel Rmask, pixel Gmask, pixel Bmask,
		  pixel Radd, pixel Gadd, pixel Badd>
void Effects::T_DoCrazyEffect(ColorMask colorMask, pixel *screen)
{
	if (colorMask == CM_BLACK)
	{
		// Intro effect color is black.
		// Simply clear the screen.
		memset(screen, 0x00, 336*240*sizeof(pixel));
		return;
	}
	
	const pixel RBmask = (Rmask | Bmask);
	int r = 0, g = 0, b = 0;
	pixel RB, G;
	
	pixel *pix = &screen[336*240 - 1];
	pixel *prev_l = pix - 336;
	pixel *prev_p = pix - 1;
	
	for (unsigned int i = 336*240; i != 0; i--)
	{
		pixel pl, pp;
		pl = (prev_l >= screen ? *prev_l : 0);
		pp = (prev_p >= screen ? *prev_p : 0);
		
		RB = ((pl & RBmask) + (pp & RBmask)) >> 1;
		G = ((pl & Gmask) + (pp & Gmask)) >> 1;
		
		if (colorMask & CM_RED)
		{
			// Red channel.
			r = RB & Rmask;
			r += (((rand() & 0x7FFF) > 0x2C00) ? Radd : -Radd);
			
			if (r > (int)Rmask)
				r = Rmask;
			else if (r < (int)Radd)
				r = 0;
		}
		
		if (colorMask & CM_GREEN)
		{
			// Green channel.
			g = G & Gmask;
			g += (((rand() & 0x7FFF) > 0x2C00) ? Gadd : -Gadd);
			
			if (g > (int)Gmask)
				g = Gmask;
			else if (g < (int)Gadd)
				g = 0;
		}
		
		if (colorMask & CM_BLUE)
		{
			// Blue channel.
			b = RB & Bmask;
			b += (((rand() & 0x7FFF) > 0x2C00) ? Badd : -Badd);
			
			if (b > (int)Bmask)
				b = Bmask;
			else if (b < (int)Badd)
				b = 0;
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
void Effects::DoCrazyEffect(ColorMask colorMask)
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
			T_DoCrazyEffect<uint32_t, 0xF80000, 0x00F800, 0x0000F8,
					0x080000, 0x000800, 0x000008>(colorMask, VdpRend::MD_Screen.u32);
			break;
	}
}

}