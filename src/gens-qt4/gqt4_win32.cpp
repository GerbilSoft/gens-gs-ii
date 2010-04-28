/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_win32.hpp: Win32 compatibility functions.                          *
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

#include "gqt4_win32.hpp"

// C includes.
#include <string.h>

// Qt includes.
#include <QtGui/QApplication>
#include <QtGui/QFont>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace GensQt4
{

/**
 * Win32_SetFont(): Set the Qt font to match the system font.
 */
void Win32_SetFont(void)
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
	
	// TODO: Update the Qt font when the system font is changed.
	// (WM_SETTINGCHANGE)
	
	// TODO: Menus always use the message font, and they already
	// respond to WM_SETTINGCHANGE. Make menus use the menu font.
	
	// Create the QFont.
	QFont qAppFont(ncm.lfMessageFont.lfFaceName, nFontSize,
		       -1, ncm.lfMessageFont.lfItalic);
	
	// Set the Qt application font.
	QApplication::setFont(qAppFont);
}

}
