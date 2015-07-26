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

// C includes. (C++ namespace)
#include <cstdlib>
#include <climits>
#include <cstdio>

// Onscreen Display.
#include "OsdGL.hpp"

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

namespace GensSdl {

class GLBackendPrivate {
	public:
		GLBackendPrivate(GLBackend *q);
		~GLBackendPrivate();

	private:
		friend class GLBackend;
		GLBackend *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		GLBackendPrivate(const GLBackendPrivate &);
		GLBackendPrivate &operator=(const GLBackendPrivate &);

	public:
		// Last MdFb bpp.
		LibGens::MdFb::ColorDepth lastBpp;

		// OpenGL texture.
		GLuint tex;		// Texture name.
		int colorComponents;	// Number of color components. (3 == RGB; 4 == BGRA)
		GLenum texFormat;	// Texture format. (GL_RGB, GL_BGRA)
		GLenum texType;		// Texture type. (GL_UNSIGNED_BYTE, etc.)
		// TODO: Size type?
		int texW, texH;		// Texture size. (1x == 512x256 for pow2 textures.)
		int texVisW, texVisH;	// Texture visible size. (1x == 320x240)

		// Texture rectangle.
		GLdouble texRectF[4][2];

		// Previous stretch mode parameters.
		int prevMD_W, prevMD_H;
		VBackend::StretchMode_t prevStretchMode;

		// Find the next highest power of two. (signed integers)
		// http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
		template <class T>
		static inline T next_pow2s(T k) {
			k--;
			for (int i = 1; i < (int)(sizeof(T)*CHAR_BIT); i <<= 1)
				k = k | k >> i;
			return k + 1;
		}

		// Onscreen Display.
		OsdGL *osd;

	public:
		/**
		 * Reallocate the OpenGL texture.
		 */
		void reallocTexture(void);

		/**
		 * Recalculate the texture rectangle.
		 */
		void recalcTexRectF(void);
};

/** GLBackendPrivate **/

GLBackendPrivate::GLBackendPrivate(GLBackend *q)
	: q(q)
	, lastBpp(MdFb::BPP_MAX)
	, tex(0)
	, colorComponents(0)
	, texFormat(0)
	, texType(0)
	, texW(0), texH(0)
	, texVisW(0), texVisH(0)
	, prevMD_W(0), prevMD_H(0)
	, prevStretchMode(VBackend::STRETCH_MAX)
	, osd(new OsdGL())
{ }

GLBackendPrivate::~GLBackendPrivate()
{
	delete osd;
}

/**
 * Reallocate the OpenGL texture.
 */
void GLBackendPrivate::reallocTexture(void)
{
	// TODO: makeCurrent()?

	if (tex > 0) {
		glDeleteTextures(1, &tex);
	}

	MdFb *fb = q->m_fb;
	if (!fb) {
		// No framebuffer.
		tex = 0;
		lastBpp = MdFb::BPP_MAX;
		return;
	}

	// Get the current color depth.
	lastBpp = fb->bpp();

	// Determine the texture format and type.
	// TODO: If using 15/16, make sure PACKED PIXELS are supported.
	switch (lastBpp) {
#ifdef GL_HEADER_HAS_PACKED_PIXELS
		// TODO: Verify that packed pixels is actually supported using GLEW.
		case MdFb::BPP_15:
			colorComponents = 4;
			texFormat = GL_BGRA;
			texType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;

		case MdFb::BPP_16:
			colorComponents = 3;
			texFormat = GL_RGB;
			texType = GL_UNSIGNED_SHORT_5_6_5;
			break;
#else /* !GL_HEADER_HAS_PACKED_PIXELS */
		case MdFb::BPP_15:
		case MdFb::BPP_16:
			// GL_EXT_packed_pixels / GL_APPLE_packed_pixels
			// is required for 15-bit and 16-bit color.
			// TODO: Error code?
			tex = 0;
			lastBpp = MdFb::BPP_MAX;
			return;
#endif /* GL_HEADER_HAS_PACKED_PIXELS */

		case MdFb::BPP_32:
		default:
			colorComponents = 4;
			texFormat = GL_BGRA;
			texType = SDLGL_UNSIGNED_BYTE;
			break;
	}

	// Create and initialize a GL texture.
	// TODO: Add support for NPOT textures and/or GL_TEXTURE_RECTANGLE_ARB.
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// GL filtering.
	// TODO: Make it selectable: GL_LINEAR, GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// TODO: Determine texture size based on MDP renderer.
	texVisW = fb->pxPerLine();
	texVisH = fb->numLines();
	texW = next_pow2s(texVisW);
	texH = next_pow2s(texVisH);

	// Allocate a memory buffer to use for texture initialization.
	// This will ensure that the entire texture is initialized to black.
	// (This fixes garbage on the last column when using the Fast Blur shader.)
	const size_t texSize = (texW * texH *
				(lastBpp == MdFb::BPP_32 ? 4 : 2));
	void *texBuf = calloc(1, texSize);

	// Allocate the texture.
	glTexImage2D(GL_TEXTURE_2D, 0,
			colorComponents,
			texW, texH,
			0,	// No border.
			texFormat, texType, texBuf);

	// Free the temporary texture buffer.
	free(texBuf);
	glDisable(GL_TEXTURE_2D);

	// Recalculate the texture rectangle.
	recalcTexRectF();
}

/**
 * Recalculate the texture rectangle.
 */
void GLBackendPrivate::recalcTexRectF(void)
{
	MdFb *fb = q->m_fb;
	if (!fb) {
		// No framebuffer.
		return;
	}

	// Default to no stretch.
	double x = 0.0, y = 0.0;
	double w = (double)texVisW / (double)texW;
	double h = (double)texVisH / (double)texH;

	// Save the current MD screen resolution.
	prevMD_W = fb->imgWidth();
	prevMD_H = fb->imgHeight();
	prevStretchMode = q->m_stretchMode;

	if (q->m_stretchMode == VBackend::STRETCH_H ||
	    q->m_stretchMode == VBackend::STRETCH_FULL)
	{
		// Horizontal stretch.
		const int imgXStart = fb->imgXStart();
		if (imgXStart > 0) {
			// Less than 320 pixels wide.
			// Adjust horizontal stretch.
			x = (double)imgXStart / (double)texW;
			w -= x;
		}
	}

	if (q->m_stretchMode == VBackend::STRETCH_V ||
	    q->m_stretchMode == VBackend::STRETCH_FULL)
	{
		// Vertical stretch.
		const int imgYStart = fb->imgYStart();
		if (imgYStart > 0) {
			// Less than 240 pixels tall.
			// Adjust vertical stretch.
			y = (double)imgYStart / (double)texH;
			h -= y;
		}
	}

	// Set the texture rectangle coordinates.
	texRectF[0][0] = x;
	texRectF[0][1] = y;
	texRectF[1][0] = w;
	texRectF[1][1] = y;
	texRectF[2][0] = w;
	texRectF[2][1] = h;
	texRectF[3][0] = x;
	texRectF[3][1] = h;
}

/** GLBackend **/

GLBackend::GLBackend()
	: d(new GLBackendPrivate(this))
	, m_winW(640), m_winH(480)
{
	// Default window size is 640x480.
	// GL must be initialized by the subclass.
}

GLBackend::~GLBackend()
{
	endGL();
	delete d;
}

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
	d->reallocTexture();
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
		if (m_fb->bpp() != d->lastBpp /*|| d->texVisSizeChanged*/) {
			// Bpp has changed. reallocate the texture.
			// VDP palettes will be recalculated on the next frame.
			d->reallocTexture();
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
		glBindTexture(GL_TEXTURE_2D, d->tex);

		// TODO: This only works for 1x.
		// For other renderers, use non-MD screen buffer.

		// (Re-)Upload the texture.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_fb->pxPitch());
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 8); // TODO: 16 on amd64?

		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, 0,					// x/y offset
				m_fb->pxPerLine(), m_fb->numLines(),	// width/height
				d->texFormat, d->texType, screen);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
	} else {
		// MD Screen isn't dirty.
		// Simply bind the texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, d->tex);
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
	if (m_stretchMode != d->prevStretchMode ||
	    m_fb->imgWidth() != d->prevMD_W ||
	    m_fb->imgHeight() != d->prevMD_H)
	{
		d->recalcTexRectF();
	}

	// Draw the texture.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	static const int vtx[4][2] = {{-1, 1}, {1, 1}, {1, -1}, {-1, -1}};
	glVertexPointer(2, GL_INT, 0, vtx);
	glTexCoordPointer(2, GL_DOUBLE, 0, d->texRectF);
	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// Draw the OSD.
	d->osd->draw();

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
		const double texRatio = ((double)d->texVisW / (double)d->texVisH);

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
	d->reallocTexture();

	// Initialize the OSD.
	d->osd->init();
}

/**
 * Shut down OpenGL.
 * This must be called by the subclass destructor.
 */
void GLBackend::endGL(void)
{
	// TODO: makeCurrent()?

	// Shut down the OSD.
	d->osd->end();

	if (d->tex > 0) {
		glDeleteTextures(1, &d->tex);
		d->tex = 0;
	}
}

/** Onscreen Display functions. **/

/**
 * Print a message to the Onscreen Display.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message. (printf-formatted; UTF-8)
 * @param ap Format arguments.
 */
void GLBackend::osd_vprintf(const int duration, const utf8_str *msg, va_list ap)
{
	// TODO: printf() it here or in OsdGL?
	char buf[2048];
	vsnprintf(buf, sizeof(buf), msg, ap);
	d->osd->print(duration, buf);
}

}
