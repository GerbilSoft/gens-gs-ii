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

#include "PausedEffect.hpp"

// MD framebuffer.
#include "../Util/MdFb.hpp"

// C includes.
#include <stdlib.h>
#include <stdint.h>

// C includes. (C++ namespace)
#include <cmath>
#include <cstring>
#include <cassert>

// Color component masks and shift values for 32-bit color.
#define PAUSED_MASK32_R  0x00FF0000
#define PAUSED_MASK32_G  0x0000FF00
#define PAUSED_MASK32_B  0x000000FF
#define PAUSED_SHIFT32_R (16+3)
#define PAUSED_SHIFT32_G (8+3)
#define PAUSED_SHIFT32_B (0+3)

// CPU flags.
#include "libcompat/cpuflags.h"

#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__) || defined(__x86_64__))
#define HAVE_MMX 1
#endif

namespace LibGens {

class PausedEffectPrivate
{
	private:
		PausedEffectPrivate();
		~PausedEffectPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		PausedEffectPrivate(const PausedEffectPrivate &);
		PausedEffectPrivate &operator=(const PausedEffectPrivate &);

	public:
		template<typename pixel, uint8_t RBits, uint8_t GBits, uint8_t BBits>
		static inline void T_DoPausedEffect(
			pixel* RESTRICT outScreen,
			unsigned int pxCount);

		template<typename pixel, uint8_t RBits, uint8_t GBits, uint8_t BBits>
		static inline void T_DoPausedEffect(
			pixel* RESTRICT outScreen,
			const pixel* RESTRICT mdScreen,
			unsigned int pxCount);

#ifdef HAVE_MMX
		static inline void DoPausedEffect_32_SSE2(
			uint32_t* RESTRICT outScreen,
			unsigned int pxCount);

		static inline void DoPausedEffect_32_SSE2(
			uint32_t* RESTRICT outScreen,
			const uint32_t* RESTRICT mdScreen,
			unsigned int pxCount);

		static inline void DoPausedEffect_32_MMX(
			uint32_t* RESTRICT outScreen,
			unsigned int pxCount);

		static inline void DoPausedEffect_32_MMX(
			uint32_t* RESTRICT outScreen,
			const uint32_t* RESTRICT mdScreen,
			unsigned int pxCount);
#endif
};

}

#if defined(HAVE_MMX)
// MMX/SSE2-optimized functions.
#define __IN_LIBGENS_PAUSEDEFFECT_CPP__
#define DO_1FB
#include "PausedEffect.x86.inc.cpp"
#undef DO_1FB
#define DO_2FB
#include "PausedEffect.x86.inc.cpp"
#undef DO_2FB
#endif

namespace LibGens {

#define MMASK(bits) ((1 << (bits)) - 1)

// TODO: Use an include file for the C++ version,
// using DO_1FB and DO_2FB?

/**
 * Tint the screen a purple hue to indicate that emulation is paused. [1-FB]
 * @param pixel Type of pixel.
 * @param RBits Number of bits for Red.
 * @param GBits Number of bits for Green.
 * @param BBits Number of bits for Blue.
 * @param outScreen Pointer to the source/destination screen buffer.
 * @param pxCount Pixel count.
 */
template<typename pixel, uint8_t RBits, uint8_t GBits, uint8_t BBits>
inline void PausedEffectPrivate::T_DoPausedEffect(
	pixel* RESTRICT outScreen,
	unsigned int pxCount)
{
	uint8_t r, g, b;
	unsigned int nRG, nB;
	unsigned int monoPx;

	for (; pxCount > 0; pxCount--) {
		// Get the color components.
		r = (uint8_t)((*outScreen >> (GBits + BBits)) & MMASK(RBits)) << (8 - RBits);
		g = (uint8_t)((*outScreen >> BBits) & MMASK(GBits)) << (8 - GBits);
		b = (uint8_t)((*outScreen) & MMASK(BBits)) << (8 - BBits);

		// Convert the color components to grayscale.
		// Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
		// Source: http://en.wikipedia.org/wiki/YCbCr
		// NOTE: Old float code that was more accurate, but slower.
		//monoPx = ((float)r * 0.299f) + ((float)g * 0.587f) + ((float)b * 0.114f);

		// Integer grayscale vector: [0x4D, 0x96, 0x1D]
		// Reference: http://www.asmcommunity.net/forums/topic/?id=19704
		monoPx = (r * 0x4D) + (g * 0x96) + (b * 0x1D);
		monoPx >>= 8;

		// Save the R and G components.
		nRG = (uint8_t)monoPx;
		if (nRG > 0xFF)
			nRG = 0xFF;

		// Left-shift the blue component to tint the image.
		// TODO: Keep the LSB? (bit 0 for 32-bit, 3 for 15-bit/16-bit)
		nB = (nRG << 1);
		if (nB > 0xFF)
			nB = 0xFF;

		// Put the new pixel.
		*outScreen++ =
			((nRG >> (8 - RBits)) << (GBits + BBits)) |
			((nRG >> (8 - GBits)) << (BBits)) |
			((nB  >> (8 - BBits)));
	}
}

/**
 * Tint the screen a purple hue to indicate that emulation is paused. [2-FB]
 * @param pixel Type of pixel.
 * @param RBits Number of bits for Red.
 * @param GBits Number of bits for Green.
 * @param BBits Number of bits for Blue.
 * @param outScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param pxCount Pixel count.
 */
template<typename pixel, uint8_t RBits, uint8_t GBits, uint8_t BBits>
inline void PausedEffectPrivate::T_DoPausedEffect(
	pixel* RESTRICT outScreen,
	const pixel* RESTRICT mdScreen,
	unsigned int pxCount)
{
	uint8_t r, g, b;
	unsigned int nRG, nB;
	unsigned int monoPx;
	
	for (; pxCount > 0; pxCount--) {
		// Get the color components.
		r = (uint8_t)((*mdScreen >> (GBits + BBits)) & MMASK(RBits)) << (8 - RBits);
		g = (uint8_t)((*mdScreen >> BBits) & MMASK(GBits)) << (8 - GBits);
		b = (uint8_t)((*mdScreen) & MMASK(BBits)) << (8 - BBits);
		mdScreen++;

		// Convert the color components to grayscale.
		// Grayscale vector: [0.299 0.587 0.114] (ITU-R BT.601)
		// Source: http://en.wikipedia.org/wiki/YCbCr
		// NOTE: Old float code that was more accurate, but slower.
		//monoPx = ((float)r * 0.299f) + ((float)g * 0.587f) + ((float)b * 0.114f);

		// Integer grayscale vector: [0x4D, 0x96, 0x1D]
		// Reference: http://www.asmcommunity.net/forums/topic/?id=19704
		monoPx = (r * 0x4D) + (g * 0x96) + (b * 0x1D);
		monoPx >>= 8;

		// Save the R and G components.
		nRG = (uint8_t)monoPx;
		if (nRG > 0xFF)
			nRG = 0xFF;

		// Left-shift the blue component to tint the image.
		// TODO: Keep the LSB? (bit 0 for 32-bit, 3 for 15-bit/16-bit)
		nB = (nRG << 1);
		if (nB > 0xFF)
			nB = 0xFF;

		// Put the new pixel.
		*outScreen++ =
			((nRG >> (8 - RBits)) << (GBits + BBits)) |
			((nRG >> (8 - GBits)) << (BBits)) |
			((nB  >> (8 - BBits)));
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
	const unsigned int pxCount = (outScreen->pxPitch() * outScreen->numLines());

	// Render to outScreen.
	switch (outScreen->bpp()) {
		case MdFb::BPP_15:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 5, 5, 5>
				(outScreen->fb16(), pxCount);
			break;
		case MdFb::BPP_16:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 5, 6, 5>
				(outScreen->fb16(), pxCount);
			break;
		case MdFb::BPP_32:
		default:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_SSE2) {
				PausedEffectPrivate::DoPausedEffect_32_SSE2(
					outScreen->fb32(), pxCount);
			} else if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				PausedEffectPrivate::DoPausedEffect_32_MMX(
					outScreen->fb32(), pxCount);
			} else
#endif /* HAVE_MMX */
			{
				PausedEffectPrivate::T_DoPausedEffect<uint32_t, 8, 8, 8>
					(outScreen->fb32(), pxCount);
			}
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

	// Pixel count.
	// TODO: Verify that both framebuffers are the same.
	const unsigned int pxCount = (outScreen->pxPitch() * outScreen->numLines());

	// Render to outScreen.
	switch (outScreen->bpp()) {
		case MdFb::BPP_15:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 5, 5, 5>
				(outScreen->fb16(), mdScreen->fb16(), pxCount);
			break;
		case MdFb::BPP_16:
			PausedEffectPrivate::T_DoPausedEffect<uint16_t, 5, 6, 5>
				(outScreen->fb16(), mdScreen->fb16(), pxCount);
			break;
		case MdFb::BPP_32:
		default:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_SSE2) {
				PausedEffectPrivate::DoPausedEffect_32_SSE2(
					outScreen->fb32(), mdScreen->fb32(), pxCount);
			} else if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				PausedEffectPrivate::DoPausedEffect_32_MMX(
					outScreen->fb32(), mdScreen->fb32(), pxCount);
			} else
#endif /* HAVE_MMX */
			{
				PausedEffectPrivate::T_DoPausedEffect<uint32_t, 8, 8, 8>
					(outScreen->fb32(), mdScreen->fb32(), pxCount);
			}
			break;
	}

	// Unreference the framebuffers.
	outScreen->unref();
	mdScreen->unref();
}

}
