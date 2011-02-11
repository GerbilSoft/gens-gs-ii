/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQApplication_win32.cpp: QApplication subclass.                      *
 * Win32-specific functions.                                               *
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

#ifndef _WIN32
#error GensQApplication_win32.cpp should only be compiled on Win32!
#endif

#include "GensQApplication.hpp"

// Include "gqt4_main.hpp" first for main().
#include "gqt4_main.hpp"

// C includes.
#include <string.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// QtCore includes.
#include <QtCore/qt_windows.h>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

// qWinMain declaration.
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);

/**
 * WinMain(): Main entry point on Win32.
 * Code based on libqtmain 4.7.1.
 * Windows CE-specific parts have been removed.
 * @param hInst Instance.
 * @param hPrevInst Previous instance. (Unused on Win32)
 * @param lpCmdLine Command line parameters. (ANSI)
 * @param nCmdShow Main window show parameter.
 * @return Return code.
 */
extern "C"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	QByteArray cmdParam;
	
	wchar_t *cmdW = GetCommandLineW();
	if (cmdW)
	{
		// Unicode system.
		cmdParam = QString::fromWCharArray(cmdW).toLocal8Bit();
	}
	else
	{
		// ANSI system.
		cmdParam = QByteArray(lpCmdLine);
	}
	
	// Tokenize the command line parameters.
	int argc = 0;
	QVector<char*> argv(8);
	qWinMain(hInst, hPrevInst, cmdParam.data(), nCmdShow, argc, argv);
	
	// Call the real main function.
	return gens_main(argc, argv.data());
}

// QtGui includes.
#include <QtGui/QFont>

namespace GensQt4
{

/**
 * winEventFilter(): Win32 event filter.
 * @param msg Win32 message.
 * @param result Return value for the window procedure.
 * @return True if we're handling the message; false if we should let Qt handle the message.
 */
bool GensQApplication::winEventFilter(MSG *msg, long *result)
{
	if (msg->message != WM_SETTINGCHANGE &&
	    msg->wParam != SPI_SETNONCLIENTMETRICS)
	{
		// GensQApplication doesn't handle this message.
		return false;
	}
	
	// WM_SETTINGCHANGE / SPI_SETNONCLIENTMETRICS.
	// Update the Qt font.
	SetFont_Win32();
	
	// Allow QApplication to handle this message anyway.
	return false;
}


/**
 * SetFont_Win32(): Set the Qt font to match the system font.
 */
void GensQApplication::SetFont_Win32(void)
{
	// Get the Win32 message font.
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	int nFontSize = 0;
	HDC hDC = ::GetDC(NULL);
	
	// Calculate the font size in points.
	// http://www.codeguru.com/forum/showthread.php?t=476244
	if (ncm.lfMessageFont.lfHeight < 0)
	{
		nFontSize = -::MulDiv(ncm.lfMessageFont.lfHeight,
				      72, ::GetDeviceCaps(hDC, LOGPIXELSY));
	}
	else
	{
		TEXTMETRIC tm;
		memset(&tm, 0x00, sizeof(tm));
		::GetTextMetrics(hDC, &tm);
		
		nFontSize = -::MulDiv(ncm.lfMessageFont.lfHeight - tm.tmInternalLeading,
				      72, ::GetDeviceCaps(hDC, LOGPIXELSY));
	}
	
	// TODO: Scale Windows font weights to Qt font weights.
	
	// TODO: Menus always use the message font, and they already
	// respond to WM_SETTINGCHANGE. Make menus use the menu font.
	
	// Create the QFont.
	QFont qAppFont(ncm.lfMessageFont.lfFaceName, nFontSize,
		       -1, ncm.lfMessageFont.lfItalic);
	
	// Set the Qt application font.
	QApplication::setFont(qAppFont);
}

}
