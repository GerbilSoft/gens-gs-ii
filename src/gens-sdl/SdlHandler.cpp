/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlHandler.hpp: SDL library handler.                                    *
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

namespace GensSdl {

SdlHandler::SdlHandler()
	: m_screen(nullptr)
	, m_fb(nullptr)
	, m_md(nullptr)
	, m_sem(nullptr)
	, m_ticks(0)
	, m_timer(nullptr)
	, m_framesRendered(0)
	, m_audioBuffer(nullptr)
	, m_audioBufferLen(0)
	, m_audioBufferUsed(0)
	, m_sampleSize(0)
	, m_audioWritePos(nullptr)
{ }

SdlHandler::~SdlHandler()
{
	// Shut. Down. EVERYTHING.
	end_video();
	end_timers();
	end_audio();
}

/**
 * Initialize SDL video.
 * TODO: Parameter for GL rendering.
 * @return 0 on success; non-zero on error.
 */
int SdlHandler::init_video(void)
{
	if (m_screen) {
		// Video is already initialized.
		// Shut it down, then reinitialize it.
		end_video();
	}

	int ret = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if (ret < 0) {
		fprintf(stderr, "%s failed: %d - %s\n",
			__func__, ret, SDL_GetError());
		return ret;
	}

	// Create the screen surface.
	// TODO: Fullscreen option.
	m_screen = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE);
	return 0;
}

/**
 * Shut down SDL video.
 */
void SdlHandler::end_video(void)
{
	if (m_screen) {
		SDL_FreeSurface(m_screen);
		m_screen = nullptr;
	}
	// Delete m_md before unreferencing m_fb.
	if (m_md) {
		SDL_FreeSurface(m_md);
		m_md = nullptr;
	}
	if (m_fb) {
		m_fb->unref();
		m_fb = nullptr;
	}
}

/**
 * Set the SDL video source to an MdFb.
 * If nullptr, removes the SDL video source.
 * @param fb MdFb.
 */
void SdlHandler::set_video_source(LibGens::MdFb *fb)
{
	// Free the existing MD surface first.
	if (m_md) {
		SDL_FreeSurface(m_md);
		m_md = nullptr;
		m_fb->unref();
		m_fb = nullptr;
	}

	if (fb) {
		m_fb = fb->ref();
		m_md = SDL_CreateRGBSurfaceFrom(m_fb->fb32(), 320, 240, 32, 336*4, 0, 0, 0, 0);
	}
}

/**
 * Update SDL video.
 */
void SdlHandler::update_video(void)
{
	if (!m_md) {
		// No source surface.
		SDL_FillRect(m_screen, nullptr, 0);
	} else {
		// Source surface is available.
		SDL_Rect rect { 0, 0, 320, 240 };
		SDL_BlitSurface(m_md, &rect, m_screen, &rect);
	}

	// Update the screen.
	SDL_UpdateRect(m_screen, 0, 0, 0, 0);
	m_framesRendered++;
}

/**
 * Initialize SDL timers and threads.
 * @return 0 on success; non-zero on error.
 */
int SdlHandler::init_timers(void)
{
	int ret = SDL_InitSubSystem(SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD);
	if (ret < 0) {
		fprintf(stderr, "%s failed: %d - %s\n",
			__func__, ret, SDL_GetError());
		return ret;
	}

	m_sem = SDL_CreateSemaphore(0);
	m_ticks = 0;
	return 0;
}

/**
 * Shut down SDL timers and threads.
 */
void SdlHandler::end_timers(void)
{
	if (m_sem) {
		SDL_DestroySemaphore(m_sem);
		m_sem = nullptr;
	}
}

/**
 * Start and/or restart the synchronization timer.
 * @param isPal If true, use PAL timing.
 */
void SdlHandler::start_timer(bool isPal)
{
	if (m_timer) {
		// Timer is already set.
		// Stop it first.
		SDL_RemoveTimer(m_timer);
	}

	// Synchronize on every three frames:
	// - NTSC: 60 Hz == 50 ms
	// - PAL: 50 Hz == 60 ms
	m_isPal = isPal;
	m_timer = SDL_AddTimer(m_isPal ? 60 : 50, sdl_timer_callback, this);
}

/**
 * SDL synchronization timer callback.
 * @param interval Timer interval.
 * @param param SdlHandler class pointer.
 * @return Timer interval to use.
 */
uint32_t SdlHandler::sdl_timer_callback(uint32_t interval, void *param)
{
	SdlHandler *handler = (SdlHandler*)param;
	// TODO: Skip frames if it's running too slowly?
	SDL_SemPost(handler->m_sem);
	handler->m_ticks++;
	if (handler->m_ticks == (handler->m_isPal ? 50 : 20)) {
		// TODO: Update the framerate on the window title.

		// Clear the tick and frame rendering count.
		handler->m_ticks = 0;
		handler->m_framesRendered = 0;
	}

	return interval;
}

/**
 * Wait for frame synchronization.
 * On every third frame, wait for the timer.
 */
void SdlHandler::wait_for_frame_sync(void)
{
	if (m_framesRendered % 3 == 0) {
		// Third frame.
		SDL_SemWait(m_sem);
	}
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
	static const int BUFFER_SIZE = 1024;
	wanted_spec.freq	= 44100;
	wanted_spec.format	= AUDIO_S16LSB;
	wanted_spec.channels	= 2;
	wanted_spec.samples	= BUFFER_SIZE;
	wanted_spec.callback	= sdl_audio_callback;
	wanted_spec.userdata	= this;
	ret = SDL_OpenAudio(&wanted_spec, &actual_spec);
	if (ret < 0) {
		fprintf(stderr, "%s: SDL_OpenAudio() failed: %d - %s\n",
			__func__, ret, SDL_GetError());
		return ret;
	}

	// Allocate the buffer.
	m_sampleSize = 4;
	m_audioBufferUsed = 0;
	m_audioBufferLen = actual_spec.samples * m_sampleSize * SEGMENTS_TO_BUFFER;
	m_audioBuffer = (uint8_t*)calloc(m_audioBufferLen, 1);
	if (!m_audioBuffer) {
		fprintf(stderr, "%s: calloc() failed for audio buffer\n", __func__);
		SDL_CloseAudio();
		m_sampleSize = 0;
		m_audioBufferLen = 0;
		return -1;
	}
	m_audioWritePos = m_audioBuffer;

	// Reinitialize SoundMgr.
	// TODO: NTSC/PAL setting.
	SoundMgr::ReInit(actual_spec.freq, false, true);

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
	m_audioBufferLen = 0;
	m_sampleSize = 0;
	m_audioWritePos = nullptr;
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
	//printf("requesting %d bytes; buffer has %d bytes used\n", len, handler->m_audioBufferUsed);
	if (handler->m_audioBufferUsed < len) {
		// Not enough audio data.
		// Copy over what's available?
		memcpy(stream, handler->m_audioBuffer, handler->m_audioBufferUsed);
		memset(stream + handler->m_audioBufferUsed, 0,
		       len - handler->m_audioBufferUsed);
		handler->m_audioBufferUsed = 0;
		handler->m_audioWritePos = handler->m_audioBuffer;
	} else {
		// Copy our emulated audio to the SDL buffer.
		memcpy(stream, handler->m_audioBuffer, len);
		handler->m_audioBufferUsed -= len;

		// Compesate for desynchronization.
		// FIXME: This causes audio to randomly speed up...
		do {
			handler->m_audioBufferUsed -= len;
		} while (handler->m_audioBufferUsed > (2 * len));

		// Adjust the buffer contents.
		memmove(handler->m_audioBuffer,
			handler->m_audioWritePos - handler->m_audioBufferUsed,
			handler->m_audioBufferUsed);

		handler->m_audioWritePos = handler->m_audioBuffer + handler->m_audioBufferUsed;
	}
}

/**
 * Update SDL audio using SoundMgr.
 */
void SdlHandler::update_audio(void)
{
	// Mostly copied from GensQt4's ABackend.
	// TODO: Reimplement the MMX version.
	// TODO: Move to LibGens.
	const int SegLength = LibGens::SoundMgr::GetSegLength();
	const int SegBytes = SegLength * m_sampleSize;
	// FIXME: This keeps running over for some reason.
	if (SegLength * m_sampleSize > m_audioBufferLen - m_audioBufferUsed) {
		// Not enough space left in the buffer...
		// TODO: What should we do here?
		printf("audio buffer out of space; %d bytes used, %d total, needs %d more bytes\n",
		       m_audioBufferLen, m_audioBufferUsed, SegBytes);
		return;
	}

	SDL_LockAudio();
	int16_t *dest = (int16_t*)m_audioWritePos;

	// Source buffer pointers.
	int32_t *srcL = &SoundMgr::ms_SegBufL[0];
	int32_t *srcR = &SoundMgr::ms_SegBufR[0];

	for (int i = SegLength; i > 0; i--, srcL++, srcR++, dest += 2) {
		if (*srcL < -0x8000)
			*dest = -0x8000;
		else if (*srcL > 0x7FFF)
			*dest = 0x7FFF;
		else
			*dest = (int16_t)(*srcL);
		
		if (*srcR < -0x8000)
			*(dest+1) = -0x8000;
		else if (*srcR > 0x7FFF)
			*(dest+1) = 0x7FFF;
		else
			*(dest+1) = (int16_t)(*srcR);
	}

	// Clear the segment buffers.
	memset(SoundMgr::ms_SegBufL, 0x00, SegLength*sizeof(SoundMgr::ms_SegBufL[0]));
	memset(SoundMgr::ms_SegBufR, 0x00, SegLength*sizeof(SoundMgr::ms_SegBufR[0]));

	m_audioWritePos = (uint8_t*)dest;
	m_audioBufferUsed += (SegLength * m_sampleSize);
	SDL_UnlockAudio();
}

}
