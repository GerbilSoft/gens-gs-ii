/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ARingBuffer.hpp: Audio Ring Buffer class.                               *
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

#ifndef __GENS_QT4_AUDIO_ARINGBUFFER_HPP__
#define __GENS_QT4_AUDIO_ARINGBUFFER_HPP__

#include "libgens/sound/SoundMgr.hpp"

// C includes.
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

// Qt includes.
#include <QtCore/QMutex>

namespace GensQt4
{

class ARingBuffer
{
	public:
		ARingBuffer();
		~ARingBuffer();
		
		/**
		 * reInit(): Reinitialize the Ring Buffer.
		 * @param segSize Segment size. (Number of samples)
		 * @param stereo If true, segments are stereo; otherwise, they're mono.
		 */
		void reInit(int segSize, bool stereo);
		
		int getSegWP(void) const { return (m_segWP / m_segLength); }
		int getSegRP(void) const { return (m_segRP / m_segLength); }
		
		/**
		 * writeLock(): Lock the buffer for writing one segment.
		 * @return Pointer to current write segment.
		 */
		int16_t *writeLock(void);
		
		/**
		 * writeUnlock(): Unlock the buffer and advance the write pointer.
		 */
		void writeUnlock(void);
		
		/**
		 * read(): Read data into the specified output buffer.
		 * @param out Output buffer.
		 * @param samples Samples to read.
		 */
		void read(int16_t *out, int samples);
		
		/**
		 * wpSegWait(): Wait for the read pointer to pass the write pointer.
		 */
		void wpSegWait(void) const
		{
			while (getSegWP() == getSegRP())
			{
				// NOTE: On Gens/GS Win32, I had to remove usleep()
				// due to lag issues. Let's see if that happens here.
				usleep(500);
			}
		}
		
		/**
		 * isBufferEmpty(): Check if the write pointer matches the read pointer.
		 * @return True if it does; false if it doesn't.
		 */
		bool isBufferEmpty(void) const
		{
			int WpRpDiff = (m_segWP - m_segRP);
			
			if (m_segRP_wraparound > m_segWP_wraparound)
			{
				// Read pointer is ahead of the write pointer.
				return true;
			}
			else if (m_segWP_wraparound > m_segRP_wraparound)
			{
				// Write pointer is ahead of the read pointer.
				WpRpDiff += ((m_segWP_wraparound - m_segRP_wraparound) * m_segLength);
			}
			
			if (WpRpDiff <= m_segLength)
				return true;
			
			return false;
		}
		
		static const int NUM_SEGMENTS = 8;
		static const int MAX_SEGMENT_SIZE = LibGens::SoundMgr::MAX_SEGMENT_SIZE;
	
	protected:
		/**
		 * m_buffer[]: Segment buffer.
		 * Stores up to NUM_SEGMENTS segments.
		 */
		int16_t m_buffer[MAX_SEGMENT_SIZE * NUM_SEGMENTS * 2];
		
		// TODO: Allow locking individual segments.
		QMutex m_bufLock;
		
		/**
		 * m_segLength: Length of a segment, in int16_t units.
		 */
		int m_segLength;
		
		/**
		 * m_bufLength: Length of the entire buffer.
		 * This is usually m_segLength * MAX_SEGMENTS.
		 */
		int m_bufLength;
		
		/**
		 * m_stereo: Stereo/Mono setting.
		 */
		bool m_stereo;
		
		/**
		 * Read/Write pointers. (int16_t offsets)
		 * m_segWP: emulator to m_buffer
		 * m_segRP: m_buffer to sound card
		 */
		int m_segWP;
		int m_segRP;
		int m_segWP_wraparound;
		int m_segRP_wraparound;
};

}

#endif /* __GENS_QT4_AUDIO_ARINGBUFFER_HPP__ */
