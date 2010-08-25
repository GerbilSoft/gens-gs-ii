/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ARingBuffer.cpp: Audio Ring Buffer class.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "ARingBuffer.hpp"

// C includes.
#include <string.h>

namespace GensQt4
{

ARingBuffer::ARingBuffer()
{
	// Set the segment and buffer lengths to 0.
	// NOTE: ARingBuffer::reInit() MUST be called before using the ring buffer!
	// Otherwise, SIGFPE will occur.
	m_segLength = 0;
	
	// Clear the segment pointers.
	m_segWP = 0;
	m_segRP = 0;
	m_segRP_minor = 0;
	
	// TODO: Dynamically allocate m_buffer?
}

ARingBuffer::~ARingBuffer()
{
	// TODO
}


/**
 * reInit(): Reinitialize the Ring Buffer.
 * @param segSize Segment size. (Number of samples)
 * @param stereo If true, segments are stereo; otherwise, they're mono.
 */
void ARingBuffer::reInit(int segSize, bool stereo)
{
	// Calculate the segment and buffer lengths.
	m_segLength = (segSize * (stereo ? 2 : 1));
	m_stereo = stereo;
	
	// Clear the segment buffer.
	memset(m_buffer, 0x00, sizeof(m_buffer));
	
	// Clear the segment pointers.
	m_segWP = 0;
	m_segRP = 0;
	m_segRP_minor = 0;
}


/**
 * writeLock(): Lock the buffer for writing one segment.
 * @return Pointer to current write segment.
 */
int16_t *ARingBuffer::writeLock(void)
{
	// Lock the buffer segment.
	m_bufLock[m_segWP].lock();
	
	// Return a pointer to the buffer at the current WP.
	return &m_buffer[m_segWP][0];
}


/**
 * writeUnlock(): Unlock the buffer and advance the write pointer.
 */
void ARingBuffer::writeUnlock(void)
{
	// Unlock the buffer segment.
	m_bufLock[m_segWP].unlock();
	
	// Advance the WP to the next segment.
	m_segWP++;
	m_segWP %= NUM_SEGMENTS;
}


/**
 * read(): Read data into the specified output buffer.
 * @param out Output buffer.
 * @param samples Samples to read.
 */
void ARingBuffer::read(int16_t *out, int samples)
{
	// Lock the current read buffer segment.
	m_bufLock[m_segRP].lock();
	
	if (m_stereo)
		samples *= 2;
	
	// TODO: Optimize this!
	for (; samples > 0; samples--)
	{
		*out++ = m_buffer[m_segRP][m_segRP_minor];
		
		// Next sample.
		m_segRP_minor++;
		if (m_segRP_minor >= m_segLength)
		{
			// Next segment.
			m_bufLock[m_segRP].unlock();
			m_segRP++;
			m_segRP %= NUM_SEGMENTS;
			m_segRP_minor = 0;
			m_bufLock[m_segRP].lock();
		}
	}
	
	// Unlock the current read buffer segment.
	m_bufLock[m_segRP].unlock();
}

}
