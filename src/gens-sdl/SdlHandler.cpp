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
}

}
