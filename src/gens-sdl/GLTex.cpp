/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLTex.cpp: OpenGL texture wrapper.                                      *
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

#include "GLTex.hpp"

// C includes. (C++ namespace)
#include <cerrno>
#include <cstdlib>
#include <cstring>

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

// NOTE: These constants are provided by OpenGL 1.2,
// *not* GL_EXT_packed_pixels.
#if defined(GL_UNSIGNED_SHORT_1_5_5_5_REV) && \
    defined(GL_UNSIGNED_SHORT_5_6_5)
#define GL_HEADER_HAS_GL_1_2_PACKED_PIXELS
#endif

namespace GensSdl {

GLTex::GLTex()
	: name(0)
	, format(0)
	, type(0)
	, texW(0), texH(0)
	, texVisW(0), texVisH(0)
{ }

GLTex::~GLTex()
{
	// NOTE: If the GL context isn't active,
	// this might fail.
	dealloc();
}

/**
 * Allocate the texture.
 * @param format Texture format.
 * @param w Width.
 * @param h Height.
 * @return 0 on success; non-zero on error.
 */
int GLTex::alloc(Format format, int w, int h)
{
	// BIG COMMIT NOTE: Third parameter of glTexImage2D()
	// is NOT number of components; it's internal format,
	// which should be the same as format.

	// Determine the internal texture format.
	switch (format) {
		case FMT_UNKNOWN:
		default:
			// Unknown format.
			dealloc();
			return -EINVAL;
#ifdef GL_HEADER_HAS_GL_1_2_PACKED_PIXELS
		// TODO: Verify that OpenGL 1.2 is supported.
		// TODO: GL_RGB5 seems to work as an internal format.
		// https://www.opengl.org/registry/doc/glspec121_bookmarked.pdf
		case FMT_XRGB1555:
			// TODO: Store as GL_RGB5 internally?
			this->intformat = GL_RGBA;
			this->format = GL_BGRA;
			this->type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;
		case FMT_RGB565:
			this->intformat = GL_RGB;
			this->format = GL_RGB;
			this->type = GL_UNSIGNED_SHORT_5_6_5;
			break;
#else /* !GL_HEADER_HAS_GL_1_2_PACKED_PIXELS */
		case FMT_XRGB1555:
		case FMT_RGB565:
			// OpenGL 1.2 is required for 15-bit and 16-bit color.
			// NOTE: GL_EXT_packed_pixels could work for 15-bit.
			// NOTE: GL_APPLE_packed_pixels could work for 15-bit and 16-bit.
			// TODO: Error code?
			dealloc();
			return -EINVAL;
#endif /* GL_HEADER_HAS_GL_1_2_PACKED_PIXELS */
		case FMT_XRGB8888:
			// TODO: Verify that GL_BGRA is supported.
			// Pretty much everything supports it,
			// but you can never be sure...
			this->intformat = GL_RGBA;
			this->format = GL_BGRA;
			this->type = SDLGL_UNSIGNED_BYTE;
			break;
	}

	// Create and initialize a GL texture.
	if (name == 0) {
		glGenTextures(1, &name);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// GL filtering.
	// TODO: Make it selectable: GL_LINEAR, GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	texVisW = w;
	texVisH = h;
	texW = next_pow2s(texVisW);
	texH = next_pow2s(texVisH);

	// If the texture size is a power of two,
	// we don't need to clear it because the
	// caller will initialize the entire texture.
	void *texBuf = nullptr;
	if (texW != texVisW || texH != texVisH) {
		// Texture size is not a power of two.
		// Initialize it here so filtering works properly.
		const size_t texSize = (texW * texH *
				(type == SDLGL_UNSIGNED_BYTE ? 4 : 2));
		texBuf = calloc(1, texSize);
		memset(texBuf, 0x44, texSize);
	}

	// Allocate the texture.
	glTexImage2D(GL_TEXTURE_2D, 0,
			this->intformat,	// Internal format.
			texW, texH,		// width/height
			0,			// No border.
			this->format, SDLGL_UNSIGNED_BYTE, texBuf);

	// Free the temporary texture buffer.
	free(texBuf);
	glDisable(GL_TEXTURE_2D);
	return 0;
}

void GLTex::dealloc(void)
{
	if (name > 0) {
		glDeleteTextures(1, &name);
		name = 0;
	}
}

/**
 * Upload a sub-image.
 * Image data must be in the format allocated in alloc().
 * @param w Width.
 * @param h Height.
 * @param pitch Pitch, in bytes.
 * @param data Image data.
 */
void GLTex::subImage2D(int w, int h, int pitch, const void *data)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, name);

	// Set pixel storage properties.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);	// TODO: 16 on amd64?

	// Upload the sub-image.
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0, 0,		// x/y offset
			w, h,		// width/height
			this->format, this->type, data);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 0);

	glDisable(GL_TEXTURE_2D);
}

}
