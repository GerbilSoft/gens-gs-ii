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

#ifndef __GENS_QT4_GQT4_WIN32_HPP__
#define __GENS_QT4_GQT4_WIN32_HPP__

#ifndef _WIN32
#error gqt4_win32.hpp should only be #included on Win32!
#endif

namespace GensQt4
{

void Win32_SetFont(void);

}

#endif /* __GENS_QT4_GQT4_WIN32_HPP__ */
