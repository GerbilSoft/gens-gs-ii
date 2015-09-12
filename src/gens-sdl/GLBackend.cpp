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

// Effects.
#include "libgens/Effects/PausedEffect.hpp"
#include "libgens/Effects/FastBlur.hpp"
using LibGens::PausedEffect;
using LibGens::FastBlur;

// C includes. (C++ namespace)
#include <cstdlib>
#include <climits>
#include <cstdio>

// OpenGL (GLEW)
#include <GL/glew.h>

// Onscreen Display.
#include "OsdGL.hpp"

// GL Texture wrpaper.
#include "GLTex.hpp"

// GL shaders.
#include "GLShaderFastBlur.hpp"

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
		GLTex tex;

		// Texture rectangle.
		GLdouble texRectF[4][2];

		// Previous stretch mode parameters.
		int prevMD_W, prevMD_H;
		VBackend::StretchMode_t prevStretchMode;
		bool prevAspectRatioConstraint;

		// Onscreen Display.
		OsdGL *osd;

		// GL shaders.
		GLShaderFastBlur *fastBlurShader;

	public:
		/**
		 * Reallocate the OpenGL texture.
		 */
		void reallocTexture(void);

		/**
		 * Recalculate the texture rectangle.
		 */
		void recalcTexRectF(void);

		/**
		 * Recalculate the aspect ratio.
		 */
		void recalcAspectRatio(void);

		/**
		 * Start applying software-based framebuffer effects.
		 *
		 * If any effects require framebuffer manipulation,
		 * m_int_fb will be allocated and returned.
		 *
		 * @return Framebuffer in use. (m_int_fb or m_fb)
		 */
		const MdFb *applySoftwareEffects(void);

		/**
		 * Start applying shader effects.
		 */
		void startShaderEffects(void);

		/**
		 * Stop applying shader effects.
		 */
		void stopShaderEffects(void);
};

/** GLBackendPrivate **/

GLBackendPrivate::GLBackendPrivate(GLBackend *q)
	: q(q)
	, lastBpp(MdFb::BPP_MAX)
	, prevMD_W(0), prevMD_H(0)
	, prevStretchMode(VBackend::STRETCH_MAX)
	, prevAspectRatioConstraint(true)
	, osd(new OsdGL())
	, fastBlurShader(new GLShaderFastBlur())
{ }

GLBackendPrivate::~GLBackendPrivate()
{
	delete osd;
	delete fastBlurShader;
}

/**
 * Reallocate the OpenGL texture.
 */
void GLBackendPrivate::reallocTexture(void)
{
	// TODO: makeCurrent()?

	// Initialize GLEW.
	// TODO: Initialize this in the main program, not here?
	// TODO: Multi-context?
	static bool glew_initialized = false;
	if (!glew_initialized) {
		int ret = glewInit();
		if (ret != GLEW_OK) {
			// TODO: Error handling.
			// Return an error code?
			fprintf(stderr, "GLEW initialization failed: %d\n", ret);
			//return ret;
		}
		glew_initialized = true;
	}

	MdFb *fb = q->m_fb;
	if (!fb) {
		// No framebuffer.
		tex.name = 0;
		lastBpp = MdFb::BPP_MAX;
		return;
	}

	// Get the current color depth.
	lastBpp = fb->bpp();

	// Determine the texture format and type.
	GLTex::Format format;
	switch (lastBpp) {
		case MdFb::BPP_15:
			format = GLTex::FMT_XRGB1555;
			break;
		case MdFb::BPP_16:
			format = GLTex::FMT_RGB565;
			break;
		case MdFb::BPP_32:
		default:
			format = GLTex::FMT_XRGB8888;
			break;
	}

	// Allocate the GL texture.
	tex.alloc(format, fb->pxPerLine(), fb->numLines());

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
	double w = tex.ratioW();
	double h = tex.ratioH();

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
			x = (double)imgXStart / (double)tex.texW;
			w -= (2*x);
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
			y = (double)imgYStart / (double)tex.texH;
			h -= (2*y);
		}
	}

	// Set the texture rectangle coordinates.
	// NOTE: toCoords() adds X and Y to the second set of coordinate pairs.
	// To adjust for this, we have to multiply the W and H differences
	// by two above.
	GLTex::toCoords(texRectF, x, y, w, h);
}

/**
 * Recalculate the aspect ratio.
 */
void GLBackendPrivate::recalcAspectRatio(void)
{
	// Set the OpenGL projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	prevAspectRatioConstraint = q->aspectRatioConstraint();
	if (!prevAspectRatioConstraint) {
		// No aspect ratio constraint.
		// TODO: Center and/or left-align the OSD instead of
		// stretching it, or enlarge the visible area?
		glOrtho(-1, 1, -1, 1, -1, 1);
		osd->setDisplayOffset(0.0, 0.0);
	} else {
		// Aspect ratio constraint.
		// TODO: Use integer arithmetic for ratio comparisons?
		const double screenRatio = ((double)q->m_winW / (double)q->m_winH);
		const double texRatio = ((double)tex.texVisW / (double)tex.texVisH);
		// OSD offsets.
		// TODO: Optimize these calculations.
		double offset_x, offset_y;

		if (screenRatio > texRatio) {
			// Screen is wider than the texture.
			const double ratio = (screenRatio / texRatio);
			glOrtho(-ratio, ratio, -1, 1, -1, 1);
			// Adjust the OSD rectangle.
			offset_x = ((tex.texVisW * ratio) - tex.texVisW) / 2;
			offset_y = 0.0;
		} else if (screenRatio < texRatio) {
			// Screen is taller than the texture.
			const double ratio = (texRatio / screenRatio);
			glOrtho(-1, 1, -ratio, ratio, -1, 1);
			// Adjust the OSD rectangle.
			offset_x = 0.0;
			offset_y = ((tex.texVisH * ratio) - tex.texVisH) / 2;
		} else {
			// Image has the correct aspect ratio.
			glOrtho(-1, 1, -1, 1, -1, 1);
			offset_x = 0.0;
			offset_y = 0.0;
		}

		// Set the OSD offsets.
		osd->setDisplayOffset(offset_x, offset_y);
	}

	// Reset the GL model view.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/**
 * Start applying software-based framebuffer effects.
 *
 * If any effects require framebuffer manipulation,
 * m_int_fb will be allocated and returned.
 *
 * @return Framebuffer in use. (m_int_fb or m_fb)
 */
const MdFb *GLBackendPrivate::applySoftwareEffects(void)
{
	// TODO: Shader version of Fast Blur.
	MdFb *fb = q->m_fb;	// FB to use.
	bool isIntFb = false;

	if (q->m_fastBlur  && !fastBlurShader->isUsable()) {
		// Make sure we have an internal framebuffer.
		if (!q->m_int_fb) {
			q->m_int_fb = new MdFb();
		}
		fb = q->m_int_fb;

		// Apply the paused effect.
		if (isIntFb) {
			FastBlur::DoFastBlur(q->m_int_fb);
		} else {
			FastBlur::DoFastBlur(q->m_int_fb, q->m_fb);
		}
		isIntFb = true;
	}

	if (q->m_pausedEffect) {
		// Make sure we have an internal framebuffer.
		if (!q->m_int_fb) {
			q->m_int_fb = new MdFb();
		}
		fb = q->m_int_fb;

		// Apply the paused effect.
		if (isIntFb) {
			PausedEffect::DoPausedEffect(q->m_int_fb);
		} else {
			PausedEffect::DoPausedEffect(q->m_int_fb, q->m_fb);
		}
		isIntFb = true;
	}

	return fb;
}

/**
 * Start applying shader effects.
 */
void GLBackendPrivate::startShaderEffects(void)
{
	if (q->m_fastBlur && fastBlurShader->isUsable()) {
		// Enable the Fast Blur effect.
		fastBlurShader->enable();
	}
}

/**
 * Stop applying shader effects.
 */
void GLBackendPrivate::stopShaderEffects(void)
{
	if (q->m_fastBlur && fastBlurShader->isUsable()) {
		// Disable the Fast Blur effect.
		fastBlurShader->disable();
	}
}

/** GLBackend **/

GLBackend::GLBackend()
	: super()
	, d(new GLBackendPrivate(this))
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
	if (m_fb && (fb_dirty || isForceFbDirty())) {
		// Check if the bpp or texture size has changed.
		// TODO: texVisSizeChanged?
		if (m_fb->bpp() != d->lastBpp /*|| d->texVisSizeChanged*/) {
			// Bpp has changed. reallocate the texture.
			// VDP palettes will be recalculated on the next frame.
			d->reallocTexture();
		}

		// Apply software framebuffer effects.
		const MdFb *fb = d->applySoftwareEffects();

		// Get the screen buffer.
		const GLvoid *screen;
		if (fb->bpp() != MdFb::BPP_32) {
			screen = fb->fb16();
		} else {
			screen = fb->fb32();
		}

		// (Re-)Upload the texture.
		d->tex.subImage2D(fb->pxPerLine(), fb->numLines(),
				fb->pxPitch(), screen);
	}

	// Bind the texture.
	// NOTE: Already done by d->tex.subImage2D(),
	// but the function disables GL_TEXTURE_2D when it's done.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, d->tex.name);

	// Enable shader effects.
	d->startShaderEffects();

	// Clear the framebuffer first.
	// TODO: Change to black once we're done debugging.
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
	if (m_aspectRatioConstraint != d->prevAspectRatioConstraint) {
		d->recalcAspectRatio();
	}

	// Draw the texture.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	static const int vtx[4][2] = {{-1, 1}, {1, 1}, {1, -1}, {-1, -1}};
	glVertexPointer(2, GL_INT, 0, vtx);
	glTexCoordPointer(2, GL_DOUBLE, 0, d->texRectF);
	glDrawArrays(GL_QUADS, 0, 4);

	// Disable shader effects.
	d->stopShaderEffects();

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

	// Recalculate the aspect ratio.
	d->recalcAspectRatio();
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

	// Initialize the shaders.
	d->fastBlurShader->init();
}

/**
 * Shut down OpenGL.
 * This must be called by the subclass destructor.
 */
void GLBackend::endGL(void)
{
	// TODO: makeCurrent()?

	// Shut down the shaders.
	d->fastBlurShader->end();

	// Shut down the OSD.
	d->osd->end();

	if (d->tex.name > 0) {
		glDeleteTextures(1, &d->tex.name);
		d->tex.name = 0;
	}
}

/** Onscreen Display functions. **/

/**
 * Are any OSD messages currently onscreen?
 * @return True if OSD messages are onscreen; false if not.
 */
bool GLBackend::has_osd_messages(void) const
{
	return d->osd->hasMessages();
}

/**
 * Process OSD messages.
 * This usually only needs to be called if the emulator is paused.
 * @return True if OSD messages were processed; false if not.
 */
bool GLBackend::process_osd_messages(void)
{
	bool ret = d->osd->processMessages();
	if (ret) {
		setDirty();
	}
	return ret;
}

/**
 * Print a message to the Onscreen Display.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message. (printf-formatted; UTF-8)
 */
void GLBackend::osd_print(int duration, const char *msg)
{
	// Print the message to the OSD.
	d->osd->print(duration, msg);
	// VBackend is dirty.
	setDirty();
}

/**
 * Display a preview image on the Onscreen Display.
 * @param duration Duration for the preview image to appear, in milliseconds.
 * @param img_data Image data. (If nullptr, or internal data is nullptr, hide the current image.)
 */
void GLBackend::osd_preview_image(int duration, const _Zomg_Img_Data_t *img_data)
{
	d->osd->preview_image(duration, img_data);
}

}
