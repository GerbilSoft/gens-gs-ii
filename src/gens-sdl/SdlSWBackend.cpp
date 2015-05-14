/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlSWBackend.cpp: SDL software rendeirng backend.                       *
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

#include "SdlSWBackend.hpp"
#include "libgens/Util/MdFb.hpp"

// C includes. (C++ namespace)
#include <cassert>

#include <SDL.h>

namespace GensSdl {

SdlSWBackend::SdlSWBackend()
	: m_screen(nullptr)
	, m_md(nullptr)
{
	// Initialize the SDL window.
	m_screen = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
}

SdlSWBackend::~SdlSWBackend()
{
	if (m_md) {
		SDL_FreeSurface(m_md);
		m_md = nullptr;
	}

	SDL_FreeSurface(m_screen);
}

/**
 * Set the SDL video source to an MdFb.
 * If nullptr, removes the SDL video source.
 * @param fb MdFb.
 */
void SdlSWBackend::set_video_source(LibGens::MdFb *fb)
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
 * Update video.
 * @param fb_dirty If true, MdFb was updated.
 */
void SdlSWBackend::update(bool fb_dirty)
{
	// We always have to draw the MdFb in software mode.
	((void)fb_dirty);

	if (!m_md) {
		// No source surface.
		SDL_FillRect(m_screen, nullptr, 0);
	} else {
		// Source surface is available.
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = 320;
		rect.h = 240;
		SDL_BlitSurface(m_md, &rect, m_screen, &rect);
	}

	// Update the screen.
	SDL_Flip(m_screen);
}

/**
 * Viewing area has been resized.
 * @param width Width.
 * @param height Height.
 */
void SdlSWBackend::resize(int width, int height)
{
	// Can't resize this...
	((void)width);
	((void)height);
	assert(false);
	return;
}

}
