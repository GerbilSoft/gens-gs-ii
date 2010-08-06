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
#include "libgens/macros/git.h"

// Qt includes.
#include <QtCore/QString>
#include <QtGui/QScrollArea>

// Text translation macro.
#define TR(text) \
	QApplication::translate("AboutWindow", (text), NULL, QApplication::UnicodeUTF8)

namespace GensQt4
{

// Static member initialization.
AboutWindow *AboutWindow::m_AboutWindow = NULL;

#include <stdio.h>
/**
 * AboutWindow(): Initialize the About window.
 */
AboutWindow::AboutWindow(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	setupUi(this);
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
	// Build the program title text.
	QString sPrgTitle =
		"<b>" + TR("Gens/GS II") + "</b><br />\n" +
		TR("Development Build") + "<br />\n";
	
#ifdef GENS_GIT_VERSION
	// Append the git version to the title text.
	sPrgTitle += QString(GENS_GIT_VERSION) + "<br />\n";
#endif
	
	sPrgTitle += "<br />\n" +
		TR("Sega Genesis / Mega Drive,") + "<br />\n" +
		TR("Sega CD / Mega CD,") + "<br />\n" +
		TR("Sega 32X emulator");
	
	// Set the text.
        lblPrgTitle->setText(sPrgTitle);
	
	// Debug information.
	QString sDebugInfo =
		TR("Compiled using Qt") + " " + QT_VERSION_STR + "\n" +
		TR("Using Qt") + " " + qVersion() + "\n\n";
	
#ifndef HAVE_OPENGL
	sDebugInfo += TR("OpenGL disabled.\n");
#else
	const char *glVendor = (const char*)glGetString(GL_VENDOR);
	const char *glRenderer = (const char*)glGetString(GL_RENDERER);
	const char *glVersion = (const char*)glGetString(GL_VERSION);
	sDebugInfo += TR("OpenGL vendor string:") + " " +
			QString(glVendor ? glVendor : TR("(unknown)")) + "\n" +
			TR("OpenGL renderer string:") + " " +
			QString(glRenderer ? glRenderer : TR("(unknown)")) + "\n" +
			TR("OpenGL version string:") + " " +
			QString(glVersion ? glVersion : TR("(unknown)")) + "\n";
	
#ifdef GL_SHADING_LANGUAGE_VERSION
	if (glVersion && glVersion[0] >= '2' && glVersion[1] == '.')
	{
		const char *glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		sDebugInfo += TR("GLSL version string:") + " " +
				QString(glslVersion ? glslVersion : "(unknown)") + "\n";
	}
	
	// OpenGL extensions.
	sDebugInfo += "\n";
#ifndef HAVE_GLEW
	sDebugInfo += "GLEW disabled; no GL extensions supported.\n";
#else
	const char *glewVersion = (const char*)glewGetString(GLEW_VERSION);
	sDebugInfo += "GLEW version " + QString(glewVersion ? glewVersion : TR("(unknown)")) + "\n" +
			TR("GL extensions in use:") + "\n";
	const QString sBullet = QString::fromUtf8("• ");
	if (GLEW_ARB_fragment_program)
		sDebugInfo += sBullet + "GL_ARB_fragment_program\n";
#endif /* HAVE_GLEW */

#endif /* GL_SHADING_LANGUAGE_VERSION */

#endif /* HAVE_OPENGL */
	
	// Remove any newlines from the end of sDebugInfo.
	while (sDebugInfo.endsWith('\n') || sDebugInfo.endsWith('\r'))
		sDebugInfo.remove(sDebugInfo.size() - 1, 1);
	
	// Set the text.
	lblDebugInfo->setText(sDebugInfo);
	
	// Create the scroll areas.
	// Qt Designer's QScrollArea implementation is horribly broken.
	// Also, this has to be done after the labels are set, because
	// QScrollArea is kinda dumb.
	QScrollArea *scrlCopyrights = new QScrollArea();
	scrlCopyrights->setFrameShape(QFrame::NoFrame);
	scrlCopyrights->setFrameShadow(QFrame::Plain);
	scrlCopyrights->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrlCopyrights->setWidget(lblCopyrights);
	lblCopyrights->setAutoFillBackground(false);
	vboxCopyrights->addWidget(scrlCopyrights);
	
	QScrollArea *scrlDebugInfo = new QScrollArea();
	scrlDebugInfo->setFrameShape(QFrame::NoFrame);
	scrlDebugInfo->setFrameShadow(QFrame::Plain);
	scrlCopyrights->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrlDebugInfo->setWidget(lblDebugInfo);
	lblDebugInfo->setAutoFillBackground(false);
	vboxDebugInfo->addWidget(scrlDebugInfo);
}


/**
 * ~AboutWindow(): Shut down the About window.
 */
AboutWindow::~AboutWindow()
{
	// Clear the m_AboutWindow pointer.
	m_AboutWindow = NULL;
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
		// NOTE: This doesn't seem to work on KDE 4.4.2...
		QApplication::setActiveWindow(m_AboutWindow);
	}
	else
	{
		// About Window is not displayed.
		m_AboutWindow = new AboutWindow(parent);
		m_AboutWindow->show();
	}
}

}
