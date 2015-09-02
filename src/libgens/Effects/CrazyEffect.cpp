/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * CrazyEffect.cpp: "Crazy" effect.                                        *
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

#include "CrazyEffect.hpp"
#include "Util/MdFb.hpp"

// C includes.
#include <stdlib.h>
#include <stdint.h>

// C includes. (C++ namespace)
#include <cmath>
#include <cstring>
#include <cstdio>

namespace LibGens {

// TODO: Private class.

/**
 * Get a random number in the range [0,0x7FFF].
 * This uses the internal random number
 * cache if it's available.
 * @return Random number.
 */
unsigned int CrazyEffect::getRand(void)
{
	// TODO: Faster rand() implementation, e.g. Mersenne Twister?
	unsigned int ret;
#if RAND_MAX >= 0x3FFFFFFF
	if (rand_cache <= 0x7FFF) {
		// rand_cache is valid.
		ret = rand_cache;
		rand_cache = ~0;
		return ret;
	}
#endif

	// Get a random number.
	ret = rand();
#if RAND_MAX >= 0x3FFFFFFF
	// Cache the high bits as a second random number.
	rand_cache = (ret >> 15) & 0x7FFF;
	ret &= 0x7FFF;
#endif
	return ret;
}

/**
 * Adjust a pixel's color.
 * @param pixel     [in] Type of pixel.
 * @param add_shift [in] Shift value for the add value.
 * @param px        [in] Pixel data.
 * @param mask      [in] Pixel mask.
 * @return Adjusted pixel color.
 */
template<typename pixel, uint8_t add_shift>
inline pixel CrazyEffect::adj_color(pixel px, pixel mask)
{
	pixel add = 1 << add_shift;
	px &= mask;
	if ((getRand() & 0x7FFF) > 0x2C00) {
		if ((mask - add) <= px) {
			px = mask;
		} else {
			px += add;
		}
	} else {
		if (add >= px) {
			px = 0;
		} else {
			px -= add;
		}
	}
	return px;
}

/**
 * Do the "Crazy" effect.
 * @param pixel     [in]  Type of pixel.
 * @param RBits     [in]  Number of bits for Red.
 * @param GBits     [in]  Number of bits for Green.
 * @param BBits     [in]  Number of bits for Blue.
 * @param outScreen [out] Destination screen.
 */
template<typename pixel, uint8_t RBits, uint8_t GBits, uint8_t BBits>
inline void CrazyEffect::T_doCrazyEffect(pixel *outScreen)
{
	if (m_colorMask == CM_BLACK) {
		// Intro effect color is black.
		// Simply clear the screen.
		memset(outScreen, 0x00, 336*240*sizeof(pixel));
		return;
	}

	const pixel Rmask = (0x1F << (RBits-5)) << (GBits + BBits);
	const pixel Gmask = (0x1F << (GBits-5)) << BBits;
	const pixel Bmask = (0x1F << (BBits-5));
	const pixel RBmask = (Rmask | Bmask);
	pixel r = 0, g = 0, b = 0;
	pixel RB, G;

	pixel *pix = &outScreen[336*240 - 1];
	pixel *prev_l = pix - 336;
	pixel *prev_p = pix - 1;

	// TODO: Unroll the last-line/last-pixel code.
	for (unsigned int i = 336*240; i != 0; i--) {
		pixel pl, pp;
		pl = (prev_l >= outScreen ? *prev_l : 0);
		pp = (prev_p >= outScreen ? *prev_p : 0);

		// Separate RB and G components.
		RB = ((pl & RBmask) + (pp & RBmask)) >> 1;
		G = ((pl & Gmask) + (pp & Gmask)) >> 1;

		if (m_colorMask & CM_RED) {
			// Red channel.
			r = adj_color<pixel, (RBits-5)+GBits+BBits>(RB, Rmask);
		}
		if (m_colorMask & CM_GREEN) {
			// Green channel.
			g = adj_color<pixel, (GBits-5)+BBits>(G, Gmask);
		}
		if (m_colorMask & CM_BLUE) {
			// Blue channel.
			b = adj_color<pixel, (BBits-5)>(RB, Bmask);
		}

		// Combine the color components.
		*pix = r | g | b;

		// Next pixels.
		prev_l--;
		prev_p--;
		pix--;
	}
}

/** CrazyEffect **/

CrazyEffect::CrazyEffect()
	: m_colorMask(CM_WHITE)
#if RAND_MAX >= 0x3FFFFFFF
	, rand_cache(~0)
#endif
{ }

/**
 * Run the "Crazy" effect.
 * @param fb MdFb to apply the effect to.
 */
void CrazyEffect::run(MdFb *fb)
{
	// FIXME: Add a bpp value to fb.
	switch (2 /*fb.bpp()*/) {
		case 0: /*VdpPalette::BPP_15:*/
			T_doCrazyEffect<uint16_t, 5, 5, 5>(fb->fb16());
			break;

		case 1: /*VdpPalette::BPP_16:*/
			T_doCrazyEffect<uint16_t, 5, 6, 5>(fb->fb16());
			break;

		case 2: /*VdpPalette::BPP_32:*/
		default:
			T_doCrazyEffect<uint32_t, 8, 8, 8>(fb->fb32());
			break;
	}
}

}
