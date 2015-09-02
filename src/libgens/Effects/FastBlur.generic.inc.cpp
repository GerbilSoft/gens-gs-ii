/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * FastBlur.generic.inc.cpp: Fast Blur effect. (Generic version.)          *
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

#include <stdio.h>
#ifndef __IN_LIBGENS_FASTBLUR_CPP__
#error FastBlur.generic.inc.cpp should only be included by FastBlur.cpp.
#endif

#if (!defined(DO_1FB) && !defined(DO_2FB)) || \
    ( defined(DO_1FB) &&  defined(DO_2FB))
#error Must define DO_1FB or DO_2FB, but not both.
#endif

#include "libcompat/aligned_malloc.h"

namespace LibGens {

/**
 * 15/16-bit color Fast Blur. (Generic version)
 * @param outScreen [out] Destination screen.
 * @param mdScreen  [in]  Source screen. [2-FB only]
 * @param pxCount   [in]  Pixel count.
 * @param mask      [in]  Division mask to use. (MASK_DIV2_15 or MASK_DIV2_16)
 */
void FastBlurPrivate::DoFastBlur_16(
	uint16_t* RESTRICT outScreen,
#ifdef DO_2FB
	const uint16_t* RESTRICT mdScreen,
#endif
	unsigned int pxCount,
	uint16_t mask)
{
	uint16_t px, px_prev;

	// Read the first pixel as the previous pixel
	// to prevent issues.
#ifdef DO_2FB
	px_prev = (*mdScreen >> 1) & mask;
#else /* DO_1FB */
	px_prev = (*outScreen >> 1) & mask;
#endif

	// Blur 1px at a time.
	// TODO: Try blurring 2px, or 4px on 64-bit?
	// and/or unroll the loop.
	//assert(pxCount % 1 == 0);
	for (/*pxCount /= 1*/; pxCount > 0; pxCount--) {
		// Get 1 pixel.
		// NOTE: This may read 1px over at the end of the buffer.
#ifdef DO_2FB
		px = (*(mdScreen+1) >> 1) & mask;
#else /* DO_1FB */
		px = (*(outScreen+1) >> 1) & mask;
#endif

		px_prev += px;		// Blur with previous pixel.
		*outScreen = px_prev;	// Write new pixel.
		px_prev = px;		// Save pixel.

		// Next group of pixels.
		outScreen += 1;
#ifdef DO_2FB
		mdScreen += 1;
#endif
	}
}

/**
 * 32-bit color Fast Blur, MMX-optimized.
 * @param outScreen [out] Destination screen.
 * @param mdScreen  [in]  Source screen. [2-FB only]
 * @param pxCount   [in]  Pixel count.
 */
void FastBlurPrivate::DoFastBlur_32(
	uint32_t* RESTRICT outScreen,
#ifdef DO_2FB
	const uint32_t* RESTRICT mdScreen,
#endif
	unsigned int pxCount)
{
	static const uint32_t MASK_DIV2_32 = 0x007F7F7F;
	uint32_t px, px_prev;

	// Read the first pixel as the previous pixel
	// to prevent issues.
#ifdef DO_2FB
	px_prev = (*mdScreen >> 1) & MASK_DIV2_32;
#else /* DO_1FB */
	px_prev = (*outScreen >> 1) & MASK_DIV2_32;
#endif

	// Blur 1px at a time.
	// TODO: 2px on 64-bit, and/or unroll loop?
	//assert(pxCount % 1 == 0);
	for (/*pxCount /= 1*/; pxCount > 0; pxCount--) {
		// Get 1 pixel.
		// NOTE: This may read 1px over at the end of the buffer.
#ifdef DO_2FB
		px = (*(mdScreen+1) >> 1) & MASK_DIV2_32;
#else /* DO_1FB */
		px = (*(outScreen+1) >> 1) & MASK_DIV2_32;
#endif

		px_prev += px;		// Blur with previous pixel.
		*outScreen = px_prev;	// Write new pixel.
		px_prev = px;		// Save pixel.

		// Next group of pixels.
		outScreen += 1;
#ifdef DO_2FB
		mdScreen += 1;
#endif
	}
}

}
