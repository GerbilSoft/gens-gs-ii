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
#include "libgens/Util/cpuflags.h"
#include "libgens/Util/Timing.hpp"

// C includes.
#include <string.h>

// Qt includes.
#include <QtCore/QString>
#include <QtGui/QScrollArea>

// OpenGL includes.
// Needed for GL_VENDOR, GL_RENDERER, GL_VERSION, and glGetString()
// if Gens/GS II isn't compiled with GLEW support.
// TODO: #include <QtGui/QGLWidget> instead?
#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

// Text translation macro.
#define TR(text) \
	QApplication::translate("AboutWindow", (text), NULL, QApplication::UnicodeUTF8)

namespace GensQt4
{

// Static member initialization.
AboutWindow *AboutWindow::m_AboutWindow = NULL;

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
	
	// Set the text.
	lblDebugInfo->setText(AboutWindow::GetDebugInfo());
	
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


/**
 * GetDebugInfo(): Get debug information.
 * @return Debug information.
 */
QString AboutWindow::GetDebugInfo(void)
{
	// Debug information.
	QString sDebugInfo =
		TR("Compiled using Qt") + " " + QT_VERSION_STR + ".\n" +
		TR("Using Qt") + " " + qVersion() + ".\n\n";
	
	// CPU flags.
	// TODO: Move the array of CPU flag names to LibGens.
	sDebugInfo += TR("CPU Flags") + ": ";
#if defined(__i386__) || defined(__amd64__)
	const char *CpuFlagNames[11] =
	{
		"MMX", "MMXEXT", "3DNow!", "3DNow! EXT",
		"SSE", "SSE2", "SSE3", "SSSE3",
		"SSE4.1", "SSE4.2", "SSE4a"
	};
	unsigned int cnt = 0;
	for (unsigned int i = 0; i < (sizeof(CpuFlagNames)/sizeof(CpuFlagNames[0])); i++)
	{
		if (CPU_Flags & (1 << i))
		{
			if (cnt != 0)
				sDebugInfo += ", ";
			sDebugInfo += CpuFlagNames[i];
			cnt++;
		}
	}
	sDebugInfo += "\n";
#else
	sDebugInfo += "(none)\n";
#endif /* defined(__i386__) || defined(__amd64__) */
	
	// Timing method.
	sDebugInfo += TR("Timing Method") + ": " +
		LibGens::Timing::GetTimingMethodName(LibGens::Timing::GetTimingMethod()) + "()\n\n";
	
#ifdef Q_OS_WIN32
	// Win32 code page information.
	sDebugInfo += GetCodePageInfo() + "\n";
#endif /* Q_OS_WIN32 */
	
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
	
	// TODO: Print "No GL extensions in use." if no GL extensions are in use.
	const QChar chrBullet(0x2022);	// U+2022: BULLET
	if (GLEW_ARB_fragment_program)
		sDebugInfo += chrBullet + QString(" ") + "GL_ARB_fragment_program" + "\n";
#endif /* HAVE_GLEW */

#endif /* GL_SHADING_LANGUAGE_VERSION */

#endif /* HAVE_OPENGL */
	
	// Trim whitespace at the end of sDebugInfo.
	return sDebugInfo.trimmed();
}


#ifdef Q_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

/**
 * GetCodePageInfo(): Get information about the system code pages.
 * @return System code page information.
 */
QString AboutWindow::GetCodePageInfo(void)
{
	QString sCodePageInfo;
	
	// Get the ANSI and OEM code pages.
	struct cpInfo
	{
		unsigned int cp;
		const char *cpStr;
	};
	
	cpInfo m_cpInfo[2] =
	{
		{CP_ACP,	"System ANSI code page"},
		{CP_OEMCP,	"System OEM code page"}
	};
	
	// TODO: GetCPInfoExU() support?
	for (int i = 0; i < 2; i++)
	{
		sCodePageInfo += m_cpInfo[i].cpStr;
		sCodePageInfo += ": ";
		
		// Get the code page information.
		CPINFOEX cpix;
		BOOL bRet = GetCPInfoExA(m_cpInfo[i].cp, 0, &cpix);
		if (!bRet)
		{
			sCodePageInfo += "Unknown [GetCPInfoExA() failed]\n";
			continue;
		}
		
		sCodePageInfo += QString::number(cpix.CodePage);
		
		// if the code page name is blank, don't add extra parentheses.
		if (cpix.CodePageName[0] == 0x00)
		{
			sCodePageInfo += "\n";
			continue;
		}
		
		// Add the code page name.
		sCodePageInfo += " (";
		
		// Windows XP has the code page number in cpix.CodePageName,
		// followed by two spaces, and then the code page name in parentheses.
		char *parenStart = strchr(cpix.CodePageName, '(');
		if (!parenStart)
		{
			// No parentheses. Use the code page name as-is.
			sCodePageInfo += QString::fromLocal8Bit(cpix.CodePageName);
		}
		else
		{
			// Found starting parenthesis. Check for ending parenthesis.
			char *parenEnd = strrchr(parenStart, ')');
			if (parenEnd)
			{
				// Found ending parenthesis. Null it out.
				*parenEnd = 0x00;
			}
			
			sCodePageInfo += QString::fromLocal8Bit(parenStart + 1);
		}
		
		sCodePageInfo += ")\n";
	}
	
	// Is Gens/GS II using Unicode?
	if (GetModuleHandleW(NULL) != NULL)
		sCodePageInfo += "Using Unicode strings for Win32 API.\n";
	else
		sCodePageInfo += "Using ANSI strings for Win32 API.\n";
	
	return sCodePageInfo;
}
#endif /* Q_OS_WIN32 */

}
