/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.hpp: Gens Window.                                            *
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

#ifndef __GENS_QT4_GENSWINDOW_HPP__
#define __GENS_QT4_GENSWINDOW_HPP__

// Qt4 includes.
#include <QtCore/qglobal.h>

#ifndef Q_WS_MAC
#define GQT4_USE_QMAINWINDOW
#endif

#ifdef GQT4_USE_QMAINWINDOW
#include <QMainWindow>
#endif

#include <QCloseEvent>

#include <QtGui/QVBoxLayout>

#include "GensQGLWidget.hpp"
#include "GensMenuBar.hpp"

namespace GensQt4
{

#ifdef GQT4_USE_QMAINWINDOW
#define GENSWINDOW_BASECLASS QMainWindow
#else
#define GENSWINDOW_BASECLASS QWidget
#endif

class GensWindow : public GENSWINDOW_BASECLASS
{
	Q_OBJECT
	
	public:
		GensWindow();
		
		// Widgets.
		GensQGLWidget *m_glWidget;	// QGLWidget.
		GensMenuBar *m_menubar;		// Gens menu bar.
		
#ifdef GQT4_USE_QMAINWINDOW
		// QMainWindow-specific functions.
		
		// Resize the window.
		void gensResize(void);
#else
		// Fake GensWindow functions for compatibility.
		inline void show(void) { }
		inline void gensResize(void) { }
#endif
	
	protected:
		void setupUi(void);
		void retranslateUi(void);
		
		void closeEvent(QCloseEvent *event);
		
#ifdef GQT4_USE_QMAINWINDOW
		QWidget *centralwidget;
		QVBoxLayout *layout;
#endif
		
		// Temporary scaling variable.
		int m_scale;
	
	protected slots:
		// Window resize.
		void resizeEvent(QSize size);
		
		// Menu item selection.
		void menuTriggered(int id);
};

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
