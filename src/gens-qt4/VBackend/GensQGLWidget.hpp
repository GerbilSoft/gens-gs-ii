/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQGLWidget.hpp: QGLWidget subclass.                                  *
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

#include <config.h>

#ifndef __GENS_QT4_GENSQGLWIDGET_HPP__
#define __GENS_QT4_GENSQGLWIDGET_HPP__

#include "VBackend.hpp"

// C includes.
#include <limits.h>

// OpenGL Shader Manager.
// This file MUST be before any other GL includes,
// since it includes the GLEW headers.
#include "GLShaderManager.hpp"

// Qt includes.
#include <QtOpenGL/QGLWidget>

// Key Handler.
#include "Input/KeyHandlerQt.hpp"

// LibGens includes.
#include "libgens/MD/VdpRend.hpp"

namespace GensQt4
{

class GensQGLWidget : public QGLWidget, public VBackend
{
	Q_OBJECT
	
	public:
		GensQGLWidget(QWidget *parent = 0);
		~GensQGLWidget();
		
		// TODO: Expand this function?
		void vbUpdate(void) { updateGL(); }
		
		// Return a QWidget* version of this object.
		QWidget *toQWidget(void) { return this; }
		
		/**
		 * osd_show_preview(): Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osd_show_preview(int duration, const QImage& img);
	
	protected:
		void reallocTexture(void);
		void reallocTexOsd(void);
		
		void initializeGL(void);
		
		void resizeGL(int width, int height);
		void paintGL(void);
		
		// OpenGL Texture ID.
		GLuint m_tex;
		
		// Texture format.
		int m_colorComponents;
		GLenum m_texFormat;
		GLenum m_texType;
		
		// OSD texture.
		GLuint m_texOsd;	// Texture containing U+0000 - U+00FF.
		GLuint m_glListOsd;	// Display list.
		void printOsdText(void);
		void printOsdLine(int x, int y, const QString &msg);
		
		// OpenGL Shader Manager.
		GLShaderManager m_shaderMgr;
		
		// Preview image.
		bool m_preview_show;
		QImage m_preview_img;
		GLuint m_texPreview;
		GLdouble m_preview_img_w;
		GLdouble m_preview_img_h;
		void showOsdPreview(void);
		
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
		
		// Keyboard handler functions.
		void keyPressEvent(QKeyEvent *event)
		{
			KeyHandlerQt::KeyPressEvent(event);
		}
		void keyReleaseEvent(QKeyEvent *event)
		{
			KeyHandlerQt::KeyReleaseEvent(event);
		}
		
		// Mouse handler functions.
		void mouseMoveEvent(QMouseEvent *event)
		{
			KeyHandlerQt::MouseMoveEvent(event);
		}
		void mousePressEvent(QMouseEvent *event)
		{
			KeyHandlerQt::MousePressEvent(event);
		}
		void mouseReleaseEvent(QMouseEvent *event)
		{
			KeyHandlerQt::MouseReleaseEvent(event);
		}
};

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
