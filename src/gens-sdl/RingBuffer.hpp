/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * RingBuffer.hpp: Ring buffer class.                                      *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __GENS_SDL_RINGBUFFER_HPP__
#define __GENS_SDL_RINGBUFFER_HPP__

#include <stdint.h>

namespace GensSdl {

class RingBuffer
{
	public:
		/**
		 * Initialize a RingBuffer.
		 * @param samples Number of 16-bit stereo samples to allocate.
		 */
		RingBuffer(unsigned int samples);

		~RingBuffer();

		/**
		 * Write/copy data into a circular buffer.
		 * @param src Buffer to copy from.
		 * @param size Size of src.
		 * @return Number of bytes copied.
		 */
		unsigned int write(const uint8_t *src, unsigned int size);

		/**
		 * Read bytes out of a circular buffer.
		 * @param dst Destination buffer.
		 * @param size Maximum number of bytes to copy to dst.
		 * @return Number of bytes copied.
		 */
		unsigned int read(uint8_t *dst, unsigned int size);

	protected:
		unsigned int m_i;	// Data start index.
		unsigned int m_s;	// Data size, in bytes.

		unsigned int m_size;	// Buffer size, in bytes.

		// Data buffer.
		// Stores up to 32,768 16-bit samples.
		// (16,384 16-bit samples in stereo.)
		// TODO: Does it need to be larger?
		union {
			uint8_t u8[65536];
			int16_t i16[32768];
		} m_data;
};

}

#endif /* __GENS_SDL_RINGBUFFER_HPP__ */
