/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlGLBackend.hpp: SDL OpenGL rendeirng backend.                         *
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

#ifndef __GENS_SDL_SDLGLBACKEND_HPP__
#define __GENS_SDL_SDLGLBACKEND_HPP__

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <climits>

// SDL
#include <SDL.h>

// Video Backend.
#include "VBackend.hpp"
#include "GLBackend.hpp"
// LibGens includes.
#include "libgens/Util/MdFb.hpp"

namespace GensSdl {

class SdlGLBackend : public GLBackend {
	public:
		SdlGLBackend();
		virtual ~SdlGLBackend();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		SdlGLBackend(const SdlGLBackend &);
		SdlGLBackend &operator=(const SdlGLBackend &);

	public:
		/**
		 * Set the window title.
		 * TODO: Set based on "paused" and fps values?
		 * @param title Window title.
		 */
		virtual void set_window_title(const char *title) final;

		/**
		 * Update video.
		 * @param fb_dirty If true, MdFb was updated.
		 */
		virtual void update(bool fb_dirty) final;

		/**
		 * Toggle fullscreen.
		 */
		virtual void toggle_fullscreen(void) final;

	private:
		// Screen context.
		SDL_Window *m_window;
		SDL_GLContext m_glContext;
};

}

#endif /* __GENS_SDL_SDLGLBACKEND_HPP__ */
