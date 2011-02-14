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

/**
 * GLCommon is intended to be used as a private member
 * of a class that inherits from VBackend.
 */

#ifndef __GENS_QT4_VBACKEND_GLCOMMON_HPP__
#define __GENS_QT4_VBACKEND_GLCOMMON_HPP__

#include <config.h>

// OpenGL Shader Manager.
// This file MUST be before any other GL includes,
// since it includes the GLEW headers.
#include "GLShaderManager.hpp"

// OpenGL GL_TEXTURE_2D wrapper.
#include "GLTex2D.hpp"

// Video Backend.
#include "VBackend.hpp"

// Qt includes.
#include <QtCore/QStringList>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtGui/QColor>

namespace GensQt4
{

class GLBackend : public VBackend
{
	public:
		GLBackend();
		virtual ~GLBackend();
		
#ifdef HAVE_GLEW
		/**
		 * GLExtsInUse(): Get a list of the OpenGL extensions in use.
		 * @return List of OpenGL extensions in use.
		 */
		static QStringList GLExtsInUse(void);
#endif /* HAVE_GLEW */
		
		/**
		 * osd_show_preview(): Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osd_show_preview(int duration, const QImage& img);
		
		/**
		 * setBilinearFilter(): Set the bilinear filter setting.inline
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 * @param newBilinearFilter True to enable bilinear filtering; false to disable it.
		 */
		void setBilinearFilter(bool newBilinearFilter);
		
		/**
		 * setPauseTint(): Set the Pause Tint effect setting.
		 * @param newFastBlur True to enable Pause Tint; false to disable it.
		 */
		void setPauseTint(bool newPauseTint);
	
	protected:
		/**
		 * glb_initializeGL(): Called when OpenGL is initialized.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 */
		void glb_initializeGL(void);
		
		/**
		 * glb_resizeGL(): Window has been resized.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 * @param width Window width.
		 * @param height Window height.inline
		 */
		void glb_resizeGL(int width, int height);
		
		/**
		 * paintGL(): OpenGL paint event.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 */
		void glb_paintGL(void);
		
		/**
		 * glb_clearPreviewTex(): Clear the preview image.
		 * TODO: Does this function need to be called from within an active OpenGL context?
		 */
		void glb_clearPreviewTex();
	
	private:
		// Window size.
		QSize m_winSize;
		
		// Main texture.
		GLuint m_tex;		// Texture ID.
		int m_colorComponents;	// Number of color components. (3 == RGB; 4 == BGRA)
		GLenum m_texFormat;	// Texture format. (GL_RGB, GL_BGRA)
		GLenum m_texType;	// Texture type. (GL_UNSIGNED_BYTE, etc.)
		
		// OSD texture.
		GLTex2D *m_texOsd;	// Texture containing U+0000 - U+00FF.
		GLuint m_glListOsd;	// Display list.
		QRectF m_rectfOsd;	// Projection rectangle.
		
		// Reallocate Texture functions.
		void reallocTexture(void);
		void reallocTexOsd(void);
		
		// OSD functions.
		void printOsdText(void);
		void printOsdLine(int x, int y, const QString &msg);
		
		// OSD preview image.
		GLTex2D *m_texPreview;
		void showOsdPreview(void);
		
		// OpenGL Shader Manager.
		GLShaderManager m_shaderMgr;
		
		/**
		 * glb_setColor(): Set the current OpenGL color.
		 * @param color QColor.
		 */
		void glb_setColor(const QColor& color);
};


/**
 * glb_setColor(): Set the current OpenGL color.
 * @param color QColor.
 */
inline void GLBackend::glb_setColor(const QColor& color)
{
	glColor4ub(color.red(), color.green(), color.blue(), color.alpha());
}

}

#endif /* __GENS_QT4_VBACKEND_GLCOMMON_HPP__ */
