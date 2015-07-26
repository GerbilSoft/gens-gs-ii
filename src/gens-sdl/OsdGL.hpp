/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdGL.hpp: Onscreen Display for OpenGL.                                 *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __GENS_SDL_OSDGL_HPP__
#define __GENS_SDL_OSDGL_HPP__

namespace GensSdl {

class OsdGLPrivate;
class OsdGL {
	public:
		OsdGL();
		~OsdGL();

	private:
		friend class OsdGLPrivate;
		OsdGLPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		OsdGL(const OsdGL &);
		OsdGL &operator=(const OsdGL &);

	public:
		/**
		 * Initialize the Onscreen Display.
		 * This must be called from a valid GL context.
		 */
		void init(void);

		/**
		 * Shut down the Onscreen Display.
		 * This must be called from a valid GL context.
		 */
		void end(void);

		/**
		 * Draw the Onscreen Display.
		 * This must be called from a valid GL context.
		 */
		void draw(void);
};

}

#endif /* __GENS_SDL_OSDGL_HPP__ */
