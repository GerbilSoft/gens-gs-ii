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
#include <cassert>

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

/** SoundMgrPrivate: SSE-optimized functions. **/

#ifdef SOUNDMGR_HAS_MMX
/**
 * Write stereo audio to a buffer. (SSE2-optimized)
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
 */
void SoundMgrPrivate::writeStereo_SSE2(int16_t *dest, int samples)
{
	// samples is clamped to std::min(samples, ms_SegLength)
	// by writeStereo().

	// Source buffer pointers.
	const int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	const int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	// Write 8 samples at once using SSE2.
	assert((uintptr_t)dest % 16 == 0);
	int i = samples;
	for (; i > 7; i -= 8, srcL += 8, srcR += 8, dest += 16) {
		__asm__ (
			"movdqa		(%[srcL]), %%xmm0\n"	// %xmm0 = [L4h | L4l | L3h | L3l | L2h | L2l | L1h | L1l]
			"movdqa		(%[srcR]), %%xmm1\n"	// %xmm1 = [R4h | R4l | R3h | R3l | R2h | R2l | R1h | R1l]
			"movdqa		16(%[srcL]), %%xmm2\n"	// %xmm2 = [L8h | L8l | L7h | L7l | L6h | L6l | L5h | L5l]
			"movdqa		16(%[srcR]), %%xmm3\n"	// %xmm3 = [R8h | R8l | R7h | R7l | R6h | R6l | R5h | R5l]
			// NOTE: On my ThinkPad T60p with Core 2 Duo T7200, the
			// pshufd version is faster than the punpcklwd version, even though
			// agner.org's optimization documents say otherwise.
			// Reference: http://www.agner.org/optimize/instruction_tables.pdf
			// punpcklwd version:
			#if 0
			"packssdw	%%xmm0, %%xmm0\n"	// %xmm0 = [L4  | L3  | L2  | L1  | L4  | L3  | L2  | L1 ]
			"packssdw	%%xmm1, %%xmm1\n"	// %xmm1 = [R4  | R3  | R2  | R1  | R4  | R3  | R2  | R1 ]
			"packssdw	%%xmm2, %%xmm2\n"	// %xmm2 = [L8  | L7  | L6  | L5  | L8  | L7  | L6  | L5 ]
			"packssdw	%%xmm3, %%xmm3\n"	// %xmm3 = [R8  | R7  | R6  | R5  | R8  | R7  | R6  | R5 ]
			"punpcklwd	%%xmm1, %%xmm0\n"	// %xmm0 = [R4  | L4  | R3  | L3  | R2  | L2  | R1  | L1 ]
			"punpcklwd	%%xmm3, %%xmm2\n"	// %xmm2 = [R8  | L8  | R7  | L7  | R6  | L6  | R5  | L5 ]
			#endif
			// pshufd version:
			"packssdw	%%xmm1, %%xmm0\n"		// %xmm0 = [R4  | R3  | R2  | R1  | L4  | L3  | L2  | L1 ]
			"packssdw	%%xmm3, %%xmm2\n"		// %xmm2 = [R8  | R7  | R6  | R5  | L8  | L7  | L6  | L5 ]
			"pshufd		$0xD8, %%xmm0, %%xmm0\n"	// %xmm0 = [R4  | R3  | L4  | L3  | R2  | R1  | L2  | L1 ]
			"pshuflw	$0xD8, %%xmm0, %%xmm0\n"	// %xmm0 = [R4  | R3  | L4  | L3  | R2  | L2  | R1  | L1 ]
			"pshufhw	$0xD8, %%xmm0, %%xmm0\n"	// %xmm0 = [R4  | L4  | R3  | L3  | R2  | L2  | R1  | L1 ]
			"pshufd		$0xD8, %%xmm2, %%xmm2\n"	// %xmm0 = [R8  | R7  | L8  | L7  | R6  | R5  | L6  | L5 ]
			"pshuflw	$0xD8, %%xmm2, %%xmm2\n"	// %xmm0 = [R8  | R7  | L8  | L7  | R6  | L6  | R5  | L5 ]
			"pshufhw	$0xD8, %%xmm2, %%xmm2\n"	// %xmm0 = [R8  | L8  | R7  | L7  | R6  | L6  | R5  | L5 ]
			"movdqa		%%xmm0, (%[dest])\n"
			"movdqa		%%xmm2, 16(%[dest])\n"
			:
			: [srcL] "r" (srcL), [srcR] "r" (srcR), [dest] "r" (dest)
			// FIXME: gcc complains xmm? registers are unknown.
			// May need to compile with -msse...
			//: "xmm0", "xmm1", "xmm2", "xmm3"
			);
	}

	// If the buffer size isn't a multiple of 8 samples,
	// write the remaining samples normally.
        for (; i > 0; i--, srcL++, srcR++, dest += 2) {
                *(dest+0) = clamp(*srcL);
                *(dest+1) = clamp(*srcR);
        }
}

/**
 * Write monaural audio to a buffer. (SSE2-optimized)
 * @param dest Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
 */
void SoundMgrPrivate::writeMono_SSE2(int16_t *dest, int samples)
{
	// samples is clamped to std::min(samples, ms_SegLength)
	// by writeStereo().

	// Source buffer pointers.
	const int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	const int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	// Write 8 samples at once using SSE2.
	assert((uintptr_t)dest % 16 == 0);
	int i = samples;
	for (; i > 7; i -= 8, srcL += 8, srcR += 8, dest += 8) {
		__asm__ (
			"movdqa		(%[srcL]), %%xmm0\n"	// %xmm0 = [L4h | L4l | L3h | L3l | L2h | L2l | L1h | L1l]
			"movdqa		(%[srcR]), %%xmm1\n"	// %xmm1 = [R4h | R4l | R3h | R3l | R2h | R2l | R1h | R1l]
			"movdqa		16(%[srcL]), %%xmm2\n"	// %xmm2 = [L8h | L8l | L7h | L7l | L6h | L6l | L5h | L5l]
			"movdqa		16(%[srcR]), %%xmm3\n"	// %xmm3 = [R8h | R8l | R7h | R7l | R6h | R6l | R5h | R5l]
			// NOTE: This may overflow if samples are >= 2^30,
			// but that shouldn't happen except in unit tests.
			// TODO: Use pavgw after packing? (Unsigned, thoguh...)
			"paddd		%%xmm1, %%xmm0\n"
			"paddd		%%xmm3, %%xmm2\n"
			// TODO: Add 1 to match SSE2 'pavgw'?
			"psrad		$1, %%xmm0\n"		// %xmm0 = [M4h | M4l | M3h | M3l | M2h | M2l | M1h | M1l]
			"psrad		$1, %%xmm2\n"		// %xmm2 = [M8h | M8l | M7h | M7l | M6h | M6l | M5h | M5l]
			"packssdw	%%xmm0, %%xmm0\n"	// %xmm0 = [M4  | M3  | M2  | M1  | M4  | M3  | M2  | M1 ]
			"packssdw	%%xmm2, %%xmm2\n"	// %xmm2 = [M8  | M7  | M6  | M5  | M8  | M7  | M6  | M5 ]
			// TODO: Combine %%xmm0 and %%xmm1 into a single register before writing?
			"movq		%%xmm0, (%[dest])\n"
			"movq		%%xmm2, 8(%[dest])\n"
			:
			: [srcL] "r" (srcL), [srcR] "r" (srcR), [dest] "r" (dest)
			// FIXME: gcc complains xmm? registers are unknown.
			// May need to compile with -msse...
			//: "xmm0", "xmm1", "xmm2", "xmm3"
			);
	}

	// If the buffer size isn't a multiple of 8 samples,
	// write the remaining samples normally.
        for (; i > 0; i--, srcL++, srcR++, dest++) {
		// Combine the L and R samples into one sample.
		const int32_t out = ((*srcL + *srcR) >> 1);
		*dest = clamp(out);
        }
}
#endif /* SOUNDMGR_HAS_MMX */

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

	// Write 4 samples at once using MMX.
	int i = samples;
	for (; i > 3; i -= 4, srcL += 4, srcR += 4, dest += 8) {
		__asm__ (
			// NOTE: pshufw is NOT an MMX instruction.
			// We have to use movd and shift to get the words into place.
			// FIXME: Optimize it using movq somehow.
			// (Attempted a movq optimization, and it ended up being slower...)
			"movd		(%[srcL]), %%mm0\n"	// %mm0 = [ 0  |  0  | L1h | L1l]
			"movd		(%[srcR]), %%mm1\n"	// %mm1 = [ 0  |  0  | R1h | R1l]
			"psllq		$32, %%mm1\n"		// %mm1 = [R1h | R1l |  0  |  0 ]
			"por		%%mm1, %%mm0\n"		// %mm0 = [R1h | R1l | L1h | L1l]
			"packssdw	%%mm0, %%mm0\n"		// %mm0 = [R1  | L1  | R1  | L1 ]

			"movd		4(%[srcL]), %%mm2\n"	// %mm2 = [ 0  |  0  | L2h | L2l]
			"movd		4(%[srcR]), %%mm3\n"	// %mm3 = [ 0  |  0  | R2h | R2l]
			"psllq		$32, %%mm3\n"		// %mm3 = [R2h | R2l |  0  |  0 ]
			"por		%%mm3, %%mm2\n"		// %mm2 = [R2h | R2l | L2h | L2l]
			"packssdw	%%mm2, %%mm2\n"		// %mm2 = [R2  | L2  | R2  | L2 ]

			"movd		8(%[srcL]), %%mm4\n"	// %mm4 = [ 0  |  0  | L3h | L3l]
			"movd		8(%[srcR]), %%mm5\n"	// %mm5 = [ 0  |  0  | R3h | R3l]
			"psllq		$32, %%mm5\n"		// %mm5 = [R3h | R3l |  0  |  0 ]
			"por		%%mm5, %%mm4\n"		// %mm4 = [R3h | R3l | L3h | L3l]
			"packssdw	%%mm4, %%mm4\n"		// %mm4 = [R3  | L3  | R3  | L3 ]

			"movd		12(%[srcL]), %%mm6\n"	// %mm6 = [ 0  |  0  | L4h | L4l]
			"movd		12(%[srcR]), %%mm7\n"	// %mm7 = [ 0  |  0  | R4h | R4l]
			"psllq		$32, %%mm7\n"		// %mm7 = [R4h | R4l |  0  |  0 ]
			"por		%%mm7, %%mm6\n"		// %mm6 = [R4h | R4l | L4h | L4l]
			"packssdw	%%mm6, %%mm6\n"		// %mm6 = [R4  | L4  | R4  | L4 ]

			"movd		%%mm0, (%[dest])\n"
			"movd		%%mm2, 4(%[dest])\n"
			"movd		%%mm4, 8(%[dest])\n"
			"movd		%%mm6, 12(%[dest])\n"
			:
			: [srcL] "r" (srcL), [srcR] "r" (srcR), [dest] "r" (dest)
			// FIXME: gcc complains mm? registers are unknown.
			// May need to compile with -mmmx...
			//: "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
			);
	}

	// Reset the FPU state.
	__asm__ ("emms");

	// If the buffer size isn't a multiple of 4 samples,
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

	// Write 4 samples at once using MMX.
	int i = samples;
	for (; i > 3; i -= 4, srcL += 4, srcR += 4, dest += 4) {
		__asm__ (
			// NOTE: Add/shift may overflow if samples are >= 2^30,
			// but that shouldn't happen except in unit tests.
			// TODO: Add 1 before shifting to match SSE2 'pavgw'?
			"movq		(%[srcL]), %%mm0\n"	// %mm0 = [L2h | L2l | L1h | L1l]
			"movq		(%[srcR]), %%mm1\n"	// %mm1 = [R2h | R2l | R1h | R1l]
			"movq		8(%[srcL]), %%mm2\n"	// %mm2 = [L4h | L3l | L2h | L2l]
			"paddd		%%mm1, %%mm0\n"
			"movq		8(%[srcR]), %%mm3\n"	// %mm3 = [R4h | R4l | R3h | R3l]
			"psrad		$1, %%mm0\n"		// %mm0 = [M2h | M2l | M1h | M1l]
			"paddd		%%mm3, %%mm2\n"
			"packssdw	%%mm0, %%mm0\n"		// %mm0 = [M2  | M1  | M2  | M1 ]
			"psrad		$1, %%mm2\n"		// %mm2 = [M4h | M4l | M3h | M3l]
			"movd		%%mm0, (%[dest])\n"
			"packssdw	%%mm2, %%mm2\n"		// %mm2 = [M4  | M3  | M4  | M3 ]
			// TODO: Combine %%mm0 and %%mm2 so we only have to do one movq?
			"movd		%%mm2, 4(%[dest])\n"
			:
			: [srcL] "r" (srcL), [srcR] "r" (srcR), [dest] "r" (dest)
			// FIXME: gcc complains mm? registers are unknown.
			// May need to compile with -mmmx...
			//: "mm0", "mm1", "mm2", "mm3"
		);
	}

	// Reset the FPU state.
	__asm__ ("emms");

	// If the buffer size isn't a multiple of 4 samples,
	// write the remaining samples normally.
        for (; i > 0; i--, srcL++, srcR++, dest++) {
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
	if (CPU_Flags & MDP_CPUFLAG_X86_SSE2) {
		SoundMgrPrivate::writeStereo_SSE2(dest, samples);
	} else if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
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
	if (CPU_Flags & MDP_CPUFLAG_X86_SSE2) {
		SoundMgrPrivate::writeMono_SSE2(dest, samples);
	} else if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
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
