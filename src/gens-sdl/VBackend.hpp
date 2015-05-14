/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * VBackend.hpp: Video Backend base class.                                 *
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

#ifndef __GENS_SDL_VBACKEND_HPP__
#define __GENS_SDL_VBACKEND_HPP__

#include <stdint.h>

namespace LibGens {
	class MdFb;
	class EmuContext;
}

namespace GensSdl {

class VBackend {
	public:
		VBackend();
		virtual ~VBackend();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		VBackend(const VBackend &);
		VBackend &operator=(const VBackend &);

	public:
		/**
		 * Set the video source to an MdFb.
		 * If nullptr, removes the video source.
		 * @param fb MdFb.
		 */
		virtual void set_video_source(LibGens::MdFb *fb) = 0;

		/**
		 * Update video.
		 * @param fb_dirty If true, MdFb was updated.
		 */
		virtual void update(bool fb_dirty) = 0;

		/**
		 * Viewing area has been resized.
		 * @param width Width.
		 * @param height Height.
		 */
		virtual void resize(int width, int height) = 0;

	public:
		/** Properties. **/

		// Stretch mode.
		enum StretchMode_t {
			STRETCH_NONE = 0,
			STRETCH_H,
			STRETCH_V,
			STRETCH_FULL,

			STRETCH_MAX
		};
		StretchMode_t stretchMode(void) const;
		void setStretchMode(StretchMode_t stretchMode);

	protected:
		// MdFb object.
		LibGens::MdFb *m_fb;

		// Properties.
		StretchMode_t m_stretchMode;
};

}

#endif /* __GENS_SDL_VBACKEND_HPP__ */
