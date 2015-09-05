/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQGLWidget.hpp: QGLWidget subclass.                                  *
 * (PRIVATE HEADER)                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __GENS_QT4_VBACKEND_GENSQGLWIDGET_P_HPP__
#define __GENS_QT4_VBACKEND_GENSQGLWIDGET_P_HPP__

#include "GensQGLWidget.hpp"
#include "../Input/KeyHandlerQt.hpp"

// Qt includes.
#include <QtGui/QWidget>

namespace GensQt4
{

class GensQGLWidgetPrivate : public QGLWidget
{
	Q_OBJECT

	public:
		GensQGLWidgetPrivate(GensQGLWidget *q);

	private:
		typedef QGLWidget super;
	private:
		friend class GensQGLWidget;
		GensQGLWidget *const q;
		Q_DISABLE_COPY(GensQGLWidgetPrivate)

	protected:
		// Keyboard handler functions.
		void keyPressEvent(QKeyEvent *event)
			{ q->m_keyHandler->keyPressEvent(event); }
		void keyReleaseEvent(QKeyEvent *event)
			{ q->m_keyHandler->keyReleaseEvent(event); }

		// Mouse handler functions.
		void mouseMoveEvent(QMouseEvent *event)
			{ q->m_keyHandler->mouseMoveEvent(event); }
		void mousePressEvent(QMouseEvent *event)
			{ q->m_keyHandler->mousePressEvent(event); }
		void mouseReleaseEvent(QMouseEvent *event)
			{ q->m_keyHandler->mouseReleaseEvent(event); }

		// QGLWidget protected functions.
		void initializeGL(void);
		void resizeGL(int width, int height);
		void paintGL(void);
};

inline GensQGLWidgetPrivate::GensQGLWidgetPrivate(GensQGLWidget *q)
	: super(QGLFormat(QGL::NoAlphaChannel | QGL::NoDepthBuffer), q)
	, q(q)
{
	// Accept keyboard focus.
	setFocusPolicy(Qt::StrongFocus);
	
}

inline void GensQGLWidgetPrivate::initializeGL(void)
	{ q->glb_initializeGL(); }

inline void GensQGLWidgetPrivate::resizeGL(int width, int height)
	{ q->glb_resizeGL(width, height); }

inline void GensQGLWidgetPrivate::paintGL(void)
	{ q->glb_paintGL(); }

}

#endif /* __GENS_QT4_VBACKEND_GENSQGLWIDGET_P_HPP__ */
