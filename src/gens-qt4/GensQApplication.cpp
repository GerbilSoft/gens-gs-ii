/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQApplication.cpp: QApplication subclass.                            *
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

#include "GensQApplication.hpp"

#include <QtGui/QApplication>
#include <QtCore/QThread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "gqt4_win32.hpp"
#endif

namespace GensQt4
{

/**
 * gqaInit(): GensQApplication initialization function.
 * The same code is used in all three GensQApplication() constructors.
 */
void GensQApplication::gqaInit(void)
{
	// Save the GUI thread pointer for later.
	m_guiThread = QThread::currentThread();
	
	// Set application information.
	QCoreApplication::setOrganizationName("GerbilSoft");
	QCoreApplication::setApplicationName("Gens/GS II");
	
	// Connect the crash handler.
#ifdef HAVE_SIGACTION
	connect(this, SIGNAL(signalCrash(int, siginfo_t*, void*)),
		this, SLOT(slotCrash(int, siginfo_t*, void*)));
#else /* !HAVE_SIGACTION */
	connect(this, SIGNAL(signalCrash(int)),
		this, SLOT(slotCrash(int)));
#endif /* HAVE_SIGACTION */
}


#ifdef Q_OS_WIN32
/**
 * winEventFilter(): Win32 event filter.
 * @param msg Win32 message.
 * @param result Return value for the window procedure.
 * @return True if we're handling the message; false if we should let Qt handle the message.
 */
bool GensQApplication::winEventFilter(MSG *msg, long *result)
{
	if (msg->message != WM_SETTINGCHANGE)
		return false;
	if (msg->wParam != SPI_SETNONCLIENTMETRICS)
		return false;
	
	// WM_SETTINGCHANGE / SPI_SETNONCLIENTMETRICS.
	// Update the Qt font.
	Win32_SetFont();
	return false;
}
#endif /* Q_OS_WIN32 */

}
