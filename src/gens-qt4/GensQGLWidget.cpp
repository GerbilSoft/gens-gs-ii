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

namespace GensQt4
{

GensQGLWidget::GensQGLWidget(QWidget *parent)
	: QGLWidget(parent)
{
	// TODO: Initialize GensQGLWidget.
}

/**
 * initializeGL(): Called when GL is initialized.
 */
void GensQGLWidget::initializeGL(void)
{
	// OpenGL initialization.
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, 320, 240);	// MD resolution.
	
	// Set the OpenGL projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 320, 240, 0, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
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
	printf("Resize to: %dx%d\n", width, height);
}

void GensQGLWidget::paintGL(void)
{
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	
	// TODO: Copy MD screen to the texture.
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
	
	// Draw the texture.
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex2i(0, 0);
	glTexCoord2d((320.0/512.0), 0.0);
	glVertex2i(320, 0);
	glTexCoord2d((320.0/512.0), (240.0/256.0));
	glVertex2i(320, 240);
	glTexCoord2d(0.0, (240.0/256.0));
	glVertex2i(0, 240);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
}

}
