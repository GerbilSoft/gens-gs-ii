/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensCtrlCfgWidget.hpp: Controller configuration widget.                 *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#ifndef __GENS_QT4_WIDGETS_GENSCTRLCFGWIDGET_HPP__
#define __GENS_QT4_WIDGETS_GENSCTRLCFGWIDGET_HPP__

// LibGens includes.
#include "libgens/IO/IoBase.hpp"

// Qt includes.
#include <QtGui/QWidget>

namespace GensQt4
{

class GensCtrlCfgWidgetPrivate;

class GensCtrlCfgWidget : public QWidget
{
	Q_OBJECT
	
	public:
		GensCtrlCfgWidget(QWidget* parent = 0);
		~GensCtrlCfgWidget();
		
		LibGens::IoBase::IoType ioType(void);
		void setIoType(LibGens::IoBase::IoType newIoType);
	
	private:
		friend class GensCtrlCfgWidgetPrivate;
		GensCtrlCfgWidgetPrivate *const d;
		
		Q_DISABLE_COPY(GensCtrlCfgWidget)
};

}

#endif /* __GENS_QT4_WIDGETS_GENSCTRLCFGWIDGET_HPP__ */
