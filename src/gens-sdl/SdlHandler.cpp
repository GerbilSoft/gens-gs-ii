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
{ }

SdlHandler::~SdlHandler()
{
	// Shut. Down. EVERYTHING.
	end_video();
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

}
