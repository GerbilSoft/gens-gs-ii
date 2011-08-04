/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLBackend.hpp: Common OpenGL backend.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

// C includes.
#include <stdint.h>

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

// LibGens includes.
#include "libgens/Vdp/Vdp.hpp"
#include "libgens/Util/Timing.hpp"
#include "libgens/MD/EmuMD.hpp"

// Win32 requires GL/glext.h for OpenGL 1.2/1.3.
#if defined(Q_WS_MAC)
#include <OpenGL/glext.h>
#else
#include <GL/glext.h>
#endif

// WGL / GLX extensions.
#if defined(Q_WS_WIN)
#include <GL/wglew.h>
#elif defined(Q_WS_X11)
#include <GL/glxew.h>
#endif

namespace GensQt4
{

/**
 * GLBackend(): Initialize the common OpenGL backend.
 */
GLBackend::GLBackend(QWidget *parent, KeyHandlerQt *keyHandler)
	: VBackend(parent, keyHandler)
{
	// Initialize the OpenGL variables.
	m_tex = 0;		// Main texture.
	m_texOsd = 0;		// OSD texture.
	m_glListOsd = 0;	// OSD display list.
	m_texPreview = NULL;	// Preview image texture. (GlTex2D)
	
	// TODO: Have GensQGLWidget set the window size.
	m_winSize = QSize(320, 240);
}

GLBackend::~GLBackend()
{
	if (m_tex > 0)
		glDeleteTextures(1, &m_tex);
	if (m_texOsd)
		delete m_texOsd;
	if (m_glListOsd > 0)
		glDeleteLists(m_glListOsd, 1);
	
	// Delete the preview texture.
	glb_clearPreviewTex();
	
	// Shut down the OpenGL Shader Manager.
	m_shaderMgr.end();
}


#ifdef HAVE_GLEW
/**
 * GLExtsInUse(): Get a list of the OpenGL extensions in use.
 * @return List of OpenGL extensions in use.
 */
QStringList GLBackend::GLExtsInUse(void)
{
	QStringList exts;
	
	if (GLEW_EXT_bgra || GLEW_VERSION_1_2)
		exts.append(QLatin1String("GL_EXT_bgra"));
	
	// TODO: GLEW doesn't have GL_APPLE_packed_pixels.
	// Check if it exists manually.
	// For now, we're always assuming it exists on Mac OS X.
#ifdef Q_WS_MAC
	if (/*GLEW_APPLE_PACKED_PIXELS ||*/ GLEW_VERSION_1_2)
		exts.append(QLatin1String("GL_APPLE_packed_pixels"));
	else
#endif
	if (GLEW_EXT_packed_pixels || GLEW_VERSION_1_2)
		exts.append(QLatin1String("GL_EXT_packed_pixels"));
	
	// TODO: Vertical sync.
#if 0
#if defined(Q_WS_WIN)
	if (WGLEW_EXT_swap_control)
		exts.append(QLatin1String("WGL_EXT_swap_control"));
#elif defined(Q_WS_X11)
	if (GLXEW_EXT_swap_control)
		exts.append(QLatin1String("GLX_EXT_swap_control"));
	if (GLXEW_SGI_swap_control)
		exts.append(QLatin1String("GLX_SGI_swap_control"));
	// TODO: GLX_MESA_swap_control (not defined in GLEW)
	//if (GLXEW_MESA_swap_control)
		//exts.append(QLatin1String("GLX_MESA_swap_control"));
#elif defined(Q_WS_MAC)
	// TODO: Mac VSync.
#endif
#endif
	
#if 0
	// TODO: Rectangular texture.
	if (GLEW_ARB_texture_rectangle || GLEW_VERSION_2_0)
		exts.append(QLatin1String("GL_ARB_texture_rectangle"));
	else if (GLEW_EXT_texture_rectangle)
		exts.append(QLatin1String("GL_EXT_texture_rectangle"));
	else if (GLEW_NV_texture_rectangle)
		exts.append(QLatin1String("GL_NV_texture_rectangle"));
#endif
	
	// GL shader extensions from GLShaderManager.
	exts += GLShaderManager::GLExtsInUse();
	
	// Return the list of extensions.
	return exts;
}
#endif /* HAVE_GLEW */


/**
 * reallocTexture(): (Re-)Allocate the OpenGL texture.
 */
void GLBackend::reallocTexture(void)
{
	if (m_tex > 0)
		glDeleteTextures(1, &m_tex);
	
	// If we don't have an emulation context, don't allocate a texture for now.
	// TODO: Intro effects.
	if (!m_emuContext)
	{
		// Clear texture; set last bpp as invalid to force update.
		m_tex = 0;
		m_lastBpp = LibGens::VdpPalette::BPP_MAX;
		return;
	}
	
	// Create and initialize a GL texture.
	// TODO: Add support for NPOT textures and/or GL_TEXTURE_RECTANGLE_ARB.
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// GL filtering.
	const GLint filterMethod = (this->bilinearFilter() ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMethod);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMethod);
	
	// Determine the texture format and type.
	m_lastBpp = m_emuContext->m_vdp->m_palette.bpp();
	switch (m_lastBpp)
	{
		case LibGens::VdpPalette::BPP_15:
			m_colorComponents = 4;
			m_texFormat = GL_BGRA;
			m_texType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			break;
		
		case LibGens::VdpPalette::BPP_16:
			m_colorComponents = 3;
			m_texFormat = GL_RGB;
			m_texType = GL_UNSIGNED_SHORT_5_6_5;
			break;
		
		case LibGens::VdpPalette::BPP_32:
		default:
			m_colorComponents = 4;
			m_texFormat = GL_BGRA;
			m_texType = GLTEX2D_FORMAT_32BIT;
			break;
	}
	
#ifdef HAVE_GLEW
	// TODO: Show an OSD message.
	
	// GL_BGRA format requires GL_EXT_bgra.
	const bool hasExtBgra = (GLEW_EXT_bgra || GLEW_VERSION_1_2);
	if (m_texFormat == GL_BGRA && !hasExtBgra)
	{
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"GL_EXT_bgra is missing.");
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"15/32-bit color may not work properly.");
	}
	
	// 15-bit and 16-bit color requires GL_EXT_packed_pixels.
	// 32-bit color requires GL_EXT_packed_pixels on big-endian systems.
	// TODO: GLEW doesn't have GL_APPLE_packed_pixels.
	// Check if it exists manually.
	const bool hasExtPackedPixels = (GLEW_VERSION_1_2
						|| GLEW_EXT_packed_pixels
						/*|| GLEW_APPLE_packed_pixels*/
						);
	if (m_texType != GL_UNSIGNED_BYTE && !hasExtPackedPixels)
	{
#ifdef Q_WS_MAC
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"GL_APPLE_packed_pixels is missing.");
#else
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"GL_EXT_packed_pixels is missing.");
#endif
		LOG_MSG_ONCE(video, LOG_MSG_LEVEL_ERROR,
				"15/16-bit color may not work properly.");
	}
#endif /* HAVE_GLEW */
	
	// TODO: Determine size based on renderer.
	m_texVisSize = QSize(320, 240);
	m_texSize.setWidth(next_pow2s(m_texVisSize.width()));
	m_texSize.setHeight(next_pow2s(m_texVisSize.height()));
	
	// Allocate a memory buffer to use for texture initialization.
	// This will ensure that the entire texture is initialized to black.
	// (This fixes garbage on the last column when using the Fast Blur shader.)
	const size_t texSize = (m_texSize.width() * m_texSize.height() *
				(m_lastBpp == LibGens::VdpPalette::BPP_32 ? 4 : 2));
	void *texBuf = calloc(1, texSize);
	
	// Allocate the texture.
	glTexImage2D(GL_TEXTURE_2D, 0,
		     m_colorComponents,
		     m_texSize.width(), m_texSize.height(),
		     0,		// No border.
		     m_texFormat, m_texType, texBuf);
	
	// Free the temporary texture buffer.
	free(texBuf);
	
	// Disable TEXTURE_2D.
	glDisable(GL_TEXTURE_2D);
	
	// Recalculate the stretch mode rectangle.
	recalcStretchRectF();
	
	// Texture is dirty.
	m_vbDirty = true;
}


/**
 * initializeGL(): Called when OpenGL is initialized.
 * This function MUST be called from within an active OpenGL context!
 */
void GLBackend::glb_initializeGL(void)
{
	// OpenGL initialization.
	
	// Disable various OpenGL functionality.
	glDisable(GL_DEPTH_TEST);	// Depth buffer.
	glDisable(GL_BLEND);		// Blending.
	
	// Enable face culling.
	// This disables drawing the backsides of polygons.
	// Also, set the front face to GL_CW.
	// TODO: GL_CCW is default; rework everything to use CCW instead?
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	
#ifdef HAVE_GLEW
	// Initialize GLEW.
	GLenum err = glewInit();
	if (err == GLEW_OK)
	{
		// GLEW initialized successfully.
		
		// Initialize the OpenGL Shader Manager.
		m_shaderMgr.init();
	}
#endif
	
	// Initialize the GL viewport and projection.
	// TODO: Get the actual window size.
	glb_resizeGL(320, 240);
	
	// Allocate textures.
	reallocTexture();	// Main texture.
	reallocTexOsd();	// OSD texture.
	
	// Generate a display list for the OSD.
	m_glListOsd = glGenLists(1);
}


/**
 * reallocTexOsd(): (Re-)Allocate the OSD texture.
 */
void GLBackend::reallocTexOsd(void)
{
	if (m_texOsd)
		delete m_texOsd;
	
	// Load the OSD texture.
	// TODO: Handle the case where the image isn't found.
	QImage imgOsd = QImage(QLatin1String(":/gens/vga-charset.png"));
	QVector<QRgb> colorTable = imgOsd.colorTable();
	
	if (!colorTable.isEmpty())
	{
		// Image uses a palette. Apply a color key.
		// Background color (black) will be changed to transparent.
		// TODO: Allow all 0x000000, or just opaque 0xFF000000?
		for (int i = 0; i < colorTable.size(); i++)
		{
			if (colorTable[i] == 0xFF000000)
			{
				// Found black. Make it transparent.
				colorTable[i] = 0x00000000;
				break;
			}
		}
		
		imgOsd.setColorTable(colorTable);
	}
	else if (!imgOsd.hasAlphaChannel())
	{
		// Image doesn't have an alpha channel.
		// Convert it to 32-bit RGBA and apply a color key.
		// Background color (black) will be changed to transparent.
		imgOsd = imgOsd.convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly);
		
		// TODO: Byte ordering on big-endian systems.
		// TODO: Allow all 0x000000, or just opaque 0xFF000000?
		// TODO: Handle images that don't have a color table.
		// TODO: Images with odd widths will not have the last column updated properly.
		for (int y = 0; y < imgOsd.height(); y++)
		{
			QRgb *scanline = reinterpret_cast<QRgb*>(imgOsd.scanLine(y));
			for (int x = imgOsd.width() / 2; x != 0; x--, scanline += 2)
			{
				if (*scanline == 0xFF000000)
					*scanline = 0x00000000;
				if (*(scanline+1) == 0xFF000000)
					*(scanline+1) = 0x00000000;
			}
			
			// TODO: Make this work on images with odd widths?
		}
	}
	
	// Create a new OpenGL texture.
	m_texOsd = new GLTex2D(imgOsd);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_texOsd->tex());
	
	// Set texture parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// Texture filtering.
	// TODO: Should we use linear filtering for the OSD text?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glDisable(GL_TEXTURE_2D);
}


/**
 * resizeGL(): Window has been resized.
 * This function MUST be called from within an active OpenGL context!
 * @param width Window width.
 * @param height Window height.
 */
void GLBackend::glb_resizeGL(int width, int height)
{
	// Save the width and height for later.
	m_winSize.setWidth(width);
	m_winSize.setHeight(height);
	
	glViewport(0, 0, width, height);
	
	// Set the OpenGL projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Aspect ratio constraint.
	if (!aspectRatioConstraint())
	{
		// No aspect ratio constraint.
		glOrtho(-1, 1, -1, 1, -1, 1);
		m_rectfOsd = QRectF(0, 0, 320, 240);
	}
	else
	{
		// Aspect ratio constraint.
		const int w3 = (width * 3);
		const int h4 = (height * 4);
		
		if (w3 > h4)
		{
			// Image is wider than 4:3.
			const double ratio = ((double)w3 / (double)h4);
			glOrtho(-ratio, ratio, -1, 1, -1, 1);
			
			// Adjust the OSD rectangle.
			m_rectfOsd.setTop(0);
			m_rectfOsd.setHeight(240);
			
			const double osdWidth = (320.0 * ratio);
			m_rectfOsd.setLeft(-((osdWidth - 320.0) / 2.0));
			m_rectfOsd.setWidth(osdWidth);
		}
		else if (w3 < h4)
		{
			// Image is taller than 4:3.
			const double ratio = ((double)h4 / (double)w3);
			glOrtho(-1, 1, -ratio, ratio, -1, 1);
			
			// Adjust the OSD rectangle.
			m_rectfOsd.setLeft(0);
			m_rectfOsd.setWidth(320);
			
			const double osdHeight = (240.0 * ratio);
			m_rectfOsd.setTop(-((osdHeight - 240.0) / 2.0));
			m_rectfOsd.setHeight(osdHeight);
		}
		else
		{
			// Image has the correct aspect ratio.
			glOrtho(-1, 1, -1, 1, -1, 1);
			m_rectfOsd = QRectF(0, 0, 320, 240);
		}
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Mark the video buffer and OSD as dirty.
	setVbDirty();
	setOsdListDirty();
}


/**
 * paintGL(): OpenGL paint event.
 * NOTE: This function MUST be called from within an active OpenGL context!
 */
void GLBackend::glb_paintGL(void)
{
#if 0
	// If nothing's dirty, don't paint anything.
	// TODO: Verify this on all platforms.
	// NOTE: Causes problems on Windows XP; disabled for now.
	if (!m_vbDirty && !m_mdScreenDirty)
		return;
#endif
	
	/**
	 * bFromMD: If this is true after all effects are applied,
	 * use LibGens::VDP_Rend::MD_Screen[] directly.
	 * Otherwise, use m_intScreen[].
	 */
	bool bFromMD = true;
	
	/**
	 * bDoPausedEffect: Indicates if the paused effect should be applied.
	 * This is true if we're manually paused and the pause tint is enabled.
	 */
	const bool bDoPausedEffect = (isManualPaused() && pauseTint());
	
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (!m_emuContext)
	{
		// No emulation context.
		// TODO: Intro effects.
		showOsdPreview();	// Display the OSD preview image.
		printOsdText();		// Print the OSD text to the screen.
		m_vbDirty = false;	// Video backend is no longer dirty.
		return;
	}
	
	if (hasAspectRatioConstraintChanged())
	{
		// Aspect ratio constraint has changed.
		glb_resizeGL(m_winSize.width(), m_winSize.height());
		resetAspectRatioConstraintChanged();
	}
	
	if (m_mdScreenDirty)
	{
		// MD_Screen is dirty.
		
		// Check if the Bpp has changed.
		const LibGens::VdpPalette::ColorDepth bpp = m_emuContext->m_vdp->m_palette.bpp();
		if (bpp != m_lastBpp)
		{
			// Bpp has changed. Reallocate the texture.
			// VDP palettes will be recalculated on the next frame.
			reallocTexture();
		}
		
		/** START: Apply effects. **/
		
		if (isRunning())
		{
			// Emulation is running. Check if any effects should be applied.
			
			/** Software rendering path. **/
			
			// If Fast Blur is enabled, update the Fast Blur effect.
			// NOTE: Shader version is only used if we're not paused manually.
			if (fastBlur() && (bDoPausedEffect || !m_shaderMgr.hasFastBlur()))
			{
				updateFastBlur(bFromMD);
				bFromMD = false;
			}
			
			// If emulation is manually paused, update the pause effect.
			if (bDoPausedEffect && !m_shaderMgr.hasPaused())
			{
				// Paused, but no shader is available.
				// Apply the effect in software.
				updatePausedEffect(bFromMD);
				bFromMD = false;
			}
		}
		
		// Determine which screen buffer should be used for video output.
		// TODO: Optimize this!
		GLvoid *screen;
		LibGens::MdFb *src_fb = (bFromMD
				? &m_emuContext->m_vdp->MD_Screen
				: m_intScreen);
		if (bpp != LibGens::VdpPalette::BPP_32)
			screen = src_fb->fb16();
		else
			screen = src_fb->fb32();
		
		/** END: Apply effects. **/
		
		// Bind the texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		
		// TODO: This only works for 1x.
		// For other renderers, use non-MD screen buffer.
		
		// (Re-)Upload the texture.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 336);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
		
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, 0,						// x/y offset
				m_texVisSize.width(), m_texVisSize.height(),	// width/height
				m_texFormat, m_texType, screen);
		
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
		
		// Texture is no longer dirty.
		m_mdScreenDirty = false;
	}
	else
	{
		// MD Screen isn't dirty.
		// Simply bind the texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);
	}
	
	// Enable shaders, if necessary.
	if (isRunning())
	{
		if (m_shaderMgr.hasFastBlur() && fastBlur() && !bDoPausedEffect)
		{
			// Enable the Fast Blur shader.
			// NOTE: Shader version is only used if we're not paused manually.
			m_shaderMgr.setFastBlur(true);
		}
		else if (m_shaderMgr.hasPaused() && bDoPausedEffect)
		{
			// Enable the Paused shader.
			m_shaderMgr.setPaused(true);
		}
	}
	
	// Check if the MD resolution has changed.
	// If it has, recalculate the stretch mode rectangle.
	const QSize mdResCur(m_emuContext->m_vdp->GetHPix(),
			     m_emuContext->m_vdp->GetVPix());
	if (mdResCur != m_stretchLastRes)
		recalcStretchRectF();
	
	// Draw the texture.
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glBegin(GL_QUADS);
	glTexCoord2d(m_stretchRectF.x(), m_stretchRectF.y());
	glVertex2i(-1, 1);
	glTexCoord2d(m_stretchRectF.width(), m_stretchRectF.y());
	glVertex2i(1, 1);
	glTexCoord2d(m_stretchRectF.width(), m_stretchRectF.height());
	glVertex2i(1, -1);
	glTexCoord2d(m_stretchRectF.x(), m_stretchRectF.height());
	glVertex2i(-1, -1);
	glEnd();
	
	// Disable shaders, if necessary.
	if (isRunning())
	{
		if (m_shaderMgr.hasFastBlur() && fastBlur() && !bDoPausedEffect)
		{
			// Disable the Fast Blur shader.
			// NOTE: Shader version is only used if we're not paused manually.
			m_shaderMgr.setFastBlur(false);
		}
		else if (m_shaderMgr.hasPaused() && bDoPausedEffect)
		{
			// Disable the Paused shader.
			m_shaderMgr.setPaused(false);
		}
	}
	
	// Finish up drawing.
	glDisable(GL_TEXTURE_2D);	// Disable 2D textures.
	showOsdPreview();		// Display the OSD preview image.
	printOsdText();			// Print the OSD text to the screen.
	m_vbDirty = false;		// Video backend is no longer dirty.
}


/**
 * recalcStretchRectF(): Recalculate the stretch mode rectangle.
 * @param mode Stretch mode.
 */
void GLBackend::recalcStretchRectF(StretchMode_t mode)
{
	if (m_emuContext)
	{
		// Store the current MD screen resolution.
		m_stretchLastRes.setWidth(m_emuContext->m_vdp->GetHPix());
		m_stretchLastRes.setHeight(m_emuContext->m_vdp->GetVPix());
	}
	else
	{
		// No emulation context.
		// Assume 320x240 image for now.
		m_stretchLastRes.setWidth(320);
		m_stretchLastRes.setWidth(240);
	}
	
	// Default to no stretch.
	m_stretchRectF = QRectF(
		0.0,		// X coordinate.
		0.0,		// Y coordinate.
		((double)m_texVisSize.width() / (double)m_texSize.width()),	// Width.
		((double)m_texVisSize.height() / (double)m_texSize.height())	// Height.
		);
	
	// Don't apply any stretch parameters if:
	// - stretching is disabled
	// - emulation isn't running
	// - no emulation context is present (TODO: Combine isRunning() with m_emuContext?)
	if (mode == STRETCH_NONE || !isRunning() || !m_emuContext)
		return;
	
	// Horizontal stretch.
	if (mode == STRETCH_H || mode == STRETCH_FULL)
	{
		// Horizontal stretch.
		const int h_pix_begin = m_emuContext->m_vdp->GetHPixBegin();
		if (h_pix_begin > 0)
		{
			// Less than 320 pixels wide.
			// Adjust horizontal stretch.
			// NOTE: Width is adjusted automatically by QRectF when setting X.
			m_stretchRectF.setX((double)h_pix_begin / (double)m_texSize.width());
			//m_stretchRectF.setWidth(img_dest.width() - img_dest.x());
		}
	}
	
	// Vertical stretch.
	if (mode == STRETCH_V || mode == STRETCH_FULL)
	{
		// Vertical stretch.
		int v_pix = (240 - m_emuContext->m_vdp->GetVPix());
		if (v_pix > 0)
		{
			// Less than 240 pixels tall.
			// Adjust vertical stretch.
			// NOTE: Height is adjusted automatically by QRectF when setting Y.
			v_pix /= 2;
			m_stretchRectF.setY((double)v_pix / (double)m_texSize.height());
			//m_stretchRectF.setHeight(img_dest.height() - img_dest.y());
		}
	}
}


/**
 * printOsdText(): Print the OSD text to the screen.
 */
void GLBackend::printOsdText(void)
{
	// Print text to the screen.
	// TODO:
	// * renderText() doesn't support wordwrapping.
	// * renderText() doesn't properly handle newlines.
	// * fm.boundingRect() doesn't seem to handle wordwrapping correctly, either.
	
	if (!isOsdListDirty())
	{
		// OSD message list isn't dirty.
		// Call the display list.
		glCallList(m_glListOsd);
		return;
	}
	
	// OSD message list is dirty.
	// Create a new GL display list.
	glNewList(m_glListOsd, GL_COMPILE_AND_EXECUTE);
	
	// Set pixel matrices.
	// Assuming 320x240 for 1x text rendering.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//glOrtho(0, 320, 240, 0, -1.0f, 1.0f);
	glOrtho(m_rectfOsd.left(), m_rectfOsd.right(),
		m_rectfOsd.bottom(), m_rectfOsd.top(),
		-1.0f, 1.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	// Trick to fix up pixel alignment.
	// See http://basic4gl.wikispaces.com/2D+Drawing+in+OpenGL
	glTranslatef(0.375f, 0.375f, 0.0f);
	
	// Enable 2D textures.
	glEnable(GL_TEXTURE_2D);
	
	// Enable GL blending.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Bind the OSD texture.
	glBindTexture(GL_TEXTURE_2D, m_texOsd->tex());
	glBegin(GL_QUADS);	// Start drawing quads.
	
	// Text shadow color.
	// TODO: Make this customizable?
	static const QColor clShadow(Qt::black);
	
	int y = (240 - ms_Osd_chrH);
	const double curTime = LibGens::Timing::GetTimeD();
	
	// Check if the FPS should be drawn.
	if (isRunning() && !isPaused() && osdFpsEnabled())
	{
		const QString sFps = QString::number(fpsAvg(), 'f', 1);
		
		// Next line.
		y -= ms_Osd_chrH;
		
		// TODO: Make the drop shadow optional or something.
		glb_setColor(clShadow);
		printOsdLine(ms_Osd_chrW+1, y+1, sFps);
		glb_setColor(osdFpsColor());
		printOsdLine(ms_Osd_chrW, y, sFps);
	}
	
	// If messages are enabled, print them on the screen.
	if (osdMsgEnabled())
	{
		// NOTE: QList internally uses an array of pointers.
		// We can use array indexing instead of iterators.
		for (int i = (m_osdList.size() - 1); i >= 0; i--)
		{
			if (curTime >= m_osdList[i].endTime)
			{
				// Message duration has elapsed.
				// Remove the message from the list.
				m_osdList.removeAt(i);
				continue;
			}
			
			const QString &msg = m_osdList[i].msg;
			
			// Next line.
			y -= ms_Osd_chrH;
			
			// TODO: Make the drop shadow optional or something.
			glb_setColor(clShadow);
			printOsdLine(ms_Osd_chrW+1, y+1, msg);
			glb_setColor(osdMsgColor());	// TODO: Per-message colors?
			printOsdLine(ms_Osd_chrW, y, msg);
		}
	}
	
	// Check if there's any recording status messages.
	if (!m_osdRecList.isEmpty())
	{
		// Recording status messages are present.
		// Print at the upper-right of the screen.
		y = ms_Osd_chrH;
		const QColor clStopped(Qt::white);
		
		QColor clRec;
		QString sRec;
		if (isRunning())
		{
			if (isPaused())
			{
				clRec = QColor(Qt::white);
				sRec = QChar(0x25CF);
				sRec += QChar(0xF8FE);
			}
			else
			{
				clRec = QColor(Qt::red);
				sRec = QChar(0x25CF);
			}
		}
		else
		{
			clRec = QColor(Qt::white);
			sRec = QChar(0x25A0);
		}
		
		QString msg;
		foreach(const RecOsd &recOsd, m_osdRecList)
		{
			msg = (recOsd.isRecording ? sRec : QChar(0x25A0)) +
				recOsd.component + QChar(L' ');
			
			// Format the duration.
			int secs = (recOsd.duration / 1000);
			const int mins = (secs / 60);
			secs %= 60;
			msg += QString::fromLatin1("%1:%2").arg(mins).arg(QString::number(secs), 2, QChar(L'0'));
			
			// Calculate the message width.
			const int msgW = ((msg.size() + 1) * ms_Osd_chrW);
			const int x = (320 - msgW);
			
			// TODO: Make the drop shadow optional or something.
			glb_setColor(clShadow);
			printOsdLine(x+1, y+1, msg);
			glb_setColor(recOsd.isRecording ? clRec : clStopped);	// TODO: Per-message colors?
			printOsdLine(x, y, msg);
			
			// Next line.
			y += ms_Osd_chrH;
		}
	}
	
	// We're done drawing.
	glEnd();
	
	// Reset the GL state.
	glb_setColor(QColor(Qt::white));	// Reset the color.
	glDisable(GL_BLEND);			// Disable GL blending.
	glDisable(GL_TEXTURE_2D);		// Disable 2D textures.
	
	// Restore the matrices.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	// Finished creating the GL display list.
	glEndList();
	clearOsdListDirty();	// OSD message list is no longer dirty.
}


/**
 * printOsdLine(): Print a line of text on the screen.
 * NOTE: This should ONLY be called from printOsdText()!
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param msg Line of text.
 */
void GLBackend::printOsdLine(int x, int y, const QString &msg)
{
	// TODO: Font information.
	// TODO: Wordwrapping.
	
	for (int i = 0; i < msg.size(); i++, x += ms_Osd_chrW)
	{
		uint16_t chr = msg[i].unicode();
		if (chr > 0xFF)
		{
			// Unicode characters over U+00FF are not supported right now.
			// TODO: Replacement character.
			
			// cp437 control characters (0x00-0x20) are now supported.
			// Check if this is a control character.
			// TODO: Optimize this!
			switch (chr)
			{
				case 0x263A:	chr = 0x01; break;
				case 0x263B:	chr = 0x02; break;
				case 0x2665:	chr = 0x03; break;
				case 0x2666:	chr = 0x04; break;
				case 0x2663:	chr = 0x05; break;
				case 0x2660:	chr = 0x06; break;
				case 0x2022:	chr = 0x07; break;
				case 0x2508:	chr = 0x08; break;
				case 0x25CB:	chr = 0x09; break;
				case 0x25D9:	chr = 0x0A; break;
				case 0x2642:	chr = 0x0B; break;
				case 0x2640:	chr = 0x0C; break;
				case 0x266A:	chr = 0x0D; break;
				case 0x266B:	chr = 0x0E; break;
				case 0x263C:	chr = 0x0F; break;
				case 0x25BA:	chr = 0x10; break;
				case 0x25C4:	chr = 0x11; break;
				case 0x2195:	chr = 0x12; break;
				case 0x203C:	chr = 0x13; break;
				//case 0x00B6:	chr = 0x14; break;	// This is part of cp1252...
				//case 0x00A7:	chr = 0x15; break;	// This is part of cp1252...
				case 0x25AC:	chr = 0x16; break;
				case 0x21A8:	chr = 0x17; break;
				case 0x2191:	chr = 0x18; break;
				case 0x2193:	chr = 0x19; break;
				case 0x2192:	chr = 0x1A; break;
				case 0x2190:	chr = 0x1B; break;
				case 0x221F:	chr = 0x1C; break;
				case 0x2194:	chr = 0x1D; break;
				case 0x2582:	chr = 0x1E; break;
				case 0x25BC:	chr = 0x1F; break;
				
				// VCR symbols.
				case 0x25CF:	chr = 0x80; break;	// Record. (BLACK CIRCLE)
				case 0xF8FE:	chr = 0x81; break;	// Pause. (Private Use Area)
				case 0x25A0:	chr = 0x82; break;	// Stop. (BLACK SQUARE)
				
				default:	chr = 0; break;
			}
			
			if (chr == 0)
				continue;
		}
		
		// Calculate the texture coordinates.
		const GLfloat tx1 = ((float)(chr & 0xF) / 16.0f);
		const GLfloat ty1 = ((float)((chr & 0xF0) >> 4) / 16.0f);
		const GLfloat tx2 = (tx1 + (1.0f / 16.0f));
		const GLfloat ty2 = (ty1 + (1.0f / 16.0f));
		
		// Draw the texture.
		// NOTE: glBegin() / glEnd() are called in printOsdText().
		glTexCoord2f(tx1, ty1);
		glVertex2i(x, y);
		glTexCoord2f(tx2, ty1);
		glVertex2i(x+ms_Osd_chrW, y);
		glTexCoord2f(tx2, ty2);
		glVertex2i(x+ms_Osd_chrW, y+ms_Osd_chrH);
		glTexCoord2f(tx1, ty2);
		glVertex2i(x, y+ms_Osd_chrH);
	}
}


/**
 * glb_clearPreviewTex(): Clear the preview image.
 * TODO: Does this function need to be called from within an active OpenGL context?
 */
void GLBackend::glb_clearPreviewTex(void)
{
	delete m_texPreview;
	m_texPreview = NULL;
}


/**
 * osd_show_preview(): Show a preview image on the OSD.
 * @param duration Duration for the preview image to appaer, in milliseconds.
 * @param img Image to show.
 */
void GLBackend::osd_show_preview(int duration, const QImage& img)
{
	// Call the base function first.
	VBackend::osd_show_preview(duration, img);
	
	// Delete the preview texture to force a refresh.
	glb_clearPreviewTex();
}


/**
 * showOsdPreview(): Show the OSD preview image.
 */
void GLBackend::showOsdPreview(void)
{
	if (!m_preview_show || m_preview_img.isNull())
	{
		// Don't show the preview image.
		delete m_texPreview;
		m_texPreview = NULL;
		return;
	}
	
	// Check if the duration has elapsed.
	const double curTime = LibGens::Timing::GetTimeD();
	if (curTime >= m_preview_endTime)
	{
		// Preview duration has elapsed.
		// TODO: Combine this code with the !m_preview_show code.
		delete m_texPreview;
		m_texPreview = NULL;
		return;
	}
	
	// Enable 2D textures.
	glEnable(GL_TEXTURE_2D);
	
	// Show the preview image.
	if (!m_texPreview)
	{
		// Create the texture for the preview image.
		m_texPreview = new GLTex2D(m_preview_img);
	}
	
	// Bind the texture.
	glBindTexture(GL_TEXTURE_2D, m_texPreview->tex());	
	
	// Calculate the destination coordinates.
	// TODO: Precalculate?
	// TODO: Reposition based on stretch mode and current display resolution.
	const GLdouble x1 = (-1.0 + (0.0625 * 3.0 / 4.0)), y1 = (1.0 - 0.0625);
	GLdouble x2, y2;
	
	// Calculate (x2, y2) based on stretch mode.
	
	// Horizontal stretch.
	if (stretchMode() == STRETCH_H || stretchMode() == STRETCH_FULL)
	{
		x2 = x1 + 0.5;
	}
	else
	{
		x2 = x1 + std::min(((m_texPreview->img_w() / 320.0) * 0.5), 0.5);
	}
	
	// Vertical stretch.
	if (stretchMode() == STRETCH_V || stretchMode() == STRETCH_FULL)
	{
		y2 = y1 - 0.5;
	}
	else
	{
		y2 = y1 - std::min(((m_texPreview->img_h() / 240.0) * 0.5), 0.5);
	}
	
	// Draw the texture.
	// TODO: Determine where to display it and what size to use.
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2d(x1, y1);
	glTexCoord2d(m_texPreview->pow2_w(), 0);
	glVertex2d(x2, y1);
	glTexCoord2d(m_texPreview->pow2_w(), m_texPreview->pow2_h());
	glVertex2d(x2, y2);
	glTexCoord2d(0, m_texPreview->pow2_h());
	glVertex2d(x1, y2);
	glEnd();
	
	// Disable 2D textures.
	glEnable(GL_TEXTURE_2D);
}


/**
 * bilinearFilter_changed_slot(): Bilinear filter setting has changed.
 * NOTE: This function MUST be called from within an active OpenGL context!
 * @param newBilinearFilter (bool) New bilinear filter setting.
 */
void GLBackend::bilinearFilter_changed_slot(const QVariant& newBilinearFilter)
{
	if (m_tex > 0)
	{
		// Bind the texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		
		// Set the texture filter setting.
		const GLint filterMethod = (newBilinearFilter.toBool() ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMethod);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMethod);
		
		// Finished updating the texture.
		glDisable(GL_TEXTURE_2D);
	}
	
	// Call VBackend's bilinearFilter_changed_slot().
	VBackend::bilinearFilter_changed_slot(newBilinearFilter);
}


/**
 * pauseTint_changed_slot(): Pause Tint effect setting has changed.
 * @param newPauseTint (bool) New pause tint effect setting.
 */
void GLBackend::pauseTint_changed_slot(const QVariant& newPauseTint)
{
	if (!m_shaderMgr.hasPaused() &&
	    (isRunning() && isPaused()))
	{
		// Emulation is running, but is currently paused.
		// Shader isn't available.
		// Update the MD screen.
		m_mdScreenDirty = true;
	}
	
	// Call VBackend's pauseTint_changed_slot().
	VBackend::pauseTint_changed_slot(newPauseTint);
}


/**
 * stretchMode_changed_slot(): Stretch mode setting has changed.
 * @param newStretchMode (int) New stretch mode setting.
 */
void GLBackend::stretchMode_changed_slot(const QVariant& newStretchMode)
{
	StretchMode_t stretch = (StretchMode_t)newStretchMode.toInt();
	
	// Verify that the new stretch mode is valid.
	// TODO: New ConfigItem subclass for StretchMode_t.
	if ((stretch < STRETCH_NONE) || (stretch > STRETCH_FULL))
	{
		// Invalid stretch mode.
		// Reset to default.
		stretchMode_reset();
		return;
	}
	
	// Recalculate the stretch mode rectangle.
	recalcStretchRectF(stretch);
	
	// Call VBackend's stretchMode_changed_slot().
	VBackend::stretchMode_changed_slot(newStretchMode);
}

}
