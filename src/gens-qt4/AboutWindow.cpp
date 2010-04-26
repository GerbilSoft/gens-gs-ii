/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * AboutWindow.cpp: About Window.                                          *
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

#include "AboutWindow.hpp"

namespace GensQt4
{

// Static member initialization.
AboutWindow *AboutWindow::m_AboutWindow = NULL;


/**
 * AboutWindow(): Initialize the About window.
 */
AboutWindow::AboutWindow(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
	// TODO: Git version.
}


/**
 * ~AboutWindow(): Shut down the About window.
 */
AboutWindow::~AboutWindow()
{
	m_AboutWindow = NULL;
	printf("DEL\n");
}


/**
 * ShowSingle(): Show a single instance of the About window.
 * @param parent Parent window.
 */
void AboutWindow::ShowSingle(QWidget *parent)
{
	if (m_AboutWindow != NULL)
	{
		// About Window is already displayed.
		// TODO
	}
	else
	{
		// About Window is not displayed.
		m_AboutWindow = new AboutWindow(parent);
		m_AboutWindow->show();
	}
}

}
