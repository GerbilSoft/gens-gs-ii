/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.cpp: Gens Window.                                            *
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

#include "GensWindow.hpp"
#include "gqt4_main.hpp"

#include "AboutWindow.hpp"

#include <QtGui/QIcon>


namespace GensQt4
{

/**
 * GensWindow(): Initialize the Gens window.
 */
GensWindow::GensWindow()
{
	setupUi(this);
	
	// Set the window icon.
	QIcon winIcon;
	winIcon.addFile(":/gens/gensgs_48x48", QSize(48, 48));
	winIcon.addFile(":/gens/gensgs_32x32", QSize(32, 32));
	winIcon.addFile(":/gens/gensgs_16x16", QSize(16, 16));
	this->setWindowIcon(winIcon);
	
#if QT_VERSION >= 0x040600
	// Set menu icons.
	// QIcon::fromTheme() requires Qt 4.6 or later.
	// TODO: Include fallback icons for Win32 and Mac OS X.
	mnuFileQuit->setIcon(QIcon::fromTheme("application-exit"));
	mnuHelpAbout->setIcon(QIcon::fromTheme("help-about"));
#endif
	
	// Create the SDL widget.
	sdl = new SdlWidget(this->centralwidget);
}


/**
 * closeEvent(): Window is being closed.
 * @param event Close event.
 */
void GensWindow::closeEvent(QCloseEvent *event)
{
	// Quit.
	QuitGens();
	
	// Accept the close event.
	event->accept();
}


/** Slots. **/


/**
 * on_mnuFileQuit_triggered(): File, Quit.
 */
void GensWindow::on_mnuFileQuit_triggered(void)
{
	// Quit.
	QuitGens();
	
	// Close the window.
	this->close();
}


/**
 * on_mnuHelpAbout_triggered(): Help, About.
 */
void GensWindow::on_mnuHelpAbout_triggered(void)
{
	// About Gens/GS II.
	AboutWindow::ShowSingle(this);
}

}
