/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ABackend.hpp: Audio Backend base class.                                 *
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

#ifndef __GENS_QT4_AUDIO_ABACKEND_HPP__
#define __GENS_QT4_AUDIO_ABACKEND_HPP__

// C includes.
#include <stdint.h>

// PortAudio.
#include "portaudio.h"

// Qt includes.
#include <QtCore/QMutex>

// TODO: Move this somewhere else!
#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))
#define HAVE_MMX // do this later
#endif

namespace GensQt4
{

class ABackend
{
	public:
		ABackend();
		virtual ~ABackend();
		
		inline bool isOpen(void) const { return m_open; }
		
		virtual void open(void) = 0;
		virtual void close(void) = 0;
		
		/**
		 * Properties.
		 */
		inline int rate(void) const { return m_rate; }
		virtual void setRate(int newRate) = 0;
		
		inline bool isStereo(void) const { return m_stereo; }
		virtual void setStereo(bool newStereo) = 0;
		
		/**
		 * write(): Write the current segment to the audio buffer.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int write(void) = 0;
		
		virtual void wpSegWait(void) const = 0;
		virtual bool isBufferEmpty(void) const = 0;
	
	protected:
		bool m_open;	// True if PortAudio is initialized.
		
		// Number of segments to buffer.
		static const int SEGMENTS_TO_BUFFER = 8;
		
		// Audio settings.
		int m_rate;
		bool m_stereo;
		
		/** Internal audio write functions. **/
		
		/**
		 * writeStereo(): Write the current segment to the audio buffer. (Stereo output)
		 * @param dest Destination buffer.
		 * @return 0 on success; non-zero on error.
		 */
		static int WriteStereo(int16_t *dest);
		
		/**
		 * writeMono(): Write the current segment to the audio buffer. (Monaural output)
		 * @param dest Destination buffer.
		 * @return 0 on success; non-zero on error.
		 */
		static int WriteMono(int16_t *dest);
		
#ifdef HAVE_MMX
		/**
		 * writeStereoMMX(): Write the current segment to the audio buffer. (Stereo output; MMX-optimized)
		 * @param dest Destination buffer.
		 * @return 0 on success; non-zero on error.
		 */
		static int WriteStereoMMX(int16_t *dest);
		
		/**
		 * writeMonoMMX(): Write the current segment to the audio buffer. (Monaural output; MMX-optimized)
		 * @param dest Destination buffer.
		 * @return 0 on success; non-zero on error.
		 */
		static int WriteMonoMMX(int16_t *dest);
#endif
};

}

#endif /* __GENS_QT4_AUDIO_ABACKEND_HPP__ */
