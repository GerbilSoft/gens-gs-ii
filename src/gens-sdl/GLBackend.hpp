/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLBackend.hpp: OpenGL rendeirng backend.                                *
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

#ifndef __GENS_SDL_GLBACKEND_HPP__
#define __GENS_SDL_GLBACKEND_HPP__

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <climits>

// OpenGL
#include <GL/gl.h>

// Video Backend.
#include "VBackend.hpp"
// LibGens includes.
#include "libgens/Util/MdFb.hpp"

namespace GensSdl {

class GLBackend : public VBackend {
	public:
		GLBackend();
		virtual ~GLBackend();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		GLBackend(const GLBackend &);
		GLBackend &operator=(const GLBackend &);

	public:
		/**
		 * Set the video source to an MdFb.
		 * If nullptr, removes the video source.
		 * @param fb MdFb.
		 */
		virtual void set_video_source(LibGens::MdFb *fb) final;

		/**
		 * Update video.
		 * @param fb_dirty If true, MdFb was updated.
		 */
		virtual void update(bool fb_dirty) override;

		/**
		 * Viewing area has been resized.
		 * @param width Width.
		 * @param height Height.
		 */
		virtual void resize(int width, int height) override;

	private:
		// Last MdFb bpp.
		LibGens::MdFb::ColorDepth m_lastBpp;

		// OpenGL texture.
		GLuint m_tex;			// Texture name.
		int m_colorComponents;		// Number of color components. (3 == RGB; 4 == BGRA)
		GLenum m_texFormat;		// Texture format. (GL_RGB, GL_BGRA)
		GLenum m_texType;		// Texture type. (GL_UNSIGNED_BYTE, etc.)
		// TODO: Size type?
		int m_texW, m_texH;		// Texture size. (1x == 512x256 for pow2 textures.)
		int m_texVisW, m_texVisH;	// Texture visible size. (1x == 320x240)

		// Texture rectangle.
		GLdouble m_texRectF[4][2];

		// Previous stretch mode parameters.
		int m_prevMD_W, m_prevMD_H;
		StretchMode_t m_prevStretchMode;

		// Find the next highest power of two. (signed integers)
		// http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
		template <class T>
		static inline T next_pow2s(T k)
		{
			k--;
			for (int i = 1; i < (int)(sizeof(T)*CHAR_BIT); i <<= 1)
				k = k | k >> i;
			return k + 1;
		}

	protected:
		// Window size.
		// TODO: Accessors?
		int m_winW, m_winH;

	protected:
		/**
		 * Initialize OpenGL.
		 * This must be called by the subclass constructor.
		 */
		void initGL(void);

		/**
		 * Shut down OpenGL.
		 * This must be called by the subclass destructor.
		 */
		void endGL(void);

		/**
		 * Reallocate the OpenGL texture.
		 */
		void reallocTexture(void);

		/**
		 * Recalculate the texture rectangle.
		 */
		void recalcTexRectF(void);
};

}

#endif /* __GENS_SDL_GLBACKEND_HPP__ */
