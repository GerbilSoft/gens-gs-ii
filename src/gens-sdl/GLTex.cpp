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
#include <cstdio>

// System byte order.
#include "libcompat/byteorder.h"

// GL_UNSIGNED_INT_8_8_8_8_REV is needed for native byte-order on PowerPC.
// When used with GL_BGRA, it's effectively the same as GL_ARGB.
#if SYS_BYTEORDER == SYS_BIG_ENDIAN
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_INT_8_8_8_8_REV
#else /* SYS_BYTEORDER == SYS_LIL_ENDIAN */
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_BYTE
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
	// Determine the internal texture format.
	const char *texFormat;
	switch (format) {
		case FMT_UNKNOWN:
		default:
			// Unknown format.
			dealloc();
			return -EINVAL;
		case FMT_XRGB1555:
			// 15-bit color requires OpenGL 1.2 or GL_APPLE_packed_pixels.
			// TODO: GL_RGB5 seems to work as an internal format.
			// https://www.opengl.org/registry/doc/glspec121_bookmarked.pdf
			this->intformat = GL_RGBA;
			this->format = GL_BGRA;
			this->type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			texFormat = "XRGB1555";
			break;
		case FMT_RGB565:
			// 16-bit color requires OpenGL 1.2 or GL_APPLE_packed_pixels.
			this->intformat = GL_RGB;
			this->format = GL_RGB;
			this->type = GL_UNSIGNED_SHORT_5_6_5;
			texFormat = "RGB565";
			break;
		case FMT_XRGB8888:
			// TODO: Verify that GL_BGRA is supported.
			// Pretty much everything supports it,
			// but you can never be sure...
			this->intformat = GL_RGBA;
			this->format = GL_BGRA;
			this->type = SDLGL_UNSIGNED_BYTE;
			texFormat = "XRGB8888";
			break;
		case FMT_ALPHA8:
			// TODO: Does GL_ALPHA8 work everywhere?
			this->intformat = GL_ALPHA8;
			this->format = GL_ALPHA;
			// TODO: Test this on PowerPC.
			// GL_ALPHA is single-component, so we probably
			// shouldn't use SDLGL_UNSIGNED_BYTE.
			this->type = GL_UNSIGNED_BYTE;
			texFormat = "ALPHA8";
			break;
	}

	// GL_BGRA isn't part of OpenGL 1.1.
	// COMMIT NOTE: MSVC 2010 didn't complain when it had "this->format = GL_BGRA".
	// Test again with /W4, and test newer versions?
	if (this->format == GL_BGRA) {
		// GL_BGRA requires either OpenGL 1.2 or GL_EXT_bgra.
		if (!GLEW_VERSION_1_2 && !GLEW_EXT_bgra) {
			fprintf(stderr, "WARNING: OpenGL 1.2 is not supported, and GL_EXT_bgra is missing.\n"
				"Texture format %s will not work correctly.", texFormat);
			dealloc();
			this->intformat = 0;
			this->format = 0;
			this->type = 0;
			return -EINVAL;
		}
	}

	// 15-bit/16-bit color isn't part of OpenGL 1.1.
	if (this->type == GL_UNSIGNED_SHORT_1_5_5_5_REV ||
	    this->type == GL_UNSIGNED_SHORT_5_6_5) {
		// Both 15-bit and 16-bit color require either OpenGL 1.2 or
		// the GL_APPLE_packed_pixels extension.
		// GL_EXT_packed_pixels has GL_UNSIGNED_SHORT_5_5_5_1_EXT,
		// but not GL_UNSIGNED_SHORT_1_5_5_5_REV, and it doesn't
		// have any 565 formats at all.
		// TODO: Regenerate GLEW to have GL_APPLE_packed_pixels.
		if (!GLEW_VERSION_1_2 /*&& !GLEW_APPLE_packed_pixels*/) {
			fprintf(stderr, "WARNING: OpenGL 1.2 is not supported, and GL_APPLE_packed_pixels is missing.\n"
				"Texture format %s will not work correctly.\n", texFormat);
			dealloc();
			this->intformat = 0;
			this->format = 0;
			this->type = 0;
			return -EINVAL;
		}
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
	// TODO: Use glClearBuffer()?
	// http://stackoverflow.com/questions/7195130/how-to-efficiently-initialize-texture-with-zeroes
	void *texBuf = nullptr;
	if (texW != texVisW || texH != texVisH) {
		// Texture size is not a power of two.
		// Initialize it here so filtering works properly.
		size_t texSize = texW * texH;
		if (this->format != GL_ALPHA) {
			if (this->type == SDLGL_UNSIGNED_BYTE) {
				texSize *= 4;
			} else {
				texSize *= 2;
			}
		}
		texBuf = calloc(1, texSize);
		memset(texBuf, 0, texSize);
	}

	// Allocate the texture.
	glTexImage2D(GL_TEXTURE_2D, 0,
			this->intformat,	// Internal format.
			texW, texH,		// width/height
			0,			// No border.
			this->format, this->type, texBuf);

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
 * @param pxPitch Pitch, in pixels.
 * @param data Image data.
 */
void GLTex::subImage2D(int w, int h, int pxPitch, const void *data)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, name);

	// Set pixel storage properties.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, pxPitch);
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
