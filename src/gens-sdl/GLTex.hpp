/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLTex.hpp: OpenGL texture wrapper.                                      *
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

#ifndef __GENS_SDL_GLTEX_HPP__
#define __GENS_SDL_GLTEX_HPP__

// C includes. (C++ namespace)
#include <climits>

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

namespace GensSdl {

class GLTex
{
	public:
		GLTex();
		~GLTex();

	public:
		GLuint name;		// Texture name.

		// Internal GL texture format.
		enum Format {
			FMT_UNKNOWN,	// Unknown (not initialized)
			FMT_XRGB1555,	// 15-bit RGB (555)
			FMT_RGB565,	// 16-bit RGB (565)
			FMT_XRGB8888,	// 32-bit RGB (888)

			FMT_MAX
		};

		/**
		 * Allocate the texture.
		 * @param format Texture format.
		 * @param w Width.
		 * @param h Height.
		 * @return 0 on success; non-zero on error.
		 */
		int alloc(Format format, int w, int h);

		/**
		 * Deallocate the texture.
		 */
		void dealloc(void);

		/**
		 * Upload a sub-image.
		 * Image data must be in the format allocated in alloc().
		 * @param w Width.
		 * @param h Height.
		 * @param pitch Pitch, in bytes.
		 * @param data Image data.
		 */
		void subImage2D(int w, int h, int pitch, const void *data);

	protected:
		GLenum components;	// Number of color components.
		GLenum format;		// Texture format. (GL_RGB, GL_BGRA)
		GLenum type;		// Texture type. (GL_UNSIGNED_BYTE, etc.)

	public:
		// TODO: Accessors.
		// TODO: Size type?
		int texW, texH;		// Texture size. (1x == 512x256 for pow2 textures.)
		int texVisW, texVisH;	// Texture visible size. (1x == 320x240)

	protected:
		// TODO: Move to GLTexPrivate?
		// Find the next highest power of two. (signed integers)
		// http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
		template <class T>
		static inline T next_pow2s(T k) {
			k--;
			for (int i = 1; i < (int)(sizeof(T)*CHAR_BIT); i <<= 1)
				k = k | k >> i;
			return k + 1;
		}
};

}

#endif /* __GENS_SDL_GLTEX_HPP__ */
