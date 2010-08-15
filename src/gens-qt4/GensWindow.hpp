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
#include <QtGui/QMainWindow>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCloseEvent>

#include "VBackend/VBackend.hpp"
#include "GensMenuBar.hpp"

// LibGens includes.
#include "libgens/Rom.hpp"

namespace GensQt4
{

class GensWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		GensWindow();
		~GensWindow();
		
		// Widgets.
		VBackend *m_vBackend;	// QGLWidget.
		GensMenuBar *m_menubar;		// Gens menu bar.
		
	protected:
		void setupUi(void);
		void retranslateUi(void);
		
		void closeEvent(QCloseEvent *event);
		
		QWidget *centralwidget;
		QVBoxLayout *layout;
		
		// QMainWindow virtual functions.
		void showEvent(QShowEvent *event);
		
		// GensWindow functions.
		void gensResize(void);	// Resize the window.
		
		int m_scale;		// Temporary scaling variable.
		bool m_hasInitResize;	// Has the initial resize occurred?
		
		// Controller change.
		// NOTE: DEBUG CODE: Remove this later.
		int m_ctrlChange;
	
		// Loaded ROM.
		// TODO: Move this somewhere else.
		LibGens::Rom *m_rom;
		void openRom(void);
	
	protected slots:
		// Menu item selection.
		void menuTriggered(int id);
		
		// Frame done from EmuThread.
		void emuFrameDone(void);
};

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
