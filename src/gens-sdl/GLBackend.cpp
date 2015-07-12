/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLBackend.hpp: SDL OpenGL rendeirng backend.                            *
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

#include "GLBackend.hpp"
#include "libgens/Util/MdFb.hpp"
using LibGens::MdFb;

// C includes.
#include <stdlib.h>

// Byteswapping macros.
#include "libgens/Util/byteswap.h"

// GL_UNSIGNED_INT_8_8_8_8_REV is needed for native byte-order on PowerPC.
// When used with GL_BGRA, it's effectively the same as GL_ARGB.
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_BYTE
#endif

namespace GensSdl {

GLBackend::GLBackend()
	: m_lastBpp(MdFb::BPP_MAX)
	, m_tex(0)
	, m_colorComponents(0)
	, m_texFormat(0)
	, m_texType(0)
	, m_texW(0), m_texH(0)
	, m_texVisW(0), m_texVisH(0)
	, m_prevMD_W(0), m_prevMD_H(0)
	, m_prevStretchMode(STRETCH_MAX)
	, m_winW(640), m_winH(480)
{
	// Default window size is 640x480.
	// GL must be initialized by the subclass.
}

GLBackend::~GLBackend()
{ }

/**
 * Set the SDL video source to an MdFb.
 * If nullptr, removes the SDL video source.
 * @param fb MdFb.
 */
void GLBackend::set_video_source(LibGens::MdFb *fb)
{
	if (m_fb == fb)
		return;

	// Unreference the current MdFb first.
	if (m_fb) {
		m_fb->unref();
		m_fb = nullptr;
	}

	if (fb) {
		m_fb = fb->ref();
	}

	// Reallocate the texture.
	reallocTexture();
}

/**
 * Update video.
 * @param fb_dirty If true, MdFb was updated.
 */
void GLBackend::update(bool fb_dirty)
{
	// TODO: makeCurrent()?

	// Copy the MdFb to the texture.
	if (m_fb && fb_dirty) {
		// Check if the bpp or texture size has changed.
		// TODO: texVisSizeChanged?
		if (m_fb->bpp() != m_lastBpp /*|| m_texVisSizeChanged*/) {
			// Bpp has changed. reallocate the texture.
			// VDP palettes will be recalculated on the next frame.
			reallocTexture();
		}

		// TODO: Apply effects.

		// Get the screen buffer.
		const GLvoid *screen;
		if (m_fb->bpp() != MdFb::BPP_32) {
			screen = m_fb->fb16();
		} else {
			screen = m_fb->fb32();
		}

		// Bind the texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);

		// TODO: This only works for 1x.
		// For other renderers, use non-MD screen buffer.

		// (Re-)Upload the texture.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_fb->pxPitch());
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 8); // TODO: 16 on amd64?

		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, 0,			// x/y offset
				m_texW, m_texH,		// width/height
				m_texFormat, m_texType, screen);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
	} else {
		// MD Screen isn't dirty.
		// Simply bind the texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);
	}

	// TODO: Enable shaders?

	// Clear the framebuffer first.
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// TODO:
	// - Aspect ratio constraint.
	// - Recalculate texRectF if the active display size changes.

	// Check if the MD resolution has changed.
	// If it has, recalculate the texture rectangle.
	if (m_stretchMode != m_prevStretchMode ||
	    m_fb->imgWidth() != m_prevMD_W ||
	    m_fb->imgHeight() != m_prevMD_H)
	{
		recalcTexRectF();
	}

	// Draw the texture.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	static const int vtx[4][2] = {{-1, 1}, {1, 1}, {1, -1}, {-1, -1}};
	glVertexPointer(2, GL_INT, 0, vtx);
	glTexCoordPointer(2, GL_DOUBLE, 0, m_texRectF);
	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// Subclass must swap the framebuffers.

	// VBackend is no longer dirty.
	clearDirty();
}

/**
 * Viewing area has been resized.
 * @param width Width.
 * @param height Height.
 */
void GLBackend::resize(int width, int height)
{
	// Save the window size for later.
	m_winW = width;
	m_winH = height;

	// TODO: Don't update the viewport until the next frame update?
	// TODO: makeCurrent()?

	// Initialize the OpenGL viewport.
	glViewport(0, 0, width, height);

	// Set the OpenGL projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// TODO: Aspect ratio constraint property.
	const bool aspectRatioConstraint = true;
	if (!aspectRatioConstraint) {
		// No aspect ratio constraint.
		glOrtho(-1, 1, -1, 1, -1, 1);
	} else {
		// Aspect ratio constraint.
		const double screenRatio = ((double)width / (double)height);
		const double texRatio = ((double)m_texVisW / (double)m_texVisH);

		if (screenRatio > texRatio) {
			// Screen is wider than the texture.
			const double ratio = (screenRatio / texRatio);
			glOrtho(-ratio, ratio, -1, 1, -1, 1);
		} else if (screenRatio < texRatio) {
			// Screen is taller than the texture.
			const double ratio = (texRatio / screenRatio);
			glOrtho(-1, 1, -ratio, ratio, -1, 1);
		} else {
			// Image has the correct aspect ratio.
			glOrtho(-1, 1, -1, 1, -1, 1);
		}
	}

	// Reset the GL model view.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/** OpenGL functions. **/

/**
 * Initialize OpenGL.
 * This must be called by the subclass constructor.
 */
void GLBackend::initGL(void)
{
	// TODO: makeCurrent()?

	// Disable some OpenGL functionality we don't care about.
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Enable face culling.
	// This disables drawing the backsides of polygons.
	// Also, set the front face to GL_CW.
	// TODO: GL_CCW is default; rework everything to use CCW instead?
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);

	// Initialize the GL viewport and projection.
	// TODO: Should width and height be function parameters,
	// which are then copied to m_winW and m_winH here?
	resize(m_winW, m_winH);

	// Allocate textures.
	reallocTexture();
}

/**
 * Shut down OpenGL.
 * This must be called by the subclass destructor.
 */
void GLBackend::endGL(void)
{
	// TODO: makeCurrent()?

	if (m_tex > 0) {
		glDeleteTextures(1, &m_tex);
		m_tex = 0;
	}
}

/**
 * Reallocate the OpenGL texture.
 */
void GLBackend::reallocTexture(void)
{
	// TODO: makeCurrent()?

	if (m_tex > 0) {
		glDeleteTextures(1, &m_tex);
	}

	if (!m_fb) {
		// No framebuffer.
		m_tex = 0;
		m_lastBpp = MdFb::BPP_MAX;
		return;
	}

	// Get the current color depth.
	m_lastBpp = m_fb->bpp();

	// Create and initialize a GL texture.
	// TODO: Add support for NPOT textures and/or GL_TEXTURE_RECTANGLE_ARB.
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// GL filtering.
	// TODO: Make it selectable: GL_LINEAR, GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Determine the texture format and type.
	// TODO: If using 15/16, make sure PACKED PIXELS are supported.
	switch (m_lastBpp) {
		case MdFb::BPP_15:
			m_colorComponents = 4;
			m_texFormat = GL_BGRA;
			m_texType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;

		case MdFb::BPP_16:
			m_colorComponents = 3;
			m_texFormat = GL_RGB;
			m_texType = GL_UNSIGNED_SHORT_5_6_5;
			break;

		case MdFb::BPP_32:
		default:
			m_colorComponents = 4;
			m_texFormat = GL_BGRA;
			m_texType = SDLGL_UNSIGNED_BYTE;
			break;
	}

	// TODO: Determine texture size based on MDP renderer.
	m_texVisW = m_fb->pxPerLine();
	m_texVisH = m_fb->numLines();
	m_texW = next_pow2s(m_texVisW);
	m_texH = next_pow2s(m_texVisH);

	// Allocate a memory buffer to use for texture initialization.
	// This will ensure that the entire texture is initialized to black.
	// (This fixes garbage on the last column when using the Fast Blur shader.)
	const size_t texSize = (m_texW * m_texH *
				(m_lastBpp == MdFb::BPP_32 ? 4 : 2));
	void *texBuf = calloc(1, texSize);

	// Allocate the texture.
	glTexImage2D(GL_TEXTURE_2D, 0,
			m_colorComponents,
			m_texW, m_texH,
			0,	// No border.
			m_texFormat, m_texType, texBuf);

	// Free the temporary texture buffer.
	free(texBuf);
	glDisable(GL_TEXTURE_2D);

	// Recalculate the texture rectangle.
	recalcTexRectF();
}

/**
 * Recalculate the texture rectangle.
 */
void GLBackend::recalcTexRectF(void)
{
	if (!m_fb)
		return;

	// Default to no stretch.
	double x = 0.0, y = 0.0;
	double w = (double)m_texVisW / (double)m_texW;
	double h = (double)m_texVisH / (double)m_texH;

	// Save the current MD screen resolution.
	m_prevMD_W = m_fb->imgWidth();
	m_prevMD_H = m_fb->imgHeight();
	m_prevStretchMode = m_stretchMode;

	if (m_stretchMode == STRETCH_H || m_stretchMode == STRETCH_FULL) {
		// Horizontal stretch.
		const int imgXStart = m_fb->imgXStart();
		if (imgXStart > 0) {
			// Less than 320 pixels wide.
			// Adjust horizontal stretch.
			x = (double)imgXStart / (double)m_texW;
			w -= x;
		}
	}

	if (m_stretchMode == STRETCH_V || m_stretchMode == STRETCH_FULL) {
		// Vertical stretch.
		const int imgYStart = m_fb->imgYStart();
		if (imgYStart > 0) {
			// Less than 240 pixels tall.
			// Adjust vertical stretch.
			y = (double)imgYStart / (double)m_texH;
			h -= y;
		}
	}

	// Set the texture rectangle coordinates.
	m_texRectF[0][0] = x;
	m_texRectF[0][1] = y;
	m_texRectF[1][0] = w;
	m_texRectF[1][1] = y;
	m_texRectF[2][0] = w;
	m_texRectF[2][1] = h;
	m_texRectF[3][0] = x;
	m_texRectF[3][1] = h;
}

}
