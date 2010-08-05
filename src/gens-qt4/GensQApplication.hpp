/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQApplication.hpp: QApplication subclass.                            *
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

#ifndef __GENS_QT4_GENSQAPPLICATION_HPP__
#define __GENS_QT4_GENSQAPPLICATION_HPP__

#include <QtGui/QApplication>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "gqt4_win32.hpp"
#endif

#include <stdio.h>

namespace GensQt4
{

class GensQApplication : public QApplication
{
	Q_OBJECT
	
	public:
		GensQApplication(int &argc, char **argv)
			: QApplication(argc, argv) { }
		GensQApplication(int &argc, char **argv, bool GUIenabled)
			: QApplication(argc, argv, GUIenabled) { }
		GensQApplication(int &argc, char **argv, Type type)
			: QApplication(argc, argv, type) { }
		virtual ~GensQApplication() { }
		
#ifdef _WIN32
		/**
		 * winEventFilter(): Win32 event filter.
		 * @param msg Win32 message.
		 * @param result Return value for the window procedure.
		 * @return True if we're handling the message; false if we should let Qt handle the message.
		 */
		bool winEventFilter(MSG *msg, long *result)
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
#endif
};

}

#endif /* __GENS_QT4_GENSQAPPLICATION_HPP__ */
