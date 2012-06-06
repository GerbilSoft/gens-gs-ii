/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * RecentRomsMenu.hpp: Recent ROMs Menu.                                   *
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

#ifndef __GENS_QT4_ACTIONS_RECENTROMSMENU_HPP__
#define __GENS_QT4_ACTIONS_RECENTROMSMENU_HPP__

// Qt includes.
#include <QtGui/QMenu>

// Recent ROMs class.
#include "../Config/RecentRoms.hpp"

namespace GensQt4
{

class RecentRomsMenuPrivate;

class RecentRomsMenu : public QMenu
{
	Q_OBJECT
	
	public:
		RecentRomsMenu(QWidget *parent = 0, const RecentRoms *recentRoms = 0);
		RecentRomsMenu(QString title, QWidget *parent = 0, const RecentRoms *recentRoms = 0);
		~RecentRomsMenu();
		
		const RecentRoms *recentRoms(void);
		void setRecentRoms(const RecentRoms *newRecentRoms);
	
	signals:
		/**
		 * updated(): The Recent ROMs list was updated.
		 */
		void updated(void);
		
		/**
		 * triggered(): A Recent ROM was selected.
		 * @param id Menu item ID.
		 */
		void triggered(int id);
	
	private:
		friend class RecentRomsMenuPrivate;
		RecentRomsMenuPrivate *const d;
		
		Q_DISABLE_COPY(RecentRomsMenu)
	
	private slots:
		// d->recentRoms signals.
		void recentRomsDestroyed(void);
		void recentRomsUpdated(void);
};

}

#endif /* __GENS_QT4_ACTIONS_RECENTROMSMENU_HPP__ */
