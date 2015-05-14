/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlSWBackend.hpp: SDL software rendeirng backend.                       *
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

#ifndef __GENS_SDL_SDLSWBACKEND_HPP__
#define __GENS_SDL_SDLSWBACKEND_HPP__

#include <stdint.h>

// SDL
#include <SDL.h>

namespace LibGens {
	class MdFb;
}

namespace GensSdl {

class SdlSWBackend {
	public:
		SdlSWBackend();
		virtual ~SdlSWBackend();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		SdlSWBackend(const SdlSWBackend &);
		SdlSWBackend &operator=(const SdlSWBackend &);

	public:
		/**
		 * Set the SDL video source to an MdFb.
		 * If nullptr, removes the SDL video source.
		 * @param fb MdFb.
		 */
		void set_video_source(LibGens::MdFb *fb);

		/**
		 * Update SDL video.
		 */
		void update(void);

	private:
		// Screen buffer.
		SDL_Surface *m_screen;
		// MdFb object.
		LibGens::MdFb *m_fb;
		// MD screen buffer.
		// Points to data on an MdFb.
		SDL_Surface *m_md;
};

}

#endif /* __GENS_SDL_SDLHANDLER_HPP__ */
