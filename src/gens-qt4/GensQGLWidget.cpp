/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQGLWidget.cpp: QGLWidget subclass.                                  *
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

#include "GensQGLWidget.hpp"

// C includes.
#include <stdio.h>
#include <libgens/MD/VdpRend.hpp>

// Win32 requires GL/glext.h for OpenGL 1.2/1.3.
// TODO: Check the GL implementation to see what functionality is available at runtime.
#ifdef _WIN32
#include <GL/glext.h>
#endif

namespace GensQt4
{

GensQGLWidget::GensQGLWidget(QWidget *parent)
	: QGLWidget(parent)
{
	m_tex = 0;
	m_dirty = true;
}

GensQGLWidget::~GensQGLWidget()
{
	if (m_tex > 0)
	{
		glDeleteTextures(1, &m_tex);
		m_tex = 0;
	}
}

/**
 * initializeGL(): Called when GL is initialized.
 */
void GensQGLWidget::initializeGL(void)
{
	// OpenGL initialization.
	glDisable(GL_DEPTH_TEST);
	
	// Initialize the GL viewport and projection.
	resizeGL(320, 240);
	
	// Create and initialize a GL texture.
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// GL filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// Allocate the texture.
	// TODO: Use the correct GL format and type.
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
		     512, 256,	// 512x256 (320x240 rounded up to nearest powers of two)
		     0,		// No border.
		     GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	
	glDisable(GL_TEXTURE_2D);
}

void GensQGLWidget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	
	// Set the OpenGL projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Aspect ratio constraint.
	bool aspect_constraint = true;	// TODO: Make this configurable.
	if (!aspect_constraint)
	{
		// No aspect ratio constraint.
		glOrtho(-1, 1, -1, 1, -1, 1);
	}
	else
	{
		// Aspect ratio constraint.
		if ((width * 3) > (height * 4))
		{
			// Image is wider than 4:3.
			glOrtho(-((double)(width * 3) / (double)(height * 4)),
				 ((double)(width * 3) / (double)(height * 4)),
				-1, 1, -1, 1);
		}
		else if ((width * 3) < (height * 4))
		{
			// Image is taller than 4:3.
			glOrtho(-1, 1,
				-((double)(height * 4) / (double)(width * 3)),
				 ((double)(height * 4) / (double)(width * 3)),
				-1, 1);
		}
		else
		{
			// Image has the correct aspect ratio.
			glOrtho(-1, 1, -1, 1, -1, 1);
		}
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void GensQGLWidget::paintGL(void)
{
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	
	if (m_dirty)
	{
		// MD_Screen is dirty.
		// Reupload the texture.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 336);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 8);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
		
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, 0,		// x/y offset
				320, 240,	// width/height
				GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
				LibGens::VdpRend::MD_Screen.u16);
		
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
		
		// Texture is no longer dirty.
		m_dirty = false;
	}
	
	// Draw the texture.
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex2i(-1, 1);
	glTexCoord2d((320.0/512.0), 0.0);
	glVertex2i(1, 1);
	glTexCoord2d((320.0/512.0), (240.0/256.0));
	glVertex2i(1, -1);
	glTexCoord2d(0.0, (240.0/256.0));
	glVertex2i(-1, -1);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
}

}
