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
 * fastBlur_changed_slot(): Fast Blur effect has changed.
 * @param newFastBlur (bool) New Fast Blur effect setting.
 */
void GensQGLWidget::fastBlur_changed_slot(const QVariant& newFastBlur)
{
	if (isRunning() && isPaused())
	{
		// Emulation is running, but is currently paused.
		// (TODO: Check shader status, and only update if
		// we're not using the Fast Blur shader.)
		// Update the MD screen.
		m_mdScreenDirty = true;
	}
	
	// Call VBackend's fastBlur_changed_slot().
	VBackend::fastBlur_changed_slot(newFastBlur);
}


/**
 * bilinearFilter_changed_slot(): Bilinear filter setting has changed.
 * @param newBilinearFilter (bool) New bilinear filter setting.
 */
void GensQGLWidget::bilinearFilter_changed_slot(const QVariant& newBilinearFilter)
{
	// Update GLBackend's bilinear filter setting.
	d->makeCurrent();
	GLBackend::bilinearFilter_changed_slot(newBilinearFilter);
}


/**
 * pauseTint_changed_slot(): Pause Tint effect setting has changed.
 * @param newPauseTint (bool) New pause tint effect setting.
 */
void GensQGLWidget::pauseTint_changed_slot(const QVariant& newPauseTint)
{
	// Update GLBackend's pause tint setting.
	d->makeCurrent();
	GLBackend::pauseTint_changed_slot(newPauseTint);
}

}
