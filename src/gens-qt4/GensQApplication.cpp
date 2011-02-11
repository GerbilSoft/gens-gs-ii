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

// Qt includes.
#include <QtGui/QIcon>

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
	QCoreApplication::setOrganizationName(QString::fromLatin1("GerbilSoft"));
	QCoreApplication::setApplicationName(QString::fromLatin1("Gens/GS II"));
	
	// Set the application icon.
	QIcon iconApp;
	iconApp.addFile(QString::fromLatin1(":/gens/gensgs_48x48.png"), QSize(48, 48));
	iconApp.addFile(QString::fromLatin1(":/gens/gensgs_32x32.png"), QSize(32, 32));
	iconApp.addFile(QString::fromLatin1(":/gens/gensgs_16x16.png"), QSize(16, 16));
	setWindowIcon(iconApp);
	
#ifdef Q_OS_WIN32
	// Set the application font.
	SetFont_Win32();
#endif /* Q_OS_WIN32 */
	
	// Connect the crash handler.
#ifdef HAVE_SIGACTION
	connect(this, SIGNAL(signalCrash(int, siginfo_t*, void*)),
		this, SLOT(slotCrash(int, siginfo_t*, void*)));
#else /* !HAVE_SIGACTION */
	connect(this, SIGNAL(signalCrash(int)),
		this, SLOT(slotCrash(int)));
#endif /* HAVE_SIGACTION */
}


/**
 * IconFromTheme(): Get an icon from the system theme.
 * @param name Icon name.
 * @param fallback Fallback icon filename from Qt resource file.
 * @return QIcon.
 */
QIcon GensQApplication::IconFromTheme(QString name, QString fallback)
{
#if QT_VERSION >= 0x040600
	if (QIcon::hasThemeIcon(name))
		return QIcon::fromTheme(name);
#endif
	
	// TODO: Get system theme icons on Win32 and Mac OS X.
	
	// Get the fallback icon.
	return QIcon(fallback);
}

}
