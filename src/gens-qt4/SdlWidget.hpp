/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * SdlWidget.hpp: SDL Widget class.                                        *
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

#ifndef __GENS_QT4_SDLWIDGET_HPP__
#define __GENS_QT4_SDLWIDGET_HPP__

#include <QtCore/qglobal.h>
#include <QtCore/QObject>

#ifdef Q_WS_X11
#include <QtGui/QX11EmbedContainer>
#define SDLWIDGET_BASECLASS QX11EmbedContainer
#else
// TODO: Verify that this works properly!
#include <QtGui/QWidget>
#define SDLWIDGET_BASECLASS QWidget
#endif


namespace GensQt4
{

class SdlWidget : public SDLWIDGET_BASECLASS
{
	Q_OBJECT
	
	public:
		SdlWidget(QWidget *parent = NULL);
	
	protected:
		void paintEvent(QPaintEvent *event);
};

}

#endif /* __GENS_QT4_SDLWIDGET_HPP__ */
