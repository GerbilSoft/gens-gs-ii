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

// C includes. (C++ namespace)
#include <cstring>

// C++ includes.
#include <algorithm>

namespace LibGens {

/**
 * Write stereo audio to a buffer.
 * This clears the internal audio buffer.
 * @param buf Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
 * @return Number of samples written.
 */
int SoundMgr::writeStereo(int16_t *buf, int samples)
{
	// TODO: MMX/SSE2.
	samples = std::min(samples, ms_SegLength);

	// Source buffer pointers.
	const int32_t *srcL = &ms_SegBufL[0];
	const int32_t *srcR = &ms_SegBufR[0];

	for (int i = samples; i > 0;
	     i--, srcL++, srcR++, buf += 2)
	{
		if (*srcL < -0x8000) {
			*buf = -0x8000;
		} else if (*srcL > 0x7FFF) {
			*buf = 0x7FFF;
		} else {
			*buf = (int16_t)(*srcL);
		}

		if (*srcR < -0x8000) {
			*(buf+1) = -0x8000;
		} else if (*srcR > 0x7FFF) {
			*(buf+1) = 0x7FFF;
		} else {
			*(buf+1) = (int16_t)(*srcR);
		}
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
 * @param buf Destination buffer.
 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
 * @return Number of samples written.
 */
int SoundMgr::writeMono(int16_t *buf, int samples)
{
	// TODO: MMX/SSE2.
	samples = std::min(samples, ms_SegLength);

	// Source buffer pointers.
	const int32_t *srcL = &ms_SegBufL[0];
	const int32_t *srcR = &ms_SegBufR[0];

	for (int i = samples; i > 0;
	     i--, srcL++, srcR++, buf++)
	{
		// NOTE: This will be incorrect if
		// (*srcL + *srcR) >= 2^31.
		// This is highly unlikely, since there's a
		// maximum of 4 (PSG, FM, PCM, PWM) audio chips,
		// which means a worst-case maximum of 0x8000 * 4.
		const int32_t out = ((*srcL + *srcR) >> 1);

		if (out < -0x8000) {
			*buf = -0x8000;
		} else if (out > 0x7FFF) {
			*buf = 0x7FFF;
		} else {
			*buf = (int16_t)out;
		}
	}

	// Clear the segment buffers.
	// These buffers are additive, so if they aren't cleared,
	// we'll end up with static.
	memset(ms_SegBufL, 0, ms_SegLength * sizeof(ms_SegBufL[0]));
	memset(ms_SegBufR, 0, ms_SegLength * sizeof(ms_SegBufL[0]));

	return samples;
}

}
