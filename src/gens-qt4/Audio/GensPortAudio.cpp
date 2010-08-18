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

// C includes.
#include <string.h>
#include <unistd.h>

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

// LibGens Sound Manager.
#include "libgens/sound/SoundMgr.hpp"

namespace GensQt4
{

GensPortAudio::GensPortAudio()
{
	// Assume PortAudio isn't open initially.
	m_open = false;
	
	// Initialize settings.
	// TODO: Allow user customization.
	m_rate = 44100;
	m_stereo = true;
}


GensPortAudio::~GensPortAudio()
{
	close();
};


/**
 * open(): Open the audio stream.
 */
void GensPortAudio::open(void)
{
	if (m_open)
		return;
	
	// TODO: Make sure the LibGens Sound Manager is initialized.
	
	// Clear the internal buffer.
	memset(m_buf, 0x00, sizeof(m_buf));
	
	// Initialize the buffer variables.
	m_bufLen = (LibGens::SoundMgr::GetSegLength() * 8);
	m_bufPos = 0;
	m_sampleSize = 0;
	
	// Initialize PortAudio.
	int err = Pa_Initialize();
	if (err != paNoError)
	{
		// Error initializing PortAudio!
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_Initialize() error: %s", Pa_GetErrorText(err));
		return;
	}
	
	// Get the default device information.
	PaDeviceIndex defaultDevIndex = Pa_GetDefaultOutputDevice();
	if (defaultDevIndex == paNoDevice)
	{
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_GetDefaultOutputDevice() returned paNoDevice.");
		Pa_Terminate();
		return;
	}
	
	const PaDeviceInfo *dev = Pa_GetDeviceInfo(defaultDevIndex);
	if (!dev)
	{
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_GetDeviceInfo(%d) returned NULL.", defaultDevIndex);
		Pa_Terminate();
		return;
	}
	
	PaStreamParameters stream_params;
	stream_params.channelCount = (m_stereo ? 2 : 1);
	stream_params.device = defaultDevIndex;
	stream_params.hostApiSpecificStreamInfo = NULL;
	stream_params.sampleFormat = paInt16;
	stream_params.suggestedLatency = dev->defaultLowOutputLatency;
	
	// Open an audio stream.
	err = Pa_OpenStream(&m_stream,
				NULL,			// no input channels
				&stream_params,		// output configuration
				m_rate,			// Sample rate
				256,			// Buffer size
				0,			// Stream flags. (TODO)
				GensPaCallback,		// Callback function
				this);			// Pointer to this object
	
	if (err != paNoError)
	{
		// Error initializing the PortAudio stream.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_OpenDefaultStream() error: %s", Pa_GetErrorText(err));
		Pa_Terminate();
	}
	
	// Start the PortAudio stream.
	err = Pa_StartStream(m_stream);
	if (err != paNoError)
	{
		// Error starting the PortAudio stream.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_StartStream() error: %s", Pa_GetErrorText(err));
	}
	
	// PortAudio stream is open.
	m_sampleSize = (sizeof(int16_t) * (m_stereo ? 2 : 1));
	m_open = true;
}


/**
 * close(): Close the audio stream.
 */
void GensPortAudio::close(void)
{
	if (!m_open)
		return;
	
	int err;
	
	// Stop the PortAudio stream.
	err = Pa_StopStream(m_stream);
	if (err != paNoError)
	{
		// Error stopping the PortAudio stream.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_StopStream() error: %s", Pa_GetErrorText(err));
	}
	
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
	m_sampleSize = 0;
}


/**
 * gensPaCallback(): PortAudio callback function.
 * @return ???
 */
int GensPortAudio::gensPaCallback(const void *inputBuffer, void *outputBuffer,
				  unsigned long framesPerBuffer,
				  const PaStreamCallbackTimeInfo *timeInfo,
				  PaStreamCallbackFlags statusFlags)
{
	// Sample sawtooth wave function.
	// http://www.portaudio.com/trac/wiki/TutorialDir/WritingACallback
	uint16_t *out = (uint16_t*)outputBuffer;
	
	// Lock the audio buffer.
	m_mtxBuf.lock();
	
	if (m_bufPos <= 0)
	{
		// Audio is empty.
		m_mtxBuf.unlock();
		
		// Zero the output buffer.
		memset(out, 0x00, framesPerBuffer * m_sampleSize);
		return 0;
	}
	
	// Copy our audio data directly to the output buffer.
	if (m_bufPos < framesPerBuffer)
	{
		memcpy(out, m_buf, m_bufPos * m_sampleSize);
		m_bufPos = 0;
		m_mtxBuf.unlock();
		
		// Zero out the rest of the buffer.
		memset(&out[m_bufPos], 0x00, (framesPerBuffer - m_bufPos) * m_sampleSize);
		return 0;
	}
	
	memcpy(out, m_buf, (framesPerBuffer * m_sampleSize));
	m_bufPos -= framesPerBuffer;
	
	// Shift all the data over.
	// RShift is because m_buf is int16_t.
	memmove(m_buf, &m_buf[(framesPerBuffer * m_sampleSize) >> 1], (m_bufPos * m_sampleSize));
	
	// Unlock the audio buffer.
	m_mtxBuf.unlock();
	return 0;
}

}
