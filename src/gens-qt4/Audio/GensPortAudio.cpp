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

#include "GensPortAudio.hpp"

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

namespace GensQt4
{

GensPortAudio::GensPortAudio()
{
	// Assume PortAudio isn't open initially.
	m_open = false;
	
	// Initialize PortAudio.
	int err = Pa_Initialize();
	if (err != paNoError)
	{
		// Error initializing PortAudio!
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_Initialize() error: %s", Pa_GetErrorText(err));
		return;
	}
	
	// Initialize the sawtooth wave variables.
	m_leftPhase = 0.0f;
	m_rightPhase = 0.0f;
	
	// Open an audio stream.
	err = Pa_OpenDefaultStream(&m_stream,
					0,		// no input channels
					2,		// stereo output
					paFloat32,	// 32-bit floating point output
					44100,		// Sample rate
					256,		// Frames per buffer
					GensPaCallback,	// Callback function
					this);		// Pointer to this class
	if (err != paNoError)
	{
		// Error initializing the PortAudio stream.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_OpenDefaultStream() error: %s", Pa_GetErrorText(err));
		Pa_Terminate();
	}
}

GensPortAudio::~GensPortAudio()
{
	if (!m_open)
		return;
	
	int err;
	
	// Close the PortAudio stream.
	if (m_stream)
	{
		err = Pa_CloseStream(m_stream);
		if (err != paNoError)
		{
			// Error shutting down PortAudio.
			LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
				"Pa_CloseStream() error: %s", Pa_GetErrorText(err));
		}
		
		m_stream = NULL;
	}
		
	// Shut down PortAudio.
	err = Pa_Terminate();
	if (err != paNoError)
	{
		// Error shutting down PortAudio.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_Terminate(): %s", Pa_GetErrorText(err));
	}
	
	// PortAudio is shut down.
	m_open = false;
};


/**
 * gensPaCallback(): PortAudio callback function.
 */
int GensPortAudio::gensPaCallback(const void *inputBuffer, void *outputBuffer,
				  unsigned long framesPerBuffer,
				  const PaStreamCallbackTimeInfo *timeInfo,
				  PaStreamCallbackFlags statusFlags)
{
	// Sample sawtooth wave function.
	// http://www.portaudio.com/trac/wiki/TutorialDir/WritingACallback
	
	/* Cast data passed through stream to our structure. */
	float *out = (float*)outputBuffer;
	unsigned int i;
	((void)inputBuffer); /* Prevent unused variable warning. */

	for(i = 0; i < framesPerBuffer; i++)
	{
		*out++ = m_leftPhase;  /* left */
		*out++ = m_rightPhase;  /* right */
		
		/* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
		m_leftPhase += 0.01f;
		/* When signal reaches top, drop back down. */
		if( m_leftPhase >= 1.0f ) m_leftPhase -= 2.0f;
		/* higher pitch so we can distinguish left and right. */
		m_rightPhase += 0.03f;
		if( m_rightPhase >= 1.0f ) m_rightPhase -= 2.0f;
	}
	
	return 0;
}


/**
 * start(): Start the audio stream.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::start(void)
{
	if (!m_open)
		return 1;
	
	// Start the PortAudio stream.
	int err = Pa_StartStream(m_stream);
	if (err != paNoError)
	{
		// Error starting the PortAudio stream.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_StartStream() error: %s", Pa_GetErrorText(err));
	}
	
	return err;
}


/**
 * stop(): Stop the audio stream.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::stop(void)
{
	if (!m_open)
		return 1;
	
	// Stop the PortAudio stream.
	int err = Pa_StopStream(m_stream);
	if (err != paNoError)
	{
		// Error stopping the PortAudio stream.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_StopStream() error: %s", Pa_GetErrorText(err));
	}
	
	return err;
}

}
