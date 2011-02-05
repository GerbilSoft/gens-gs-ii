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

#include <config.h>

#ifdef HAVE_GLEW
// GL Extension Wrangler.
#include <GL/glew.h>
#endif

// Included libraries.
#include <zlib.h>
#include "lzma/7z/7zVersion.h"

#include "AboutWindow.hpp"
#include "libgens/lg_main.hpp"
#include "libgens/Util/cpuflags.h"
#include "libgens/Util/Timing.hpp"
#include "libgens/credits.h"

// C includes.
#include <string.h>

// C++ includes.
#include <sstream>
using std::stringstream;

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
	
	// Scroll areas aren't initialized.
	m_scrlAreaInit = false;
	
	// Initialize the About window teXt.
	initAboutWindowText();
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
 * initAboutWindowText(): Initialize the About Window text.
 */
void AboutWindow::initAboutWindowText(void)
{
	// Line break string.
	const QString sLineBreak = QString::fromLatin1("<br/>\n");
	
	// Build the copyright string.
	QString sCopyrights = QString::fromUtf8(
			"(c) 1999-2002 by Stéphane Dallongeville.<br/>\n"
			"(c) 2003-2004 by Stéphane Akhoun.<br />\n<br />\n"
			"Gens/GS (c) 2008-2011 by David Korth.<br />\n<br />\n");
	
	sCopyrights += tr("Visit the Gens homepage:") + sLineBreak +
			QString::fromLatin1(
				"<a href=\"http://gens.consolemul.com\">"
				"http://gens.consolemul.com</a>") +
			sLineBreak + sLineBreak;
	
	sCopyrights += tr("For news on Gens/GS, visit Sonic Retro:") + sLineBreak +
			QString::fromLatin1(
				"<a href=\"http://www.sonicretro.org\">"
				"http://www.sonicretro.org</a>");
	
	// Set the copyright string.
	lblCopyrights->setText(sCopyrights);
	lblCopyrights->setTextFormat(Qt::RichText);
	
	// Build the program title text.
	QString sPrgTitle =
		QString::fromLatin1("<b>") + tr("Gens/GS II") + QString::fromLatin1("</b>") + sLineBreak +
		tr("Development Build") + sLineBreak;
	
#if !defined(GENS_ENABLE_EMULATION)
	sPrgTitle += QString::fromLatin1("<b>") +
			tr("NO-EMULATION BUILD") +
			QString::fromLatin1("</b>") + sLineBreak;
#endif
	
	if (LibGens::version_vcs != NULL)
	{
		// Append the VCS revision to the title text.
		sPrgTitle += QString::fromLatin1(LibGens::version_vcs) + sLineBreak;
	}
	
	sPrgTitle += sLineBreak +
		tr("Sega Genesis / Mega Drive,") + sLineBreak +
		tr("Sega CD / Mega CD,") + sLineBreak +
		tr("Sega 32X emulator");
	
	// Set the program title text.
        lblPrgTitle->setText(sPrgTitle);
	
	// Set the included libraries text.
	lblIncLibraries->setText(AboutWindow::GetIncLibraries());
	
	// Set the debug information text.
	lblDebugInfo->setText(AboutWindow::GetDebugInfo());
	
	// Build the credits text.
	// TODO: Use QString instead of stringstream?
	stringstream ss_credits;
	const GensGS_credits_t *p_credits = &GensGS_credits[0];
	for (; p_credits->credit_title || p_credits->credit_name; p_credits++)
	{
		if (p_credits->credit_title)
		{
			// Title specified.
			if (!strncmp(p_credits->credit_title, "-", 2))
			{
				// Title is "-". Next line.
				ss_credits << "<br/>\n";
				continue;
			}
			else
			{
				// Title is not "-". Print it.
				ss_credits << "<b>" << p_credits->credit_title << "</b><br/>\n";
			}
		}
		
		if (p_credits->credit_name)
		{
			// Name specified.
			ss_credits << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
				   << p_credits->credit_name << "<br/>\n";
		}
	}
	
	// Set the credits text.
	lblCredits->setText(QString::fromUtf8(ss_credits.str().c_str()));
	lblCredits->setTextFormat(Qt::RichText);
	
	if (!m_scrlAreaInit)
	{
		// Create the scroll areas.
		// Qt Designer's QScrollArea implementation is horribly broken.
		// Also, this has to be done after the labels are set, because
		// QScrollArea is kinda dumb.
		QScrollArea *scrlIncLibraries = new QScrollArea();
		scrlIncLibraries->setFrameShape(QFrame::NoFrame);
		scrlIncLibraries->setFrameShadow(QFrame::Plain);
		scrlIncLibraries->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlIncLibraries->setWidget(lblIncLibraries);
		scrlIncLibraries->setWidgetResizable(true);
		lblIncLibraries->setAutoFillBackground(false);
		vboxIncLibraries->addWidget(scrlIncLibraries);
		
		QScrollArea *scrlDebugInfo = new QScrollArea();
		scrlDebugInfo->setFrameShape(QFrame::NoFrame);
		scrlDebugInfo->setFrameShadow(QFrame::Plain);
		scrlDebugInfo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlDebugInfo->setWidget(lblDebugInfo);
		scrlDebugInfo->setWidgetResizable(true);
		lblDebugInfo->setAutoFillBackground(false);
		vboxDebugInfo->addWidget(scrlDebugInfo);
		
		QScrollArea *scrlCredits = new QScrollArea();
		scrlCredits->setFrameShape(QFrame::NoFrame);
		scrlCredits->setFrameShadow(QFrame::Plain);
		scrlCredits->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlCredits->setWidget(lblCredits);
		scrlCredits->setWidgetResizable(true);
		scrlCredits->setAutoFillBackground(false);
		vboxCredits->addWidget(scrlCredits);
		
		// Scroll areas initialized.
		m_scrlAreaInit = true;
	}
}


/**
 * GetIncLibraries(): Get included libraries.
 * @return Included libraries.
 */
QString AboutWindow::GetIncLibraries(void)
{
	// Common strings.
	const QString sIntCopyOf = tr("Internal copy of %1.");
	const QString sLineBreak = QString::fromLatin1("<br/>\n");
	
	// Included libraries string.
	QString sIncLibraries;
	
#if defined(HAVE_ZLIB) && !defined(ZLIB_FOUND)
	// ZLIB is included.
	sIncLibraries += sIntCopyOf.arg(QString::fromLatin1(ZLIB_VERSION)) + sLineBreak +
		QString::fromLatin1("Copyright (c) 1995-2010 Jean-loup Gailly and Mark Adler.") + sLineBreak +
		QString::fromLatin1("<a href=\"http://www.zlib.net/\">http://www.zlib.net/</a>");
#endif
	
#if defined(HAVE_ZLIB)
	// MiniZip is included.
	// TODO: Find a MiniZip version macro.
	if (!sIncLibraries.isEmpty())
		sIncLibraries += sLineBreak + sLineBreak;
	sIncLibraries += sIntCopyOf.arg(QString::fromLatin1("MiniZip 1.1")) + sLineBreak +
		QString::fromLatin1("Copyright (c) 1998-2010 by Gilles Vollant.") + sLineBreak +
		QString::fromLatin1("<a href=\"http://www.winimage.com/zLibDll/minizip.html\">"
					"http://www.winimage.com/zLibDll/minizip.html</a>") + sLineBreak +
		QString::fromLatin1("Zip64/Unzip Copyright (c) 2007-2008 by Even Rouault.") + sLineBreak +
		QString::fromLatin1("Zip64/Zip Copyright (c) 2009-2001 by Mathias Svensson.");
#endif
	
#if defined(HAVE_LZMA) && !defined(LZMA_FOUND)
	// LZMA is included.
	if (!sIncLibraries.isEmpty())
		sIncLibraries += sLineBreak + sLineBreak;
	sIncLibraries += sIntCopyOf.arg(QString::fromLatin1("the LZMA SDK " MY_VERSION)) + sLineBreak +
		QString::fromLatin1("Copyright (c) 1999-2010 by Igor Pavlov.") + sLineBreak +
		QString::fromLatin1("<a href=\"http://www.7-zip.org/sdk.html\">"
					"http://www.7-zip.org/sdk.html</a>");
#endif
	
#if defined(HAVE_GLEW) && !defined(GLEW_FOUND)
	// GLEW is included.
	if (!sIncLibraries.isEmpty())
		sIncLibraries += sLineBreak + sLineBreak;
	
	const char *glewVersion = (const char*)glewGetString(GLEW_VERSION);
	QString sGlewVersion = (glewVersion ? QString::fromLatin1(glewVersion) : QString());
	sIncLibraries += sIntCopyOf.arg(QString::fromLatin1("GLEW ") + sGlewVersion) + sLineBreak +
		QString::fromLatin1("Copyright (c) 2002-2008 by Milan Ikits.") + sLineBreak +
		QString::fromLatin1("Copyright (c) 2002-2008 by Marcelo E. Magallon.") + sLineBreak +
		QString::fromLatin1("Copyright (c) 2002 by Lev Povalahev.") + sLineBreak +
		QString::fromLatin1("Mesa 3D code Copyright (c) 1999-2007 by Brian Paul.") + sLineBreak +
		QString::fromLatin1("OpenGL code Copyright (c) 2007 The Khronos Group Inc.") + sLineBreak +
		QString::fromLatin1("<a href=\"http://glew.sourceforge.net/\'>"
					"http://glew.sourceforge.net/</a>");
#endif
	
	// Return the included libraries string.
	return sIncLibraries;
}


/**
 * GetDebugInfo(): Get debug information.
 * @return Debug information.
 */
QString AboutWindow::GetDebugInfo(void)
{
	// Debug information.
	QString sDebugInfo =
		tr("Compiled using Qt %1.").arg(QString::fromLatin1(QT_VERSION_STR)) + QChar(L'\n') +
		tr("Using Qt %1.").arg(QString::fromLatin1(qVersion())) + QChar(L'\n') + QChar(L'\n');
	
	// CPU flags.
	// TODO: Move the array of CPU flag names to LibGens.
	sDebugInfo += tr("CPU Flags") + QString::fromLatin1(": ");
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
				sDebugInfo += QString::fromLatin1(", ");
			sDebugInfo += QString::fromLatin1(CpuFlagNames[i]);
			cnt++;
		}
	}
	sDebugInfo += QChar(L'\n');
#else
	sDebugInfo += QString::fromLatin1("(none)\n");
#endif /* defined(__i386__) || defined(__amd64__) */
	
	// Timing method.
	sDebugInfo += tr("Timing Method") + QString::fromLatin1(": ") +
		QString::fromLatin1(LibGens::Timing::GetTimingMethodName(LibGens::Timing::GetTimingMethod())) +
		QString::fromLatin1("()\n\n");
	
#ifdef Q_OS_WIN32
	// Win32 code page information.
	sDebugInfo += GetCodePageInfo() + QChar(L'\n');
#endif /* Q_OS_WIN32 */
	
#ifndef HAVE_OPENGL
	sDebugInfo += tr("OpenGL disabled.\n");
#else
	const char *glVendor = (const char*)glGetString(GL_VENDOR);
	const char *glRenderer = (const char*)glGetString(GL_RENDERER);
	const char *glVersion = (const char*)glGetString(GL_VERSION);
	sDebugInfo += tr("OpenGL vendor string:") + QChar(L' ') +
			QString(glVendor ? QString::fromLatin1(glVendor) : tr("(unknown)")) + QChar(L'\n') +
			tr("OpenGL renderer string:") + QChar(L' ') +
			QString(glRenderer ? QString::fromLatin1(glRenderer) : tr("(unknown)")) + QChar(L'\n') +
			tr("OpenGL version string:") + QChar(L' ') +
			QString(glVersion ? QString::fromLatin1(glVersion) : tr("(unknown)")) + QChar(L'\n');
	
#ifdef GL_SHADING_LANGUAGE_VERSION
	if (glVersion && glVersion[0] >= '2' && glVersion[1] == '.')
	{
		const char *glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		sDebugInfo += tr("GLSL version string:") + QChar(L' ') +
				QString(glslVersion
					? QString::fromLatin1(glslVersion)
					: tr("(unknown)")) + QChar(L'\n');
	}
	
	// OpenGL extensions.
	sDebugInfo += QChar(L'\n');
#ifndef HAVE_GLEW
	sDebugInfo += tr("GLEW disabled; no GL extensions supported.") + QChar(L'\n');
#else
	const char *glewVersion = (const char*)glewGetString(GLEW_VERSION);
	sDebugInfo += QString::fromLatin1("GLEW version ") +
			QString(glewVersion
				? QString::fromLatin1(glewVersion)
				: tr("(unknown)")) + QChar(L'\n') +
			tr("GL extensions in use:") + QChar(L'\n');
	
	// TODO: Print "No GL extensions in use." if no GL extensions are in use.
	const QChar chrBullet(0x2022);	// U+2022: BULLET
	if (GLEW_ARB_fragment_program)
	{
		sDebugInfo += chrBullet + QString::fromLatin1(" ") +
			QString::fromLatin1("GL_ARB_fragment_program") + QChar(L'\n');
	}
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
