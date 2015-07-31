/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLTex.h: OpenGL texture struct.                                         *
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

#ifndef __GENS_SDL_GLTEX_H__
#define __GENS_SDL_GLTEX_H__

// TODO: Turn into an actual class?
// This is currently just a POD struct.

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// Byteswapping macros.
#include "libgens/Util/byteswap.h"

// GL_UNSIGNED_INT_8_8_8_8_REV is needed for native byte-order on PowerPC.
// When used with GL_BGRA, it's effectively the same as GL_ARGB.
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_BYTE
#endif

// MSVC ships with *ancient* GL headers.
// TODO: Use GLEW.
#if !defined(GL_BGRA) && defined(GL_BGRA_EXT)
#define GL_BGRA GL_BGRA_EXT
#endif

#if defined(GL_UNSIGNED_SHORT_1_5_5_5_REV) && \
    defined(GL_UNSIGNED_SHORT_5_6_5)
#define GL_HEADER_HAS_PACKED_PIXELS
#endif

#ifdef __cplusplus
extern "C" {
#endif

// OpenGL texture.
typedef struct _GLTex {
	GLuint name;		// Texture name.
	int components;		// Number of color components. (3 == RGB; 4 == BGRA)
	GLenum format;		// Texture format. (GL_RGB, GL_BGRA)
	GLenum type;		// Texture type. (GL_UNSIGNED_BYTE, etc.)
	// TODO: Size type?
	int texW, texH;		// Texture size. (1x == 512x256 for pow2 textures.)
	int texVisW, texVisH;	// Texture visible size. (1x == 320x240)
} GLTex;

#ifdef __cplusplus
}
#endif

#endif /* __GENS_SDL_GLTEX_H__ */
