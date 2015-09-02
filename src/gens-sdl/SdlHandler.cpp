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

// C includes. (C++ namespace)
#include <cstdio>

// aligned_malloc()
#include "libcompat/aligned_malloc.h"

#include <SDL.h>

#include "RingBuffer.hpp"
#include "SdlSWBackend.hpp"
#include "SdlGLBackend.hpp"

namespace GensSdl {

SdlHandler::SdlHandler()
	: m_vBackend(nullptr)
	, m_framesRendered(0)
	, m_audioDevice(0)
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
 * Set the window title.
 * TODO: Set based on "paused" and fps values?
 * @param title Window title.
 */
void SdlHandler::set_window_title(const char *title)
{
	if (m_vBackend) {
		m_vBackend->set_window_title(title);
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
 * Update video while emulation is paused.
 * If the VBackend is dirty, video is updated;
 * otherwise, nothing happens.
 * NOTE: This function does NOT update the frame counter.
 * @param force Force an update. Required if a window expose event occurred.
 */
void SdlHandler::update_video_paused(bool force)
{
	if (m_vBackend) {
		if (force || m_vBackend->isDirty()) {
			m_vBackend->update(false);
		}
	}
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
 * Toggle fullscreen.
 */
void SdlHandler::toggle_fullscreen(void)
{
	if (m_vBackend) {
		m_vBackend->toggle_fullscreen();
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
	wanted_spec.format	= AUDIO_S16SYS;
	wanted_spec.channels	= 2;
	wanted_spec.samples	= 1024;
	wanted_spec.callback	= sdl_audio_callback;
	wanted_spec.userdata	= this;
	m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &actual_spec, 0);
	if (m_audioDevice <= 0) {
		fprintf(stderr, "%s: SDL_OpenAudioDevice() failed: %d - %s\n",
			__func__, m_audioDevice, SDL_GetError());
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
	m_segBuffer = (int16_t*)aligned_malloc(16, m_segBufferLen);
	memset(m_segBuffer, 0, m_segBufferLen);

	// Audio is initialized.
	return 0;
}

/**
 * Pause SDL audio.
 * This resets the audio ringbuffer.
 * @param paused True to pause; false to unpause.
 */
void SdlHandler::pause_audio(bool pause)
{
	if (m_audioDevice <= 0)
		return;

	if (pause) {
		if (SDL_GetAudioDeviceStatus(m_audioDevice) == SDL_AUDIO_PLAYING) {
			// Pause audio.
			SDL_PauseAudioDevice(m_audioDevice, 1);
			// Clear the ringbuffer.
			m_audioBuffer->clear();
		}
	} else {
		if (SDL_GetAudioDeviceStatus(m_audioDevice) == SDL_AUDIO_PAUSED) {
			// Clear the ringbuffer.
			m_audioBuffer->clear();
			// Unpause audio.
			SDL_PauseAudioDevice(m_audioDevice, 0);
		}
	}
}

/**
 * Shut down SDL audio.
 */
void SdlHandler::end_audio(void)
{
	if (m_audioDevice <= 0)
		return;

	// Stop SDL audio.
	SDL_PauseAudioDevice(m_audioDevice, 1);
	SDL_CloseAudioDevice(m_audioDevice);
	m_audioDevice = 0;

	// Free the buffers.
	delete m_audioBuffer;
	m_audioBuffer = nullptr;
	m_sampleSize = 0;
	aligned_free(m_segBuffer);
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
	// TODO: If !m_audioDevice, just clear the internal
	// audio buffer instead of writing it.

	// FIXME: If our buffer is too small, we'll lose
	// some of the audio.
	int samples = SoundMgr::writeStereo(m_segBuffer, m_segBufferSamples);

	// Write to the ringbuffer.
	if (m_audioDevice > 0 && samples > 0) {
		const int bytes = samples * m_sampleSize;
		SDL_LockAudioDevice(m_audioDevice);
		m_audioBuffer->write(reinterpret_cast<const uint8_t*>(m_segBuffer), bytes);
		SDL_UnlockAudioDevice(m_audioDevice);
	}
}

}
