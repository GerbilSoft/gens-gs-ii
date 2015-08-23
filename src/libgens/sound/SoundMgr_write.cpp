/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SoundMgr_write.cpp: Sound manager: Audio Write functions.               *
 * Converts between the internal audio buffer and standard 16-bit audio.   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2015 by David Korth                                  *
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

#include "SoundMgr.hpp"
#include "Util/cpuflags.h"

// C includes. (C++ namespace)
#include <cstring>

// C++ includes.
#include <algorithm>

#include "SoundMgr_p.hpp"
namespace LibGens {

/**
 * Clamp a 32-bit sample to 16-bit.
 * @param sample 32-bit sample.
 * @return Clamped 16-bit sample.
 */
static inline int16_t clamp(int32_t sample)
{
	// TODO: Is there a faster way to clamp to 16-bit?
	if (sample < -0x8000) {
		return -0x8000;
	} else if (sample > 0x7FFF) {
		return 0x7FFF;
	}
	return (int16_t)sample;
}

/** SoundMgrPrivate: MMX-optimized functions. **/

#ifdef SOUNDMGR_HAS_MMX
/**
 * Write stereo audio to a buffer. (MMX-optimized)
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
 */
void SoundMgrPrivate::writeStereo_MMX(int16_t *dest, int samples)
{
	// samples is clamped to std::min(samples, ms_SegLength)
	// by writeStereo().

	// Source buffer pointers.
	const int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	const int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	// Load the shift value.
	__asm__ (
		"movl	$32, %%eax\n"
		"movd	%%eax, %%mm5\n"
		: // output
		: // input
		: "eax" // clobber
		);

	// Write two samples at once using MMX.
	// TODO: Do 4 samples at once?
	int i = samples;
	for (; i > 1; i -= 2, srcL += 2, srcR += 2, dest += 4) {
		__asm__ (
			"movq		(%[srcL]), %%mm0\n"	// %mm0 = [L2h | L2l | L1h | L1l]
			"movq           (%[srcR]), %%mm1\n"     // %mm1 = [R2h | R2l | R1h | R1l]
			"packssdw	%%mm1, %%mm0\n"		// %mm0 = [R2  | R1  | L2  | L1 ]
			// TODO: Is it faster to use a separate %mm2 for dest here?
			"pshufw		$0xD8, %%mm0, %%mm0\n"	// %mm0 = [R2  | L2  | R1  | L1 ]
			"movq           %%mm0, (%[dest])\n"
			:
			: [srcL] "r" (srcL), [srcR] "r" (srcR), [dest] "r" (dest)
			// FIXME: gcc complains mm0/mm1 are unknown.
			// May need to compile with -mmmx...
			//: "mm0", "mm1"
			);
	}

	// Reset the FPU state.
	__asm__ ("emms");

	// If the buffer size isn't a multiple of two samples,
	// write the remaining samples normally.
        for (; i > 0; i--, srcL++, srcR++, dest += 2) {
                *(dest+0) = clamp(*srcL);
                *(dest+1) = clamp(*srcR);
        }
}

/**
 * Write monaural audio to a buffer. (MMX-optimized)
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
 */
void SoundMgrPrivate::writeMono_MMX(int16_t *dest, int samples)
{
	// samples is clamped to std::min(samples, ms_SegLength)
	// by writeMono().

	// Source buffer pointers.
	const int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	const int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	// Write two samples at once using MMX.
	// TODO: Do 4 samples at once?
	int i = samples;
	for (; i > 1; i -= 2, srcL += 2, srcR += 2, dest += 2) {
		__asm__ (
			"movq		(%[srcL]), %%mm0\n"	// %mm0 = [L2h | L2l | L1h | L1l]
			"movq		(%[srcR]), %%mm1\n"	// %mm1 = [R2h | R2l | R1h | R1l]
			// NOTE: This may overflow if samples are >= 2^30,
			// but that shouldn't happen except in unit tests.
			"paddd		%%mm1, %%mm0\n"
			// TODO: Add 1 to match SSE2 'pavgw'?
			"psrad		$1, %%mm0\n"		// %mm0 = [M2h | M2l | M1h | M1l]
			"packssdw	%%mm0, %%mm0\n"		// %mm0 = [M2  | M1  | M2  | M1 ]
			"movd		%%mm0, (%[dest])\n"
			:
			: [srcL] "r" (srcL), [srcR] "r" (srcR), [dest] "r" (dest)
			// FIXME: gcc complains mm0/mm1 are unknown.
			// May need to compile with -mmmx...
			//: "mm0", "mm1"
		);
	}

	// Reset the FPU state.
	__asm__ ("emms");

	// If the buffer size isn't a multiple of two samples,
	// write the remaining samples normally.
        for (; i > 0; i--, srcL++, srcR++, dest += 2) {
		// Combine the L and R samples into one sample.
		const int32_t out = ((*srcL + *srcR) >> 1);
		*dest = clamp(out);
        }
}
#endif /* SOUNDMGR_HAS_MMX */

/** SoundMgrPrivate: Non-optimized functions. **/

/**
 * Write stereo audio to a buffer.
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
 */
void SoundMgrPrivate::writeStereo_noasm(int16_t *dest, int samples)
{
	// samples is clamped to std::min(samples, ms_SegLength)
	// by writeStereo().

	// Source buffer pointers.
	const int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	const int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	for (int i = samples; i > 0;
	     i--, srcL++, srcR++, dest += 2)
	{
		*(dest+0) = clamp(*srcL);
		*(dest+1) = clamp(*srcR);
	}
}

/**
 * Write monaural audio to a buffer.
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
 */
void SoundMgrPrivate::writeMono_noasm(int16_t *dest, int samples)
{
	// samples is clamped to std::min(samples, ms_SegLength)
	// by writeMono().

	// Source buffer pointers.
	const int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	const int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	for (int i = samples; i > 0;
	     i--, srcL++, srcR++, dest++)
	{
		// NOTE: This will be incorrect if
		// (*srcL + *srcR) >= 2^31.
		// This is highly unlikely, since there's a
		// maximum of 4 (PSG, FM, PCM, PWM) audio chips,
		// which means a worst-case maximum of 0x8000 * 4.
		const int32_t out = ((*srcL + *srcR) >> 1);
		*dest = clamp(out);
	}
}

/** SoundMgr **/

/**
 * Write stereo audio to a buffer.
 * This clears the internal audio buffer.
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
 * @return Number of samples written.
 */
int SoundMgr::writeStereo(int16_t *dest, int samples)
{
	samples = std::min(samples, ms_SegLength);
#ifdef SOUNDMGR_HAS_MMX
	// TODO: SSE2
	if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
		SoundMgrPrivate::writeStereo_MMX(dest, samples);
	} else
#endif /* SOUNDMGR_HAS_MMX */
	{
		SoundMgrPrivate::writeStereo_noasm(dest, samples);
	}

	// Clear the segment buffers.
	// These buffers are additive, so if they aren't cleared,
	// we'll end up with static.
	memset(ms_SegBufL, 0, ms_SegLength * sizeof(ms_SegBufL[0]));
	memset(ms_SegBufR, 0, ms_SegLength * sizeof(ms_SegBufL[0]));

	return samples;
}

/**
 * Write monaural audio to a buffer.
 * This clears the internal audio buffer.
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
 * @return Number of samples written.
 */
int SoundMgr::writeMono(int16_t *dest, int samples)
{
	samples = std::min(samples, ms_SegLength);
#ifdef SOUNDMGR_HAS_MMX
	// TODO: SSE2
	if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
		SoundMgrPrivate::writeMono_MMX(dest, samples);
	} else
#endif /* SOUNDMGR_HAS_MMX */
	{
		SoundMgrPrivate::writeMono_noasm(dest, samples);
	}

	// Clear the segment buffers.
	// These buffers are additive, so if they aren't cleared,
	// we'll end up with static.
	memset(ms_SegBufL, 0, ms_SegLength * sizeof(ms_SegBufL[0]));
	memset(ms_SegBufR, 0, ms_SegLength * sizeof(ms_SegBufL[0]));

	return samples;
}

}
