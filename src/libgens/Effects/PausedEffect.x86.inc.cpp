/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * PausedEffect.x86.inc.cpp: "Paused" effect. (i386/amd64 optimized.)      *
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

#ifndef __IN_LIBGENS_PAUSEDEFFECT_CPP__
#error PausedEffect.x86.inc.cpp should only be included by PausedEffect.cpp.
#endif

#if !defined(__GNUC__) || !(defined(__i386__) || defined(__amd64__))
#error PausedEffect.x86.inc.cpp should only be compiled on i386/amd64 with gcc.
#endif

#if (!defined(DO_1FB) && !defined(DO_2FB)) || \
    ( defined(DO_1FB) &&  defined(DO_2FB))
#error Must define DO_1FB or DO_2FB, but not both.
#endif

namespace LibGens {

/**
 * Tint the screen a purple hue to indicate that emulation is paused.
 * (32-bit color, MMX-optimized.)
 * @param outScreen Pointer to the source/destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer. [2-FB only]
 * @param pxCount Pixel count.
 */
inline void PausedEffectPrivate::DoPausedEffect_32_MMX(
	uint32_t* RESTRICT outScreen,
#ifdef DO_2FB
	const uint32_t* RESTRICT mdScreen,
#endif
	unsigned int pxCount)
{
	// Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
	// Source: http://en.wikipedia.org/wiki/YCbCr

	// Load the 32-bit grayscale vector.
	// Reference: http://www.asmcommunity.net/forums/topic/?id=19704
	static const uint64_t GRAY_32_MMX = 0x0000004D0096001D;
	__asm__ (
		"movq	%[GRAY_32_MMX], %%mm7\n"
		"pxor	%%mm0, %%mm0\n"
		:
		: [GRAY_32_MMX] "m" (GRAY_32_MMX)
	);

	// Convert the pixels to grayscale.
	// TODO: Do more than 2px at a time?
	assert(pxCount % 2 == 0);
	for (pxCount /= 2; pxCount > 0; pxCount--) {
		// %mm0 == 0
		// %mm1 == first pixel as words
		// %mm2 == second pixel as words
		// %mm3 == px1 temporary
		// %mm4 == px2 temporary
		// %mm7 == grayscale vector
		__asm__ (
#ifdef DO_2FB
			"movq		(%[mdScreen]), %%mm1\n"		// Get 2 pixels.
#else /* DO_1FB */
			"movq		(%[outScreen]), %%mm1\n"	// Get 2 pixels.
#endif
			"movq		%%mm1, %%mm2\n"			// Same 2 pixels.
			"punpcklbw	%%mm0, %%mm1\n"			// Unpack %mm1 into words using %mm0 as high bytes.
			"punpckhbw	%%mm0, %%mm2\n"			// Unpack %mm2 into words using %mm0 as low bytes.

			"pmaddwd	%%mm7, %%mm1\n"			// %mm1 == [px1] 0 + R * MULT | G * MULT + B * MULT
			"pmaddwd	%%mm7, %%mm2\n"			// %mm2 == [px2] 0 + R * MULT | G * MULT + B * MULT

			// Temporarily use mm3/mm4 to get the R values.
			"movq		%%mm1, %%mm3\n"			// %mm3 == [px1] 0 + R * MULT | G * MULT + B * MULT
			"movq		%%mm2, %%mm4\n"			// %mm4 == [px2] 0 + R * MULT | G * MULT + B * MULT
			"psrlq		  $32, %%mm3\n"			// %mm3 == [px1]            0 | 0 + R * MULT
			"psrlq		  $32, %%mm4\n"			// %mm4 == [px2]            0 | 0 + R * MULT
			// Add %mm3/%mm4 back to %mm1/%mm2.
			"paddd		%%mm3, %%mm1\n"			// %mm1 == [px1] 0 + R * MULT | R * MULT + G * MULT + B * MULT
			"paddd		%%mm4, %%mm2\n"			// %mm2 == [px2] 0 + R * MULT | R * MULT + G * MULT + B * MULT

			// The relevant grayscale values are in the low 16 bits of %mm1 and %mm2.
			// Pack them into a single value.
			"movd		%%mm1, %%eax\n"			// %ax == grayscale
			"movd		%%mm2, %%edx\n"			// %dx == grayscale

			// TODO: Optimize this.
			"movb		%%ah, 1(%[outScreen])\n"
			"movb		%%ah, 2(%[outScreen])\n"
			"movb		  $0, 3(%[outScreen])\n"
			"movb		%%dh, 5(%[outScreen])\n"
			"movb		%%dh, 6(%[outScreen])\n"
			"movb		  $0, 7(%[outScreen])\n"

			// Double the blue value.
			// NOTE: We're doing byte-wise adds because word-wise would
			// have extra precision, which can cause the blue value to
			// be slightly more than double the grayscale value.
			"paddusb	%%mm1, %%mm1\n"
			"paddusb	%%mm2, %%mm2\n"
			"movd		%%mm1, %%eax\n"			// %ax == doubled grayscale
			"movd		%%mm2, %%edx\n"			// %dx == doubled grayscale
			"movb		%%ah, 0(%[outScreen])\n"
			"movb		%%dh, 4(%[outScreen])\n"
			:
			: [outScreen] "r" (outScreen)
#ifdef DO_2FB
			, [mdScreen] "r" (mdScreen)
#endif
			// FIXME: gcc complains mm? registers are unknown.
			// May need to compile with -mmmx...
			//: "mm0", "mm1", "mm2", "mm3", "mm4"
			: "eax", "edx"
		);

		// Next group of pixels.
		outScreen += 2;
#ifdef DO_2FB
		mdScreen += 2;
#endif
	}

	// Reset the FPU state.
	__asm__ __volatile__ ("emms");
}

}
