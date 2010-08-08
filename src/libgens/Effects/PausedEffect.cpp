/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * PausedEffect.cpp: "Paused" effect.                                      *
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

#include "PausedEffect.hpp"
#include "MD/VdpRend.hpp"

// C includes.
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

namespace LibGens
{

/**
 * T_DoPausedEffect(): Tint the screen a purple hue to indicate that emulation is paused.
 * @param pixel Type of pixel.
 * @param RMask Red component mask.
 * @param GMask Green component mask.
 * @param BMask Blue component mask.
 * @param RShift Red component shift.
 * @param GShift Green component shift.
 * @param BShift Blue component shift.
 * @param rInfo Rendering information.
 * @param scale Scaling value.
 * @param mdScreen Pointer to the MD screen buffer. (MUST BE 336x240!)
 * @param outScreen Pointer to the destination screen buffer. (MUST BE 336x240!)
 */
template<typename pixel, pixel RMask, pixel GMask, pixel BMask,
	 unsigned int RShift, unsigned int GShift, unsigned int BShift>
inline void PausedEffect::T_DoPausedEffect(const pixel* RESTRICT mdScreen, pixel* RESTRICT outScreen)
{
	// TODO: Adjust this function for RGB Color Scaling.
	uint8_t r, g, b, nr, ng, nb;
	float monoPx;
	
	for (unsigned int i = (336*240); i != 0; i--)
	{
		// Get the color components.
		r = (uint8_t)((*mdScreen & RMask) >> RShift);
		g = (uint8_t)((*mdScreen & GMask) >> GShift);
		b = (uint8_t)((*mdScreen & BMask) >> BShift);
		mdScreen++;
		
		// Convert the color components to monochrome.
		// TODO: SSE optimization.
		// Monochrome vector: [0.30 0.59 0.11]
		monoPx = ((float)r * 0.30f) + ((float)g * 0.59f) + ((float)b * 0.11f);
		nr = ng = nb = (uint8_t)monoPx;
		
		// Left-shift the blue component to tint the image.
		nb <<= 1;
		if (nb > 0x1F)
			nb = 0x1F;
		
		// Mask off the LSB.
		nr &= 0x1E;
		ng &= 0x1E;
		nb &= 0x1E;
		
		// Put the new pixel.
		*outScreen++ = (nr << RShift) | (ng << GShift) | (nb << BShift);
	}
}


/**
 * T_DoPausedEffect(): Tint the screen a purple hue to indicate that emulation is paused.
 * @param pixel Type of pixel.
 * @param RMask Red component mask.
 * @param GMask Green component mask.
 * @param BMask Blue component mask.
 * @param RShift Red component shift.
 * @param GShift Green component shift.
 * @param BShift Blue component shift.
 * @param rInfo Rendering information.
 * @param scale Scaling value.
 * @param outScreen Pointer to the source/destination screen buffer. (MUST BE 336x240!)
 */
template<typename pixel, pixel RMask, pixel GMask, pixel BMask,
	 unsigned int RShift, unsigned int GShift, unsigned int BShift>
inline void PausedEffect::T_DoPausedEffect(pixel* RESTRICT outScreen)
{
	// TODO: Adjust this function for RGB Color Scaling.
	uint8_t r, g, b, nr, ng, nb;
	float monoPx;
	
	for (unsigned int i = (336*240); i != 0; i--)
	{
		// Get the color components.
		r = (uint8_t)((*outScreen & RMask) >> RShift);
		g = (uint8_t)((*outScreen & GMask) >> GShift);
		b = (uint8_t)((*outScreen & BMask) >> BShift);
		
		// Convert the color components to monochrome.
		// TODO: SSE optimization.
		// Monochrome vector: [0.30 0.59 0.11]
		monoPx = ((float)r * 0.30f) + ((float)g * 0.59f) + ((float)b * 0.11f);
		nr = ng = nb = (uint8_t)monoPx;
		
		// Left-shift the blue component to tint the image.
		nb <<= 1;
		if (nb > 0x1F)
			nb = 0x1F;
		
		// Mask off the LSB.
		nr &= 0x1E;
		ng &= 0x1E;
		nb &= 0x1E;
		
		// Put the new pixel.
		*outScreen++ = (nr << RShift) | (ng << GShift) | (nb << BShift);
	}
}


/**
 * DoPausedEffect(): Tint the screen a purple hue to indicate that emulation is paused.
 * @param outScreen Output screen. (MUST BE 336x240 with enough space for the current color depth!)
 * @param fromMdScreen If true, uses VdpRend::MD_Screen[] as the source buffer.
 * Otherwise, outScreen is used as both source and destination.
 */
void PausedEffect::DoPausedEffect(void *outScreen, bool fromMdScreen)
{
	if (fromMdScreen)
	{
		// Render from MD_Screen[] to outScreen.
		switch (VdpRend::Bpp)
		{
			case VdpRend::BPP_15:
				T_DoPausedEffect<uint16_t, 0x7C00, 0x03E0, 0x001F, 10, 5, 0>
					(VdpRend::MD_Screen.u16, (uint16_t*)outScreen);
				break;
			case VdpRend::BPP_16:
				T_DoPausedEffect<uint16_t, 0xF800, 0x07C0, 0x001F, 11, 6, 0>
					(VdpRend::MD_Screen.u16, (uint16_t*)outScreen);
				break;
			case VdpRend::BPP_32:
			default:
				T_DoPausedEffect<uint32_t, 0xFF0000, 0x00FF00, 0x0000FF, 16+3, 8+3, 0+3>
					(VdpRend::MD_Screen.u32, (uint32_t*)outScreen);
				break;
		}
	}
	else
	{
		// Update outScreen only.
		switch (VdpRend::Bpp)
		{
			case VdpRend::BPP_15:
				T_DoPausedEffect<uint16_t, 0x7C00, 0x03E0, 0x001F, 10, 5, 0>
					((uint16_t*)outScreen);
				break;
			case VdpRend::BPP_16:
				T_DoPausedEffect<uint16_t, 0xF800, 0x07C0, 0x001F, 11, 6, 0>
					((uint16_t*)outScreen);
				break;
			case VdpRend::BPP_32:
			default:
				T_DoPausedEffect<uint32_t, 0xFF0000, 0x00FF00, 0x0000FF, 16+3, 8+3, 0+3>
					((uint32_t*)outScreen);
				break;
		}
	}
}

}
