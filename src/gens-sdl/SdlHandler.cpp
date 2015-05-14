/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlHandler.cpp: SDL library handler.                                    *
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

#include "SdlHandler.hpp"
#include "libgens/Util/MdFb.hpp"

#include "libgens/sound/SoundMgr.hpp"
using LibGens::SoundMgr;

#include <SDL.h>
#include <cstdio>

#include "RingBuffer.hpp"
#include "SdlSWBackend.hpp"
#include "SdlGLBackend.hpp"

namespace GensSdl {

SdlHandler::SdlHandler()
	: m_vBackend(nullptr)
	, m_framesRendered(0)
	, m_audioBuffer(nullptr)
	, m_sampleSize(0)
	, m_segBuffer(nullptr)
	, m_segBufferLen(0)
	, m_segBufferSamples(0)
{ }

SdlHandler::~SdlHandler()
{
	// Shut. Down. EVERYTHING.
	end_video();
	end_audio();
}

/**
 * Initialize SDL video.
 * TODO: Parameter for GL rendering.
 * @return 0 on success; non-zero on error.
 */
int SdlHandler::init_video(void)
{
	if (m_vBackend) {
		// Video is already initialized.
		// Shut it down, then reinitialize it.
		delete m_vBackend;
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	int ret = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if (ret < 0) {
		fprintf(stderr, "%s failed: %d - %s\n",
			__func__, ret, SDL_GetError());
		return ret;
	}

	// Initialize the video backend.
	// TODO: Fullscreen; GL vs. SW selection; VSync.
	m_vBackend = new SdlGLBackend();
	return 0;
}

/**
 * Shut down SDL video.
 */
void SdlHandler::end_video(void)
{
	if (m_vBackend) {
		delete m_vBackend;
		m_vBackend = nullptr;
	}
}

/**
 * Set the SDL video source to an MdFb.
 * If nullptr, removes the SDL video source.
 * @param fb MdFb.
 */
void SdlHandler::set_video_source(LibGens::MdFb *fb)
{
	if (m_vBackend) {
		m_vBackend->set_video_source(fb);
	}
}

/**
 * Update SDL video.
 */
void SdlHandler::update_video(void)
{
	if (m_vBackend) {
		// TODO: Don't set fb_dirty == true when paused.
		m_vBackend->update(true);
	}

	// Update the screen.
	m_framesRendered++;
}

/**
 * Resize the video renderer.
 * @param width Width.
 * @param height Height.
 */
void SdlHandler::resize_video(int width, int height)
{
	if (m_vBackend) {
		m_vBackend->resize(width, height);
	}
}

/**
 * Get the Video Backend.
 * @return Video Backend.
 */
VBackend *SdlHandler::vBackend(void) const
{
	return m_vBackend;
}

/**
 * Initialize SDL audio.
 * @return 0 on success; non-zero on error.
 */
int SdlHandler::init_audio(void)
{
	SDL_AudioSpec wanted_spec, actual_spec;

	if (m_audioBuffer) {
		// Audio is already initialized.
		// Shut it down, then reinitialize it.
		end_audio();
	}

	int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
	if (ret < 0) {
		fprintf(stderr, "%s failed: %d - %s\n",
			__func__, ret, SDL_GetError());
		return ret;
	}

	// Number of samples to buffer.
	// FIXME: Should be segment size, rounded up to pow2.
	wanted_spec.freq	= 44100;
	wanted_spec.format	= AUDIO_S16LSB;
	wanted_spec.channels	= 2;
	wanted_spec.samples	= 1024;
	wanted_spec.callback	= sdl_audio_callback;
	wanted_spec.userdata	= this;
	ret = SDL_OpenAudio(&wanted_spec, &actual_spec);
	if (ret < 0) {
		fprintf(stderr, "%s: SDL_OpenAudio() failed: %d - %s\n",
			__func__, ret, SDL_GetError());
		return ret;
	}

	// Initialize SoundMgr.
	// TODO: NTSC/PAL setting.
	SoundMgr::ReInit(actual_spec.freq, false, true);

	// TODO: Verify the actual spec has the correct
	// number of channels and the right format.
	// Allocate the RingBuffer.
	if (m_audioBuffer) {
		delete m_audioBuffer;
	}
	// Buffer should be (SegLength * 4) + actual samples.
	int samples = (SoundMgr::GetSegLength() * 4) + actual_spec.samples;
	m_audioBuffer = new RingBuffer(samples);

	// Segment buffer.
	// Needed to convert "int32_t" to int16_t.
	m_sampleSize = 4; // TODO: Move to RingBuffer?
	m_segBufferSamples = SoundMgr::GetSegLength();
	m_segBufferLen = m_segBufferSamples * m_sampleSize;
	m_segBuffer = (int16_t*)calloc(1, m_segBufferLen);

	// Audio is initialized.
	return 0;
}

/**
 * Shut down SDL audio.
 */
void SdlHandler::end_audio(void)
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	free(m_audioBuffer);
	m_audioBuffer = nullptr;
	m_sampleSize = 0;
	free(m_segBuffer);
	m_segBuffer = nullptr;
	m_segBufferLen = 0;
	m_segBufferSamples = 0;
}

/**
 * SDL audio callback.
 * @param userdata SdlHandler class pointer.
 * @param stream SDL audio stream.
 * @param len Number of bytes requested.
 */
void SdlHandler::sdl_audio_callback(void *userdata, uint8_t *stream, int len)
{
	SdlHandler *handler = (SdlHandler*)userdata;

	// Read data from the RingBuffer.
	unsigned int wrote = handler->m_audioBuffer->read(stream, len);
	//printf("callback: request %d, read %u\n", len, wrote);
	if ((int)wrote == len) {
		// Correct amount of data read.
		return;
	}

	// Not enough data. Fill the remaining space with silence.
	memset(&stream[wrote], 0, ((unsigned int)len - wrote));
}

/**
 * Update SDL audio using SoundMgr.
 */
void SdlHandler::update_audio(void)
{
	// Mostly copied from GensQt4's ABackend.
	// TODO: Reimplement the MMX version.
	// TODO: Move to LibGens.

	// Convert from "int32_t" to int16_t.
	int16_t *dest = m_segBuffer;

	// Source buffer pointers.
	int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	for (int i = SoundMgr::GetSegLength() - 1; i >= 0;
	     i--, srcL++, srcR++, dest += 2)
	{
		if (*srcL < -0x8000) {
			*dest = -0x8000;
		} else if (*srcL > 0x7FFF) {
			*dest = 0x7FFF;
		} else {
			*dest = (int16_t)(*srcL);
		}

		if (*srcR < -0x8000) {
			*(dest+1) = -0x8000;
		} else if (*srcR > 0x7FFF) {
			*(dest+1) = 0x7FFF;
		} else {
			*(dest+1) = (int16_t)(*srcR);
		}
	}

	// Clear the segment buffers.
	// These buffers are additive, so if they aren't cleared,
	// we'll end up with static.
	memset(SoundMgr::ms_SegBufL, 0, m_segBufferSamples * sizeof(SoundMgr::ms_SegBufL[0]));
	memset(SoundMgr::ms_SegBufR, 0, m_segBufferSamples * sizeof(SoundMgr::ms_SegBufL[0]));

	// Write to the ringbuffer.
	SDL_LockAudio();
	m_audioBuffer->write(reinterpret_cast<uint8_t*>(m_segBuffer), m_segBufferLen);
	SDL_UnlockAudio();
}

}
