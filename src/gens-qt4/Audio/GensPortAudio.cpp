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

// CPU flags.
#include "libgens/Util/cpuflags.h"

// Qt includes.
#include <QtCore/QMutexLocker>

namespace GensQt4
{

GensPortAudio::GensPortAudio()
{
	// Clear internal variables.
	m_bufferPos = 0;
	m_sampleSize = 0;
}


GensPortAudio::~GensPortAudio()
{
	// NOTE: close() can't be called from ABackend::~ABackend();
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
	
	// Initialize the buffer before initializing PortAudio.
	// This prevents a race condition.
	QMutexLocker locker(&m_mtxBuffer);
	memset(m_buffer, 0x00, sizeof(m_buffer));
	m_bufferPos = 0;
	m_sampleSize = (sizeof(int16_t) * (m_stereo ? 2 : 1));
	
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
	m_open = true;
}


/**
 * close(): Close the audio stream.
 */
void GensPortAudio::close(void)
{
	QMutexLocker locker(&m_mtxBuffer);
	
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
	err = Pa_CloseStream(m_stream);
	if (err != paNoError)
	{
		// Error shutting down PortAudio.
		LOG_MSG(audio, LOG_MSG_LEVEL_ERROR,
			"Pa_CloseStream() error: %s", Pa_GetErrorText(err));
	}
	m_stream = NULL;
	
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
 * setRate(): Set the sampling rate.
 * @param newRate New sampling rate.
 */
void GensPortAudio::setRate(int newRate)
{
	if (m_rate == newRate)
		return;
	
	if (newRate > LibGens::SoundMgr::MAX_SAMPLING_RATE)
	{
		// Sampling rate is too high for LibGens.
		return;
	}
	
	if (m_open)
	{
		// Close and reopen the PortAudio stream.
		// Make sure PSG/YM state is saved when we do this.
		// TODO: Insert a pause between close() and open() to prevent stuttering?
		close();
		m_rate = newRate;
		LibGens::SoundMgr::SetRate(newRate, true);
		open();
	}
	else
	{
		// Audio isn't open. Save the new audio rate.
		// PSG/YM state doesn't need to be saved.
		m_rate = newRate;
		LibGens::SoundMgr::SetRate(newRate, false);
	}
}


/**
 * setStereo(): Set stereo or mono.
 * @param newStereo True for stereo; false for mono.
 */
void GensPortAudio::setStereo(bool newStereo)
{
	if (m_stereo == newStereo)
		return;
	
	if (m_open)
	{
		// Close and reopen the PortAudio stream.
		// TODO: Insert a pause between close() and open() to prevent stuttering?
		close();
		m_stereo = newStereo;
		open();
	}
	else
	{
		m_stereo = newStereo;
	}
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
	QMutexLocker locker(&m_mtxBuffer);
	
	// NOTE: Sample size is 16-bit, but we're using 8-bit here
	// for convenience, since everything's measured in bytes.
	uint8_t *out = (uint8_t*)outputBuffer;
	
	// Get the data from the buffer.
	framesPerBuffer *= m_sampleSize;
	
	if (m_bufferPos < framesPerBuffer)
	{
		// Not enough data in the buffer.
		// Copy whatever's left.
		memcpy(out, m_buffer, m_bufferPos);
		memset(&out[m_bufferPos], 0x00, (framesPerBuffer - m_bufferPos));
		m_bufferPos = 0;
	}
	else
	{
		// Enough data is available in the buffer.
		memcpy(out, m_buffer, framesPerBuffer);
		
		// Shift the rest of the buffer over.
		unsigned long diff = (m_bufferPos - framesPerBuffer);
		if (diff == 0)
			m_bufferPos = 0;
		else
		{
			memmove(m_buffer, &m_buffer[framesPerBuffer>>1], diff);
			m_bufferPos = diff;
		}
	}
	
	return 0;
}


/**
 * write(): Write the current segment to the audio buffer.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::write(void)
{
	QMutexLocker locker(&m_mtxBuffer);
	
	if (!m_open)
		return 1;
	
	// TODO: Lock the buffer for writing.
	// TODO: Use the segment size.
	int segLength = (LibGens::SoundMgr::GetSegLength() * m_sampleSize);
	if ((m_bufferPos + segLength) > sizeof(m_buffer))
	{
		fprintf(stderr, "GensPortAudio::%s(): Internal buffer overflow.\n", __func__);
		return 1;
	}
	
	int16_t *buf = &m_buffer[m_bufferPos>>1];
	
	// TODO: MMX versions.
	int ret;
#ifdef HAVE_MMX
	if (CPU_Flags & MDP_CPUFLAG_X86_MMX)
	{
		// MMX is supported.
		if (m_stereo)
			ret = writeStereoMMX(buf);
		else
			ret = writeMonoMMX(buf);
	}
	else
#endif /* HAVE_MMX */
	if (m_stereo)
		ret = writeStereo(buf);
	else
		ret = writeMono(buf);
	
	// Increment the buffer position.
	m_bufferPos += segLength;
	
	// Unlock the ring buffer.
	// TODO
	//m_buffer.writeUnlock();
	return ret;
}

}
