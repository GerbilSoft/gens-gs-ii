/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SoundMgr.hpp: Sound manager. (PRIVATE CLASS)                            *
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

#ifndef LIBGENS_SOUND_SOUNDMGR_P_HPP__
#define LIBGENS_SOUND_SOUNDMGR_P_HPP__

// NOTE: We're implementing the MMX and SSE2 code
// using GNU inline assembler *only*.
#if defined(__GNUC__) && \
    (defined(__i386__) || defined(__amd64__))
#define SOUNDMGR_HAS_MMX 1
#endif

namespace LibGens {

// SoundMgrPrivate
class SoundMgrPrivate
{
	private:
		SoundMgrPrivate() { }
		~SoundMgrPrivate() { }

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		SoundMgrPrivate(const SoundMgrPrivate &);
		SoundMgrPrivate &operator=(const SoundMgrPrivate &);

	public:
		// Segment length.
		static int CalcSegLength(int rate, bool isPal);

		static int rate;
		static bool isPal;

	public:
#ifdef SOUNDMGR_HAS_MMX
		/**
		 * Write stereo audio to a buffer. (SSE2-optimized)
		 * @param dest Destination buffer.
		 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
		 */
		static void writeStereo_SSE2(int16_t *dest, int samples);

		/**
		 * Write monaural audio to a buffer. (SSE2-optimized)
		 * @param dest Destination buffer.
		 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
		 */
		static void writeMono_SSE2(int16_t *dest, int samples);

		/**
		 * Write stereo audio to a buffer. (MMX-optimized)
		 * @param dest Destination buffer.
		 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
		 */
		static void writeStereo_MMX(int16_t *dest, int samples);

		/**
		 * Write monaural audio to a buffer. (MMX-optimized)
		 * @param dest Destination buffer.
		 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
		 */
		static void writeMono_MMX(int16_t *dest, int samples);
#endif /* SOUNDMGR_HAS_MMX */

		/**
		 * Write stereo audio to a buffer.
		 * @param dest Destination buffer.
		 * @param samples Number of samples in the buffer. (1 sample == 4 bytes)
		 */
		static void writeStereo_noasm(int16_t *dest, int samples);

		/**
		 * Write monaural audio to a buffer.
		 * @param dest Destination buffer.
		 * @param samples Number of samples in the buffer. (1 sample == 2 bytes)
		 */
		static void writeMono_noasm(int16_t *dest, int samples);
};

}

#endif /* LIBGENS_SOUND_SOUNDMGR_P_HPP__ */
