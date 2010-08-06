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

#ifdef HAVE_GLEW
// GL Extension Wrangler.
#include <GL/glew.h>
#endif

#include <QtCore/QObject>
#include <QtOpenGL/QGLWidget>

#include "libgens/MD/VdpRend.hpp"

namespace GensQt4
{

class GensQGLWidget : public QGLWidget
{
	Q_OBJECT
	
	public:
		GensQGLWidget(QWidget *parent = 0);
		~GensQGLWidget();
		
		void setDirty(void) { m_dirty = true; }
		void reallocTexture(void);
		
	protected:
		void initializeGL(void);
		
		void resizeGL(int width, int height);
		void paintGL(void);
		
		// OpenGL Texture ID.
		GLuint m_tex;
		
		// Dirty flag. If set, texture must be reuploaded.
		bool m_dirty;
		
		// Texture format.
		LibGens::VdpRend::ColorDepth m_lastBpp;
		int m_colorComponents;
		GLenum m_texFormat;
		GLenum m_texType;
		
#ifdef HAVE_GLEW
		// ARB fragment programs.
		GLuint m_fragPause;
		
		// ARB fragment programs: assembly code.
		static const char *ms_fragPause_asm;
#endif
};

}

#endif /* __GENS_QT4_GENSQGLWIDGET_HPP__ */
