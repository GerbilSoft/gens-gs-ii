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

#include <QMainWindow>
#include <QCloseEvent>

#include "SdlWidget.hpp"
#include "ui_GensWindow.h"

namespace GensQt4
{

class GensWindow : public QMainWindow, public Ui::GensWindow
{
	Q_OBJECT
	
	public:
		GensWindow();
		
		// SDL widget.
		SdlWidget *sdl;
		
		// Resize the window.
		void gensResize(void);
	
	protected:
		void closeEvent(QCloseEvent *event);
	
	protected slots:
		// Widget signals.
		void on_mnuFileQuit_triggered(void);
		void on_mnuHelpAbout_triggered(void);
		
		// Resolution tests.
		void on_mnuResTest1x_triggered(void);
		void on_mnuResTest2x_triggered(void);
		void on_mnuResTest3x_triggered(void);
		void on_mnuResTest4x_triggered(void);
};

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
