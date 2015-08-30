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

#include "libcompat/aligned_malloc.h"

namespace LibGens {

/**
 * Tint the screen a purple hue to indicate that emulation is paused.
 * (32-bit color, SSE2-optimized.)
 * @param outScreen Pointer to the source/destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer. [2-FB only]
 * @param pxCount Pixel count.
 */
inline void PausedEffectPrivate::DoPausedEffect_32_SSE2(
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
	static const uint64_t ALIGN(16) GRAY_32_SSE2[2] = {0x0000004D0096001D, 0x0000004D0096001D};
	static const uint64_t ALIGN(16) BLUE_MASK_32_SSE2[2] = {0x000000000000FFFF, 0x000000000000FFFF};
	__asm__ (
		"movdqa	%[GRAY_32_SSE2], %%xmm7\n"
		"movdqa	%[BLUE_MASK_32_SSE2], %%xmm6\n"
		"pxor	%%xmm0, %%xmm0\n"
		:
		: [GRAY_32_SSE2] "m" (GRAY_32_SSE2)
		, [BLUE_MASK_32_SSE2] "m" (BLUE_MASK_32_SSE2)
	);

	// Make sure the framebuffer(s) are 16-byte aligned.
	assert((uintptr_t)outScreen % 16 == 0);
#ifdef DO_2FB
	assert((uintptr_t)mdScreen % 16 == 0);
#endif

	// Convert the pixels to grayscale.
	// TODO: Apply the Blue tint.
	// TODO: Do more than 4px at a time?
	assert(pxCount % 4 == 0);
	for (pxCount /= 4; pxCount > 0; pxCount--) {
		// %xmm0 == 0
		// %xmm1 == first pixel as words
		// %xmm2 == second pixel as words
		// %xmm3 == px1 temporary
		// %xmm4 == px2 temporary
		// %xmm6 == blue mask (for tint)
		// %xmm7 == grayscale vector
		__asm__ (
#ifdef DO_2FB
			"movdqa		(%[mdScreen]), %%xmm1\n"	// Get 4 pixels.
#else /* DO_1FB */
			"movdqa		(%[outScreen]), %%xmm1\n"	// Get 4 pixels.
#endif
			"movdqa		%%xmm1, %%xmm2\n"		// Same 4 pixels.
			"punpcklbw	%%xmm0, %%xmm1\n"		// Unpack %xmm1 into words using %xmm0 as high bytes.
			"punpckhbw	%%xmm0, %%xmm2\n"		// Unpack %xmm2 into words using %xmm0 as low bytes.

			"pmaddwd	%%xmm7, %%xmm1\n"		// %xmm1 == [px1] 0 + R * MULT | G * MULT + B * MULT
			"pmaddwd	%%xmm7, %%xmm2\n"		// %xmm2 == [px2] 0 + R * MULT | G * MULT + B * MULT

			// Temporarily use mm3/mm4 to get the R values.
			"movdqa		%%xmm1, %%xmm3\n"		// %xmm3 == [px1] 0 + R * MULT | G * MULT + B * MULT
			"movdqa		%%xmm2, %%xmm4\n"		// %xmm4 == [px2] 0 + R * MULT | G * MULT + B * MULT
			"psrlq		   $32, %%xmm3\n"		// %xmm3 == [px1]            0 | 0 + R * MULT
			"psrlq		   $32, %%xmm4\n"		// %xmm4 == [px2]            0 | 0 + R * MULT
			// Add %xmm3/%xmm4 back to %xmm1/%xmm2.
			"paddd		%%xmm3, %%xmm1\n"		// %xmm1 == [px1] 0 + R * MULT | R * MULT + G * MULT + B * MULT
			"paddd		%%xmm4, %%xmm2\n"		// %xmm2 == [px2] 0 + R * MULT | R * MULT + G * MULT + B * MULT

			// Shuffle low words to create an RGB value with 16 bits per component.
			// %xmm1[63:0] <- %xmm1[63:48], %xmm1[15:0], %xmm1[15:0], %xmm1[15:0]
			"pshuflw	$0xC0, %%xmm1, %%xmm1\n"
			"pshuflw	$0xC0, %%xmm2, %%xmm2\n"
			// Shuffle high words to create an RGB value with 16 bits per component.
			// xmm1[127:64] <- xmm1[127:112], xmm1[79:64], xmm1[79:64], xmm1[79:64]
			"pshufhw	$0xC0, %%xmm1, %%xmm1\n"
			"pshufhw	$0xC0, %%xmm2, %%xmm2\n"

			// Mask the blue values and add it again to tint.
			"movdqa         %%xmm1, %%xmm3\n"
			"movdqa		%%xmm2, %%xmm4\n"
			"pand		%%xmm6, %%xmm3\n"
			"pand		%%xmm6, %%xmm4\n"
			// NOTE: We're doing byte-wise adds because word-wise would
			// have extra precision, which can cause the blue value to
			// be slightly more than double the grayscale value.
			"paddusb	%%xmm3, %%xmm1\n"
			"paddusb	%%xmm4, %%xmm2\n"

			// Shift each word to create 8-bit values.
			"psrlw		$8, %%xmm1\n"
			"psrlw		$8, %%xmm2\n"

			// Repack %xmm1 and %xmm2 into a single 128-bit value, %xmm1.
			"packuswb	%%xmm2, %%xmm1\n"

			// Write to the output screen.
			"movdqa		%%xmm1, (%[outScreen])\n"
			:
			: [outScreen] "r" (outScreen)
#ifdef DO_2FB
			, [mdScreen] "r" (mdScreen)
#endif
			// FIXME: gcc complains that xmm? registers are unknown.
			// May need to compile with -msse...
			//: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4"
		);

		// Next group of pixels.
		outScreen += 4;
#ifdef DO_2FB
		mdScreen += 4;
#endif
	}
}

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

			// TODO: Use a blue mask to mask out the blue channel for doubling?

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
			// FIXME: gcc complains that mm? registers are unknown.
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
