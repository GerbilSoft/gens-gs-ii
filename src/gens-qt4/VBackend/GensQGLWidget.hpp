/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQGLWidget.hpp: QGLWidget subclass.                                  *
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

#ifndef __GENS_QT4_VBACKEND_GENSQGLWIDGET_HPP__
#define __GENS_QT4_VBACKEND_GENSQGLWIDGET_HPP__

#include "GLBackend.hpp"

// Qt includes.
#include <QtOpenGL/QGLWidget>
#include <QtCore/QStringList>
#include <QtCore/QRectF>

// Qt classes.
class QVBoxLayout;

// Key Handler.
#include "Input/KeyHandlerQt.hpp"

namespace GensQt4
{

class GensQGLWidgetPrivate;

class GensQGLWidget : public GLBackend
{
	Q_OBJECT
	
	public:
		GensQGLWidget(QWidget *parent = 0, KeyHandlerQt *keyHandler = 0);
		virtual ~GensQGLWidget();
		
		void vbUpdate(void);
		
		/**
		 * sizeHint(): Qt size hint.
		 * TODO: Return something other than 320x240 depending on renderer?
		 * @return Preferred widget size.
		 */
		QSize sizeHint(void) const { return QSize(320, 240); }
	
	protected slots:
		void setFastBlur(bool newFastBlur);
		void setBilinearFilter(bool newBilinearFilter);
		void setPauseTint(bool newPauseTint);
	
	private:
		friend class GensQGLWidgetPrivate;
		GensQGLWidgetPrivate *d;
		Q_DISABLE_COPY(GensQGLWidget);
		
		QVBoxLayout *m_layout;
};

}

#endif /* __GENS_QT4_VBACKEND_GENSQGLWIDGET_HPP__ */
