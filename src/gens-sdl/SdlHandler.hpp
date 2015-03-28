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

#ifndef __GENS_SDL_SDLHANDLER_HPP__
#define __GENS_SDL_SDLHANDLER_HPP__

struct SDL_Surface;

namespace LibGens {
	class MdFb;
}

namespace GensSdl {

class SdlHandler {
	public:
		SdlHandler();
		~SdlHandler();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		SdlHandler(const SdlHandler &);
		SdlHandler &operator=(const SdlHandler &);

	public:
		/**
		 * Initialize SDL video.
		 * TODO: Parameter for GL rendering.
		 * @return 0 on success; non-zero on error.
		 */
		int init_video(void);

		/**
		 * Shut down SDL video.
		 */
		void end_video(void);

		/**
		 * Set the SDL video source to an MdFb.
		 * If nullptr, removes the SDL video source.
		 * @param fb MdFb.
		 */
		void set_video_source(LibGens::MdFb *fb);

		/**
		 * Update SDL video.
		 */
		void update_video(void);

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
