/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * RecentRomsMenu.hpp: Recent ROMs Menu.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __GENS_QT4_WIDGETS_RECENTROMSMENU_HPP__
#define __GENS_QT4_WIDGETS_RECENTROMSMENU_HPP__

// Qt includes.
#include <QtGui/QMenu>

// Recent ROMs class.
#include "../Config/RecentRoms.hpp"

namespace GensQt4 {

class RecentRomsMenuPrivate;
class RecentRomsMenu : public QMenu
{
	Q_OBJECT

	public:
		RecentRomsMenu(QWidget *parent = 0, const RecentRoms *recentRoms = 0);
		RecentRomsMenu(const QString &title, QWidget *parent = 0, const RecentRoms *recentRoms = 0);
		virtual ~RecentRomsMenu();

	private:
		typedef QMenu super;
		RecentRomsMenuPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(RecentRomsMenu)
	private:
		Q_DISABLE_COPY(RecentRomsMenu)

	public:
		const RecentRoms *recentRoms(void);
		void setRecentRoms(const RecentRoms *recentRoms);

	signals:
		/**
		 * The Recent ROMs list was updated.
		 */
		void updated(void);

		/**
		 * A Recent ROM was selected.
		 * @param idx Index of the recent ROM that was selected.
		 */
		void triggered(int idx);

	private slots:
		// d->recentRoms signals.
		void recentRomsDestroyed_slot(void);
		void recentRomsUpdated_slot(void);
};

}

#endif /* __GENS_QT4_WIDGETS_RECENTROMSMENU_HPP__ */
