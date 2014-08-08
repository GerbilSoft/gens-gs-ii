/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GLBackend.hpp: Common OpenGL backend.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#include <gens-qt4/config.gens-qt4.h>

// C includes.
#include <limits.h>

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
#include <QtGui/QWidget>
#include <QtGui/QColor>

namespace GensQt4
{

class GLBackend : public VBackend
{
	Q_OBJECT
	
	public:
		GLBackend(QWidget *parent, KeyHandlerQt *keyHandler = 0);
		virtual ~GLBackend();

#ifdef HAVE_GLEW
		/**
		 * Get a list of the OpenGL extensions in use.
		 * @return List of OpenGL extensions in use.
		 */
		static QStringList GLExtsInUse(void);
#endif /* HAVE_GLEW */

		/**
		 * Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osd_show_preview(int duration, const QImage& img);

	protected slots:
		/**
		 * Bilinear filter setting has changed.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 * @param newBilinearFilter (bool) New bilinear filter setting.
		 */
		virtual void bilinearFilter_changed_slot(const QVariant &newBilinearFilter);

		/**
		 * Pause Tint effect setting has changed.
		 * @param newPauseTint (bool) New pause tint effect setting.
		 */
		virtual void pauseTint_changed_slot(const QVariant &newPauseTint);

		/**
		 * Stretch mode setting has changed.
		 * @param newStretchMode (int) New stretch mode setting.
		 */
		void stretchMode_changed_slot(const QVariant &newStretchMode);

	protected:
		/**
		 * Called when OpenGL is initialized.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 */
		void glb_initializeGL(void);

		/**
		 * Window has been resized.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 * @param width Window width.
		 * @param height Window height.inline
		 */
		void glb_resizeGL(int width, int height);

		/**
		 * OpenGL paint event.
		 * NOTE: This function MUST be called from within an active OpenGL context!
		 */
		void glb_paintGL(void);

		/**
		 * Clear the preview image.
		 * TODO: Does this function need to be called from within an active OpenGL context?
		 */
		void glb_clearPreviewTex(void);

	private:
		// Window size.
		QSize m_winSize;

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

		// Main texture.
		GLuint m_tex;		// Texture ID.
		int m_colorComponents;	// Number of color components. (3 == RGB; 4 == BGRA)
		GLenum m_texFormat;	// Texture format. (GL_RGB, GL_BGRA)
		GLenum m_texType;	// Texture type. (GL_UNSIGNED_BYTE, etc.)
		QSize m_texSize;	// Texture size. (1x == 512x256 for power-of-two textures.)
		QSize m_texVisSize;	// Texture visible size. (1x == 320x240)

		// Stretch mode.
		QRectF m_stretchRectF;		// Current stretch coordinates.
		QSize m_stretchLastRes;		// Last MD screen resolution.
		void recalcStretchRectF(void);
		void recalcStretchRectF(StretchMode_t mode);

		// OSD texture.
		GLTex2D *m_texOsd;	// Texture containing U+0000 - U+00FF.
		GLuint m_glListOsd;	// Display list.
		QRectF m_rectfOsd;	// Projection rectangle.
		GLfloat m_osdVertex[256][8];	// Texture vertex array.

		// Reallocate Texture functions.
		void reallocTexture(void);
		void reallocTexOsd(void);

		// OSD functions.
		void printOsdText(void);
		void printOsdLine(int x, int y, const QString &msg);

		// OSD preview image.
		GLTex2D *m_texPreview;
		void showOsdPreview(void);

		// OSD font information.
		// TODO: Maybe make the font customizable in the future.
		static const int ms_Osd_chrW = 8;
		static const int ms_Osd_chrH = 16;

		// OpenGL Shader Manager.
		GLShaderManager m_shaderMgr;

		/**
		 * Set the current OpenGL color.
		 * @param color QColor.
		 */
		void glb_setColor(const QColor& color);
};

/**
 * Recalculate the stretch mode rectangle.
 * This version uses the current stretch mode.
 */
inline void GLBackend::recalcStretchRectF(void)
{
	recalcStretchRectF(stretchMode());
}

/**
 * Set the current OpenGL color.
 * @param color QColor.
 */
inline void GLBackend::glb_setColor(const QColor& color)
{
	glColor4ub(color.red(), color.green(), color.blue(), color.alpha());
}

}

#endif /* __GENS_QT4_VBACKEND_GLCOMMON_HPP__ */
