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
	const PaDeviceInfo *dev = Pa_GetDeviceInfo(0);
	if (!dev)
	{
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_GetDeviceInfo(0) returned NULL.");
		Pa_Terminate();
		return;
	}
	
	PaStreamParameters stream_params;
	stream_params.channelCount = (m_stereo ? 2 : 1);
	stream_params.device = 0;
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
	
	// Sample size. (16-bit stereo)
	const int sample_size = (sizeof(uint16_t) * (m_stereo ? 2 : 1));
	
	if (m_bufPos <= 0)
	{
		// Audio is empty.
		m_mtxBuf.unlock();
		
		// Zero the output buffer.
		memset(out, 0x00, framesPerBuffer * sample_size);
		return 0;
	}
	
	// Copy our audio data directly to the output buffer.
	if (m_bufPos < framesPerBuffer)
	{
		memcpy(out, m_buf, m_bufPos * sample_size);
		m_bufPos = 0;
		m_mtxBuf.unlock();
		
		// Zero out the rest of the buffer.
		memset(&out[m_bufPos], 0x00, (framesPerBuffer - m_bufPos) * sample_size);
		return 0;
	}
	
	memcpy(out, m_buf, (framesPerBuffer*sizeof(uint16_t)*2));
	m_bufPos -= framesPerBuffer;
	
	// Shift all the data over.
	memmove(m_buf, &m_buf[framesPerBuffer], m_bufPos * sample_size);
	
	// Unlock the audio buffer.
	m_mtxBuf.unlock();
	return 0;
}


/**
 * write(): Write the current segment to the audio buffer.
 * TODO: Lock the internal audio buffer.
 */
int GensPortAudio::write(void)
{
	if (!m_open)
		return 1;
	
	const int SegLength = LibGens::SoundMgr::GetSegLength();
	while ((m_bufPos + SegLength) > m_bufLen)
	{
		// Buffer overflow! Wait for the buffer to decrease.
		// This seems to help limit the framerate when it's running too fast.
		// TODO: Move somewhere else?
		// TODO: usleep() or not?
	}
	
	// Lock the audio buffer.
	m_mtxBuf.lock();
	
	// TODO: Mono/stereo, MMX, etc.
	unsigned int i = 0;
	unsigned int bufIndex = m_bufPos;
	for (; i < SegLength && bufIndex < m_bufLen; i++, bufIndex++)
	{
		int32_t L = LibGens::SoundMgr::ms_SegBufL[i];
		int32_t R = LibGens::SoundMgr::ms_SegBufR[i];
		
		if (L < -0x8000)
			m_buf[bufIndex][0] = -0x8000;
		else if (L > 0x7FFF)
			m_buf[bufIndex][0] = 0x7FFF;
		else
			m_buf[bufIndex][0] = (int16_t)L;
		
		if (R < -0x8000)
			m_buf[bufIndex][1] = -0x8000;
		else if (R > 0x7FFF)
			m_buf[bufIndex][1] = 0x7FFF;
		else
			m_buf[bufIndex][1] = (int16_t)R;
		
		// Remove the sample from the segment buffers.
		// TODO: Maybe use memset() after everything's done.
		LibGens::SoundMgr::ms_SegBufL[i] = 0;
		LibGens::SoundMgr::ms_SegBufR[i] = 0;
	}
	
	// Increase the buffer position.
	m_bufPos += i;
	
	// Unlock the audio buffer.
	m_mtxBuf.unlock();
	return 0;
}

}
