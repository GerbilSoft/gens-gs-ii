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

#ifndef __GENS_QT4_GENSQGLWIDGET_HPP__
#define __GENS_QT4_GENSQGLWIDGET_HPP__

#include <config.h>

#include "VBackend.hpp"

// OpenGL Shader Manager.
// This file MUST be before any other GL includes,
// since it includes the GLEW headers.
#include "GLShaderManager.hpp"

// Qt includes.
#include <QtOpenGL/QGLWidget>
#include <QtCore/QStringList>

// OpenGL GL_TEXTURE_2D wrapper.
#include "GlTex2D.hpp"

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
		
		/**
		 * sizeHint(): Qt size hint.
		 * TODO: Return something other than 320x240 depending on renderer?
		 * @return Preferred widget size.
		 */
		QSize sizeHint(void) const { return QSize(320, 240); }
		
#ifdef HAVE_GLEW
		/**
		 * GLExtsInUse(): Get a list of the OpenGL extensions in use.
		 * @return List of OpenGL extensions in use.
		 */
		static QStringList GLExtsInUse(void);
#endif /* HAVE_GLEW */
	
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
		GlTex2D *m_texPreview;
		void showOsdPreview(void);
		
		// Keyboard handler functions.
		void keyPressEvent(QKeyEvent *event)
			{ KeyHandlerQt::KeyPressEvent(event); }
		void keyReleaseEvent(QKeyEvent *event)
			{ KeyHandlerQt::KeyReleaseEvent(event); }
		
		// Mouse handler functions.
		void mouseMoveEvent(QMouseEvent *event)
			{ KeyHandlerQt::MouseMoveEvent(event); }
		void mousePressEvent(QMouseEvent *event)
			{ KeyHandlerQt::MousePressEvent(event); }
		void mouseReleaseEvent(QMouseEvent *event)
			{ KeyHandlerQt::MouseReleaseEvent(event); }
	
	protected slots:
		void osdFpsEnabled_changed_slot(bool enable)
			{ setOsdFpsEnabled(enable); }
		void osdFpsColor_changed_slot(const QColor& color)
			{ setOsdFpsColor(color); }
		void osdMsgEnabled_changed_slot(bool enable)
			{ setOsdMsgEnabled(enable); }
		void osdMsgColor_changed_slot(const QColor& color)
			{ setOsdMsgColor(color); }
		
		void fastBlur_changed_slot(bool newFastBlur)
			{ setFastBlur(newFastBlur); }
		void aspectRatioConstraint_changed_slot(bool newAspectRatioConstraint)
			{ setAspectRatioConstraint(newAspectRatioConstraint); }
		void bilinearFilter_changed_slot(bool newBilinearFilter);
		void pauseTint_changed_slot(bool newPauseTint)
			{ setPauseTint(newPauseTint); }
};

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
