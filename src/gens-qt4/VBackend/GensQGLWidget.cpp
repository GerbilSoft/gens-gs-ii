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

#include <config.h>

#include "GensQGLWidget.hpp"
#include "gqt4_main.hpp"

// KeyHandlerQt.
#include "Input/KeyHandlerQt.hpp"

// gqt4_main.hpp has GensConfig.
#include "gqt4_main.hpp"

namespace GensQt4
{

GensQGLWidget::GensQGLWidget(QWidget *parent)
	: QGLWidget(QGLFormat(QGL::NoAlphaChannel | QGL::NoDepthBuffer), parent)
{
	// Accept keyboard focus.
	setFocusPolicy(Qt::StrongFocus);
	
	// Initialize mouse tracking.
	// TODO: Only do this if a Mega Mouse is connected.
	// TODO: IoMegaMouse doesn't work right.
#if 0
	setMouseTracking(true);
#endif
	
	// Connect signals from GensConfig.
	// TODO: Reconnect signals if GensConfig is deleted/recreated.
	// TODO: OSD colors.
	connect(gqt4_config, SIGNAL(osdFpsEnabled_changed(bool)),
		this, SLOT(osdFpsEnabled_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(osdFpsColor_changed(const QColor&)),
		this, SLOT(osdFpsColor_changed_slot(const QColor&)));
	connect(gqt4_config, SIGNAL(osdMsgEnabled_changed(bool)),
		this, SLOT(osdMsgEnabled_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(osdMsgColor_changed(const QColor&)),
		this, SLOT(osdMsgColor_changed_slot(const QColor&)));
	
	// Video effect settings.
	connect(gqt4_config, SIGNAL(fastBlur_changed(bool)),
		this, SLOT(fastBlur_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(aspectRatioConstraint_changed(bool)),
		this, SLOT(aspectRatioConstraint_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(bilinearFilter_changed(bool)),
		this, SLOT(bilinearFilter_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(pauseTint_changed(bool)),
		this, SLOT(pauseTint_changed_slot(bool)));
}

GensQGLWidget::~GensQGLWidget()
{
}


/**
 * fastBlur_changed_slot(): Fast Blur setting has changed.
 * @param newFastBlur New fast blur setting.
 */
void GensQGLWidget::fastBlur_changed_slot(bool newFastBlur)
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
	
	setFastBlur(newFastBlur);
}


/**
 * bilinearFilter_changed_slot(): Bilinear filter setting has changed.
 * @param newBilinearFilter New bilinear filter setting.
 */
void GensQGLWidget::bilinearFilter_changed_slot(bool newBilinearFilter)
{
	if (bilinearFilter() == newBilinearFilter)
		return;
	
	// Update GLBackend's bilinear filter setting.
	this->makeCurrent();
	GLBackend::setBilinearFilter(newBilinearFilter);
}


/**
 * pauseTint_changed_slot(): Pause Tint setting has changed.
 * @param newPauseTint New pause tint setting.
 */
void GensQGLWidget::pauseTint_changed_slot(bool newPauseTint)
{
	if (pauseTint() == newPauseTint)
		return;
	
	// Update GLBackend's pause tint setting.
	GLBackend::setPauseTint(newPauseTint);
}

}
