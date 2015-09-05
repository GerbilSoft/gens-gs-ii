/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * FastBlur.cpp: Fast Blur effect.                                         *
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

#include "FastBlur.hpp"
#include "Util/MdFb.hpp"
#include "libcompat/cpuflags.h"

// C includes.
#include <stdlib.h>
#include <stdint.h>

// C includes. (C++ namespace)
#include <cassert>
#include <cmath>
#include <cstring>

#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__) || defined(__x86_64__))
#define HAVE_MMX
#endif

namespace LibGens {

class FastBlurPrivate
{
	private:
		FastBlurPrivate();
		~FastBlurPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		FastBlurPrivate(const FastBlurPrivate &);
		FastBlurPrivate &operator=(const FastBlurPrivate &);

	public:
		// Mask constants.
		static const uint16_t MASK_DIV2_15 = 0x3DEF;
		static const uint16_t MASK_DIV2_16 = 0x7BCF;

		static void DoFastBlur_16(
			uint16_t* RESTRICT outScreen,
			unsigned int pxCount,
			uint16_t mask);
		static void DoFastBlur_16(
			uint16_t* RESTRICT outScreen,
			const uint16_t* RESTRICT mdScreen,
			unsigned int pxCount,
			uint16_t mask);

		static void DoFastBlur_32(
			uint32_t* RESTRICT outScreen,
			unsigned int pxCount);
		static void DoFastBlur_32(
			uint32_t* RESTRICT outScreen,
			const uint32_t* RESTRICT mdScreen,
			unsigned int pxCount);

#ifdef HAVE_MMX
		static const uint32_t MASK_DIV2_15_MMX[2];
		static const uint32_t MASK_DIV2_16_MMX[2];

#if 0
// FIXME: SSE2 Fast Blur is generally slow due to
// unaligned access requirements.
		static void DoFastBlur_32_SSE2(
			uint32_t* RESTRICT outScreen,
			unsigned int pxCount);
		static void DoFastBlur_32_SSE2(
			uint32_t* RESTRICT outScreen,
			const uint32_t* RESTRICT mdScreen,
			unsigned int pxCount);
#endif

		static void DoFastBlur_16_MMX(
			uint16_t* RESTRICT outScreen,
			unsigned int pxCount,
			const uint32_t *mask);
		static void DoFastBlur_16_MMX(
			uint16_t* RESTRICT outScreen,
			const uint16_t* RESTRICT mdScreen,
			unsigned int pxCount,
			const uint32_t *mask);

		static void DoFastBlur_32_MMX(
			uint32_t* RESTRICT outScreen,
			unsigned int pxCount);
		static void DoFastBlur_32_MMX(
			uint32_t* RESTRICT outScreen,
			const uint32_t* RESTRICT mdScreen,
			unsigned int pxCount);
#endif /* HAVE_MMX */
};

#ifdef HAVE_MMX
const uint32_t FastBlurPrivate::MASK_DIV2_15_MMX[2] = {0x3DEF3DEF, 0x3DEF3DEF};
const uint32_t FastBlurPrivate::MASK_DIV2_16_MMX[2] = {0x7BCF7BCF, 0x7BCF7BCF};
#endif /* HAVE_MMX */

}

// Generic versions.
#define __IN_LIBGENS_FASTBLUR_CPP__
#define DO_1FB
#include "FastBlur.generic.inc.cpp"
#undef DO_1FB
#define DO_2FB
#include "FastBlur.generic.inc.cpp"
#undef DO_2FB

// MMX/SSE2-optimized versions.
#ifdef HAVE_MMX
#define DO_1FB
#include "FastBlur.x86.inc.cpp"
#undef DO_1FB
#define DO_2FB
#include "FastBlur.x86.inc.cpp"
#undef DO_2FB
#endif /* HAVE_MMX */

namespace LibGens {

/**
 * Apply a Fast Blur effect to the screen buffer.
 * @param outScreen Source and destination screen.
 */
void FastBlur::DoFastBlur(MdFb* RESTRICT outScreen)
{
	// Reference the framebuffer.
	outScreen->ref();
	const unsigned int pxCount = (outScreen->pxPitch() * outScreen->numLines());

	switch (outScreen->bpp()) {
		case MdFb::BPP_15:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				FastBlurPrivate::DoFastBlur_16_MMX(
					outScreen->fb16(), pxCount,
					FastBlurPrivate::MASK_DIV2_15_MMX);
			} else
#endif /* HAVE_MMX */
			{
				FastBlurPrivate::DoFastBlur_16(
					outScreen->fb16(), pxCount,
					FastBlurPrivate::MASK_DIV2_15);
			}
			break;

		case MdFb::BPP_16:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				FastBlurPrivate::DoFastBlur_16_MMX(
					outScreen->fb16(), pxCount,
					FastBlurPrivate::MASK_DIV2_16_MMX);
			} else
#endif /* HAVE_MMX */
			{
				FastBlurPrivate::DoFastBlur_16(
					outScreen->fb16(), pxCount,
					FastBlurPrivate::MASK_DIV2_16);
			}
			break;

		case MdFb::BPP_32:
		default:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				FastBlurPrivate::DoFastBlur_32_MMX(
					outScreen->fb32(), pxCount);
			} else
#endif /* HAVE_MMX */
			{
				FastBlurPrivate::DoFastBlur_32(
					outScreen->fb32(), pxCount);
			}
			break;
	}

	// Unreference the framebuffer.
	outScreen->unref();
}

/**
 * Apply a Fast Blur effect to the screen buffer.
 * @param outScreen Destination screen.
 * @param mdScreen Source screen.
 */
void FastBlur::DoFastBlur(MdFb* RESTRICT outScreen, const MdFb* RESTRICT mdScreen)
{
	// Reference the framebuffers.
	outScreen->ref();
	mdScreen->ref();

	// Set outScreen's bpp to match mdScreen.
	outScreen->setBpp(mdScreen->bpp());

	// Pixel count.
	// TODO: Verify that both framebuffers are the same.
	const unsigned int pxCount = (outScreen->pxPitch() * outScreen->numLines());

	switch (outScreen->bpp()) {
		case MdFb::BPP_15:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				FastBlurPrivate::DoFastBlur_16_MMX(
					outScreen->fb16(), mdScreen->fb16(), pxCount,
					FastBlurPrivate::MASK_DIV2_15_MMX);
			} else
#endif /* HAVE_MMX */
			{
				FastBlurPrivate::DoFastBlur_16(
					outScreen->fb16(), mdScreen->fb16(),
					pxCount, FastBlurPrivate::MASK_DIV2_15);
			}
			break;

		case MdFb::BPP_16:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				FastBlurPrivate::DoFastBlur_16_MMX(
					outScreen->fb16(), mdScreen->fb16(), pxCount,
					FastBlurPrivate::MASK_DIV2_16_MMX);
			} else
#endif /* HAVE_MMX */
			{
				FastBlurPrivate::DoFastBlur_16(
					outScreen->fb16(), mdScreen->fb16(),
					pxCount, FastBlurPrivate::MASK_DIV2_16);
			}
			break;

		case MdFb::BPP_32:
		default:
#ifdef HAVE_MMX
			if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
				FastBlurPrivate::DoFastBlur_32_MMX(
					outScreen->fb32(), mdScreen->fb32(), pxCount);
			}
#endif /* HAVE_MMX */
			{
				FastBlurPrivate::DoFastBlur_32(
					outScreen->fb32(), mdScreen->fb32(), pxCount);
			}
			break;
	}

	// Unreference the framebuffers.
	outScreen->unref();
	mdScreen->unref();
}

}
