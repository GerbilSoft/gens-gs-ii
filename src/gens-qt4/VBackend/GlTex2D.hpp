/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GlTex2D.hpp: GL_TEXTURE_2D wrapper class.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __GENS_QT4_VBACKEND_GLTEX2D_HPP__
#define __GENS_QT4_VBACKEND_GLTEX2D_HPP__

#include <config.h>

// C includes.
#include <limits.h>

#ifdef HAVE_GLEW
// GL Extension Wrangler.
#include <GL/glew.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// Byteswapping macros.
#include "libgens/Util/byteswap.h"

// GL_UNSIGNED_INT_8_8_8_8_REV is needed for native byte-order on PowerPC.
// When used with GL_BGRA, it's effectively the same as GL_ARGB.
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
#define GLTEX2D_FORMAT_32BIT GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define GLTEX2D_FORMAT_32BIT GL_UNSIGNED_BYTE
#endif

// Qt includes.
#include <QtGui/QImage>

namespace GensQt4
{

class GlTex2D
{
	public:
		GlTex2D();
		~GlTex2D();
		
		void setImage(const QImage& img);
		
		inline GLuint tex(void) const
			{ return m_tex; }
		
		// Pow2-adjusted image size.
		inline GLdouble pow2_w(void) const
			{ return m_pow2_w; }
		inline GLdouble pow2_h(void) const
			{ return m_pow2_h; }
		
		// Original image size.
		inline int img_w(void) const
			{ return m_img_w; }
		inline int img_h(void) const
			{ return m_img_h; }
	
	private:
		GLuint m_tex;
		bool m_hasPackedPixels;
		
		// Pow2-adjusted image size.
		GLdouble m_pow2_w;
		GLdouble m_pow2_h;
		
		// Original image size.
		int m_img_w;
		int m_img_h;
		
		// Image format.
		GLenum m_format;
		GLenum m_type;
		int m_components;
		
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
};

}

#endif /* __GENS_QT4_VBACKEND_GLTEX2D_HPP__ */
