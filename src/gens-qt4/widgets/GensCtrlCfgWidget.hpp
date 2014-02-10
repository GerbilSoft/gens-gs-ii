/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensCtrlCfgWidget.hpp: Controller configuration widget.                 *
 *                                                                         *
 * Copyright (c) 2011-2014 by David Korth.                                 *
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
#include "libgens/IO/IoManager.hpp"
#include "libgens/GensInput/GensKey_t.h"

// Qt includes.
#include <QtGui/QWidget>
#include <QtCore/QVector>

namespace GensQt4
{

class GensCtrlCfgWidgetPrivate;

class GensCtrlCfgWidget : public QWidget
{
	Q_OBJECT

	public:
		GensCtrlCfgWidget(QWidget* parent = 0);
		virtual ~GensCtrlCfgWidget();

	private:
		GensCtrlCfgWidgetPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GensCtrlCfgWidget)
	private:
		Q_DISABLE_COPY(GensCtrlCfgWidget)

	public:
		/**
		 * Get the current I/O device type.
		 * @return Current I/O device type.
		 */
		LibGens::IoManager::IoType_t ioType(void) const;

		/**
		 * Set the I/O device type.
		 * @param newIoType New I/O device type.
		 */
		void setIoType(LibGens::IoManager::IoType_t newIoType);

		/**
		 * Get the current keymap.
		 * @return Current keymap.
		 */
		QVector<GensKey_t> keyMap(void) const;

		/**
		 * Set the current keymap.
		 * @param keyMap New keymap.
		 */
		void setKeyMap(QVector<GensKey_t> keyMap);

	signals:
		/**
		 * A key's configuration has been changed.
		 * @param idx Button index.
		 * @param gensKey New GensKey_t value.
		 */
		void keyChanged(int idx, GensKey_t gensKey);

	private slots:
		void clearAllButtons(void);
		void keyChanged_slot(int idx);
};

}

#endif /* __GENS_QT4_WIDGETS_GENSCTRLCFGWIDGET_HPP__ */
