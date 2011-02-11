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
#include <QtGui/QStyle>


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
	QCoreApplication::setOrganizationName(QLatin1String("GerbilSoft"));
	QCoreApplication::setApplicationName(QLatin1String("Gens/GS II"));
	
	// Set the application icon.
	QIcon iconApp;
	iconApp.addFile(QLatin1String(":/gens/gensgs_48x48.png"), QSize(48, 48));
	iconApp.addFile(QLatin1String(":/gens/gensgs_32x32.png"), QSize(32, 32));
	iconApp.addFile(QLatin1String(":/gens/gensgs_16x16.png"), QSize(16, 16));
	setWindowIcon(iconApp);
	
#if QT_VERSION >= 0x040600
	// Check if an icon theme is available.
	if (!QIcon::hasThemeIcon(QLatin1String("application-exit")))
	{
		// Icon theme is not available.
		// Use built-in Oxygen icon theme.
		// Reference: http://tkrotoff.blogspot.com/2010/02/qiconfromtheme-under-windows.html
		QIcon::setThemeName(QLatin1String("oxygen"));
	}
#endif
	
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
 * @return QIcon.
 */
QIcon GensQApplication::IconFromTheme(QString name)
{
#ifndef Q_WS_X11
	// Check if a system icon exists.
	// TODO: Add standardPixmap parameter to reduce string comparisons?
	// TODO: Native Win32 icons for everything else.
	QStyle *style = GensQApplication::style();
	if (name == QLatin1String("document-open"))
		return style->standardPixmap(QStyle::SP_DirOpenIcon);
#endif
	
#if QT_VERSION >= 0x040600
	if (QIcon::hasThemeIcon(name))
		return QIcon::fromTheme(name);
#endif
	
	// System icon doesn't exist.
	// Get the fallback icon.
	QIcon icon;
	icon.addFile(QLatin1String(":/oxygen/64x64/") + name + QLatin1String(".png"), QSize(64, 64));
	icon.addFile(QLatin1String(":/oxygen/48x48/") + name + QLatin1String(".png"), QSize(48, 48));
	icon.addFile(QLatin1String(":/oxygen/32x32/") + name + QLatin1String(".png"), QSize(32, 32));
	icon.addFile(QLatin1String(":/oxygen/22x22/") + name + QLatin1String(".png"), QSize(22, 22));
	icon.addFile(QLatin1String(":/oxygen/16x16/") + name + QLatin1String(".png"), QSize(16, 16));
	return icon;
}

}
