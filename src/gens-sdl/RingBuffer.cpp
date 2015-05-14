/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * RingBuffer.cpp: Ring buffer class.                                      *
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

#include "RingBuffer.hpp"

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

namespace GensSdl {

#include <stdio.h>
/**
 * Initialize a RingBuffer.
 * @param samples Number of 16-bit stereo samples to allocate.
 */
RingBuffer::RingBuffer(unsigned int samples)
	: m_i(0)
	, m_s(0)
{
	m_size = samples * 4;
	assert(m_size <= sizeof(m_data));
	if (m_size > sizeof(m_data))
		m_size = sizeof(m_data);

	// Clear the data buffer.
	memset(&m_data, 0, sizeof(m_data));
}

RingBuffer::~RingBuffer()
{
	// TODO
}

/**
 * Write/copy data into a circular buffer.
 * @param src Buffer to copy from.
 * @param size Size of src.
 * @return Number of bytes copied.
 */
unsigned int RingBuffer::write(const uint8_t *src, unsigned int size)
{
	unsigned int j, k;

	if (size > m_size) {
		src += (size - m_size);
		size = m_size;
	}
	k = (m_size - m_s);
	j = ((m_i + m_s) % m_size);
	if (size > k) {
		m_i = ((m_i + (size - k)) % m_size);
		m_s = m_size;
	} else {
		m_s += size;
	}
	k = (m_size - j);
	if (k >= size) {
		memcpy(&m_data.u8[j], src, size);
	} else {
		memcpy(&m_data.u8[j], src, k);
		memcpy(&m_data.u8[0], &src[k], (size - k));
	}
	return size;
}

/**
 * Read bytes out of a circular buffer.
 * @param dst Destination buffer.
 * @param size Maximum number of bytes to copy to dst.
 * @return Number of bytes copied.
 */
unsigned int RingBuffer::read(uint8_t *dst, unsigned int size)
{
	if (size > m_s) {
		size = m_s;
	}
	if ((m_i + size) > m_size) {
		unsigned int k = (m_size - m_i);
		memcpy(&dst[0], &m_data.u8[(m_i)], k);
		memcpy(&dst[k], &m_data.u8[0], (size - k));
	} else {
		memcpy(&dst[0], &m_data.u8[(m_i)], size);
	}
	m_i = ((m_i + size) % m_size);
	m_s -= size;
	return size;
}

/**
 * Clear the buffer.
 */
void RingBuffer::clear(void)
{
	m_i = 0;
	m_s = 0;
	// TODO: Is this clear necessary?
	memset(&m_data, 0, sizeof(m_data));
}

}
