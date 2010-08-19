/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensPortAudio.hpp: PortAudio audio backend.                             *
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

#ifndef __GENS_QT4_AUDIO_PORTAUDIO_HPP__
#define __GENS_QT4_AUDIO_PORTAUDIO_HPP__

// C includes.
#include <stdint.h>

// PortAudio.
#include "portaudio.h"

// Qt includes.
#include <QtCore/QMutex>

// TODO: Move this somewhere else!
#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))
#define HAVE_MMX
#endif

namespace GensQt4
{

class GensPortAudio
{
	public:
		GensPortAudio();
		~GensPortAudio();
		
		inline bool isOpen(void) const { return m_open; }
		
		void open(void);
		void close(void);
		
		/**
		 * Properties.
		 */
		inline int rate(void) const { return m_rate; }
		void setRate(int newRate);
		
		inline bool isStereo(void) const { return m_stereo; }
		void setStereo(bool newStereo);
		
		/**
		 * write(): Write the current segment to the audio buffer.
		 * @return 0 on success; non-zero on error.
		 */
		int write(void);
	
	protected:
		bool m_open;	// True if PortAudio is initialized.
		
		// Static PortAudio callback function.
		static int GensPaCallback(const void *inputBuffer, void *outputBuffer,
					  unsigned long framesPerBuffer,
					  const PaStreamCallbackTimeInfo *timeInfo,
					  PaStreamCallbackFlags statusFlags,
					  void *userData)
		{
			// TODO: Verify userData.
			return ((GensPortAudio*)userData)->gensPaCallback(
						inputBuffer, outputBuffer,
						framesPerBuffer,
						timeInfo, statusFlags);
		}
		
		// PortAudio callback function.
		int gensPaCallback(const void *inputBuffer, void *outputBuffer,
				   unsigned long framesPerBuffer,
				   const PaStreamCallbackTimeInfo *timeInfo,
				   PaStreamCallbackFlags statusFlags);
		
		// PortAudio stream.
		PaStream *m_stream;
		
		// Number of segments to buffer.
		static const int SEGMENTS_TO_BUFFER = 8;
		
		// Audio buffer.
		// Allocate 8 segments worth of stereo samples.
		int16_t m_buf[882*SEGMENTS_TO_BUFFER*2];
		
		// Buffer length, position, and lock.
		unsigned int m_bufLen;
		unsigned int m_bufPos;
		QMutex m_mtxBuf;
		
		// Audio settings.
		int m_rate;
		bool m_stereo;
		
		// Sample size. (Calculated on open().)
		int m_sampleSize;
		
		/** Internal audio write functions. **/
		
		/**
		 * writeStereo(): Write the current segment to the audio buffer. (Stereo output)
		 * @return 0 on success; non-zero on error.
		 */
		int writeStereo(void);
		
		/**
		 * writeMono(): Write the current segment to the audio buffer. (Monaural output)
		 * @return 0 on success; non-zero on error.
		 */
		int writeMono(void);
		
#ifdef HAVE_MMX
		/**
		 * writeStereoMMX(): Write the current segment to the audio buffer. (Stereo output; MMX-optimized)
		 * @return 0 on success; non-zero on error.
		 */
		int writeStereoMMX(void);
		
		/**
		 * writeMonoMMX(): Write the current segment to the audio buffer. (Monaural output; MMX-optimized)
		 * @return 0 on success; non-zero on error.
		 */
		int writeMonoMMX(void);
#endif
};

}

#endif /* __GENS_QT4_AUDIO_PORTAUDIO_HPP__ */
