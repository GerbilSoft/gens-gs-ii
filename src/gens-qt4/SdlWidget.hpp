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

#include "libgens/lg_main.hpp"

// TODO: Use something other than QX11EmbedWidget on Windows and OS X.
// Using it on Linux because other types result in a BadWindow error,
// thanks to Qt4's alien windows feature. (Qt 4.6.2 seems to be ignoring
// the requests to use native windows instead of alien windows.)


#include <QObject>
#include <QX11EmbedWidget>

namespace GensQt4
{

class SdlWidget : public QX11EmbedWidget
{
	Q_OBJECT
	
	public:
		SdlWidget(QWidget *parent = NULL)
		{
			this->setParent(parent);
			
			// Make sure the window is embedded properly.
			this->setAttribute(Qt::WA_NativeWindow);
			this->setAttribute(Qt::WA_PaintOnScreen);
			this->setAttribute(Qt::WA_OpaquePaintEvent);
		}
	
	protected:
		void paintEvent(QPaintEvent *event)
		{
			// Send an UPDATE event to LibGens.
			// TODO: Include the coordinates to update?
			LibGens::qToLG->push(LibGens::MtQueue::MTQ_LG_UPDATE, NULL);
		}
};

}

#endif /* __GENS_QT4_SDLWIDGET_HPP__ */
