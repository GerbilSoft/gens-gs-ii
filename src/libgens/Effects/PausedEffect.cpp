/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * PausedEffect.cpp: "Paused" effect.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

// MD framebuffer.
#include "../Util/MdFb.hpp"

// C includes.
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Color component masks and shift values for 32-bit color.
#define PAUSED_MASK32_R  0x00FF0000
#define PAUSED_MASK32_G  0x0000FF00
#define PAUSED_MASK32_B  0x000000FF
#define PAUSED_SHIFT32_R (16+3)
#define PAUSED_SHIFT32_G (8+3)
#define PAUSED_SHIFT32_B (0+3)

namespace LibGens {

class PausedEffectPrivate
{
	private:
		PausedEffectPrivate();
		~PausedEffectPrivate();

	public:
		template<typename pixel, pixel RMask, pixel GMask, pixel BMask,
			 unsigned int RShift, unsigned int GShift, unsigned int BShift>
		static inline void T_DoPausedEffect(pixel* RESTRICT outScreen, const pixel* RESTRICT mdScreen);
		
		template<typename pixel, pixel RMask, pixel GMask, pixel BMask,
			 unsigned int RShift, unsigned int GShift, unsigned int BShift>
		static inline void T_DoPausedEffect(pixel* RESTRICT outScreen);
};

/**
 * Tint the screen a purple hue to indicate that emulation is paused.
 * @param pixel Type of pixel.
 * @param RMask Red component mask.
 * @param GMask Green component mask.
 * @param BMask Blue component mask.
 * @param RShift Red component shift.
 * @param GShift Green component shift.
 * @param BShift Blue component shift.
 * @param rInfo Rendering information.
 * @param scale Scaling value.
 * @param outScreen Pointer to the destination screen buffer. (MUST BE 336x240!)
 * @param mdScreen Pointer to the MD screen buffer. (MUST BE 336x240!)
 */
template<typename pixel, pixel RMask, pixel GMask, pixel BMask,
	 unsigned int RShift, unsigned int GShift, unsigned int BShift>
inline void PausedEffectPrivate::T_DoPausedEffect(pixel* RESTRICT outScreen, const pixel* RESTRICT mdScreen)
{
	// TODO: Improve precision for 32-bit color.
	uint8_t r, g, b, nRG, nB;
	float monoPx;
	
	for (unsigned int i = (336*240); i != 0; i--) {
		// Get the color components.
		r = (uint8_t)((*mdScreen & RMask) >> RShift);
		g = (uint8_t)((*mdScreen & GMask) >> GShift);
		b = (uint8_t)((*mdScreen & BMask) >> BShift);
		mdScreen++;

		// Convert the color components to grayscale.
		// TODO: SSE optimization.
		// Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
		// Source: http://en.wikipedia.org/wiki/YCbCr
		monoPx = ((float)r * 0.299f) + ((float)g * 0.587f) + ((float)b * 0.114f);
		nRG = (uint8_t)monoPx;
		if (nRG > 0x1F)
			nRG = 0x1F;

		// Left-shift the blue component to tint the image.
		nB = nRG << 1;
		if (nB > 0x1F)
			nB = 0x1F;

		// Put the new pixel.
		*outScreen++ = (nRG << RShift) | (nRG << GShift) | (nB << BShift);
	}
}

/**
 * Tint the screen a purple hue to indicate that emulation is paused.
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
inline void PausedEffectPrivate::T_DoPausedEffect(pixel* RESTRICT outScreen)
{
	// TODO: Improve precision for 32-bit color.
	uint8_t r, g, b, nRG, nB;
	float monoPx;

	for (unsigned int i = (336*240); i != 0; i--) {
		// Get the color components.
		r = (uint8_t)((*outScreen & RMask) >> RShift);
		g = (uint8_t)((*outScreen & GMask) >> GShift);
		b = (uint8_t)((*outScreen & BMask) >> BShift);

		// Convert the color components to monochrome.
		// TODO: SSE optimization.
		// Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
		// Source: http://en.wikipedia.org/wiki/YCbCr
		monoPx = ((float)r * 0.299f) + ((float)g * 0.587f) + ((float)b * 0.114f);
		nRG = (uint8_t)monoPx;
		if (nRG > 0x1F)
			nRG = 0x1F;

		// Left-shift the blue component to tint the image.
		nB = nRG << 1;
		if (nB > 0x1F)
			nB = 0x1F;

		// Put the new pixel.
		*outScreen++ = (nRG << RShift) | (nRG << GShift) | (nB << BShift);
	}
}

/**
 * Tint the screen a purple hue to indicate that emulation is paused.
 * @param outScreen Source and destination screen.
 */
void PausedEffect::DoPausedEffect(MdFb *outScreen)
{
	// Reference the framebuffer.
	outScreen->ref();

	// Render to outScreen.
	switch (outScreen->bpp()) {
		case MdFb::BPP_15:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 0x7C00, 0x03E0, 0x001F, 10, 5, 0>
				(outScreen->fb16());
			break;
		case MdFb::BPP_16:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 0xF800, 0x07C0, 0x001F, 11, 6, 0>
				(outScreen->fb16());
			break;
		case MdFb::BPP_32:
		default:
			PausedEffectPrivate::T_DoPausedEffect<uint32_t, PAUSED_MASK32_R, PAUSED_MASK32_G, PAUSED_MASK32_B,
					PAUSED_SHIFT32_R, PAUSED_SHIFT32_G, PAUSED_SHIFT32_B>
				(outScreen->fb32());
			break;
	}

	// Unreference the framebuffer.
	outScreen->unref();
}

/**
 * Tint the screen a purple hue to indicate that emulation is paused.
 * @param outScreen Destination screen.
 * @param mdScreen Source screen.
 */
void PausedEffect::DoPausedEffect(MdFb* RESTRICT outScreen, const MdFb* RESTRICT mdScreen)
{
	// Reference the framebuffers.
	outScreen->ref();
	mdScreen->ref();

	// Set outScreen's bpp to match mdScreen.
	outScreen->setBpp(mdScreen->bpp());

	// Render to outScreen.
	switch (outScreen->bpp()) {
		case MdFb::BPP_15:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 0x7C00, 0x03E0, 0x001F, 10, 5, 0>
				(outScreen->fb16(), mdScreen->fb16());
			break;
		case MdFb::BPP_16:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 0xF800, 0x07C0, 0x001F, 11, 6, 0>
				(outScreen->fb16(), mdScreen->fb16());
			break;
		case MdFb::BPP_32:
		default:
			PausedEffectPrivate::T_DoPausedEffect<uint32_t, PAUSED_MASK32_R, PAUSED_MASK32_G, PAUSED_MASK32_B,
					PAUSED_SHIFT32_R, PAUSED_SHIFT32_G, PAUSED_SHIFT32_B>
				(outScreen->fb32(), mdScreen->fb32());
			break;
	}

	// Unreference the framebuffers.
	outScreen->unref();
	mdScreen->unref();
}

}
