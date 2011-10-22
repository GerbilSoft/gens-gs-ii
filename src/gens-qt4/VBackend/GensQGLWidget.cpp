/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQGLWidget.cpp: QGLWidget subclass.                                  *
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

#include <config.h>

#include "GensQGLWidget.hpp"
#include "GensQGLWidget_p.hpp"

// KeyHandlerQt.
#include "Input/KeyHandlerQt.hpp"

// gqt4_main.hpp has GensConfig.
#include "gqt4_main.hpp"

// Qt includes.
#include <QtGui/QVBoxLayout>

namespace GensQt4
{

GensQGLWidget::GensQGLWidget(QWidget *parent, KeyHandlerQt *keyHandler)
	: GLBackend(parent, keyHandler)
	, d(new GensQGLWidgetPrivate(this))
{
	// NOTE: QVBoxLayout is required for proper resizing.
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->addWidget(d);
	
	// Set the GensQGLWidgetPrivate object as the focus proxy.
	setFocusProxy(d);
	
	// Initialize mouse tracking.
	// TODO: Only do this if a Mega Mouse is connected.
	// TODO: IoMegaMouse doesn't work right.
#if 0
	setMouseTracking(true);
#endif
}

GensQGLWidget::~GensQGLWidget()
	{ delete d; }



/**
 * vbUpdate(): Video Backend update function.
 */
void GensQGLWidget::vbUpdate(void)
{
	// TODO: Expand this function?
	d->makeCurrent();
	d->updateGL();
}


/**
 * setFastBlur(): Fast Blur setting has changed.
 * @param newFastBlur New fast blur setting.
 */
void GensQGLWidget::setFastBlur(bool newFastBlur)
{
	if (fastBlur() == newFastBlur)
		return;
	
	if (isRunning() && isPaused())
	{
		// Emulation is running, but is currently paused.
		// (TODO: Check shader status, and only update if
		// we're not using the Fast Blur shader.)
		// Update the MD screen.
		m_mdScreenDirty = true;
	}
	
	VBackend::setFastBlur(newFastBlur);
}


/**
 * setBilinearFilter(): Bilinear filter setting has changed.
 * @param newBilinearFilter New bilinear filter setting.
 */
void GensQGLWidget::setBilinearFilter(bool newBilinearFilter)
{
	if (bilinearFilter() == newBilinearFilter)
		return;
	
	// Update GLBackend's bilinear filter setting.
	d->makeCurrent();
	GLBackend::setBilinearFilter(newBilinearFilter);
}


/**
 * setPauseTint(): Pause Tint setting has changed.
 * @param newPauseTint New pause tint setting.
 */
void GensQGLWidget::setPauseTint(bool newPauseTint)
{
	if (pauseTint() == newPauseTint)
		return;
	
	// Update GLBackend's pause tint setting.
	d->makeCurrent();
	GLBackend::setPauseTint(newPauseTint);
}

}
