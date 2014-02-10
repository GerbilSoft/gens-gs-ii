/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar_p.hpp: Gens Menu Bar class.                                 *
 * (PRIVATE HEADER)                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#ifndef __GENS_QT4_ACTIONS_GENSMENUBAR_P_HPP__
#define __GENS_QT4_ACTIONS_GENSMENUBAR_P_HPP__

// Qt includes and classes.
#include <QtCore/QSignalMapper>
#include <QtCore/QHash>
class QAction;

// LibGens includes.
#include "libgens/macros/common.h"

// GensMenuBar
#include "GensMenuBar.hpp"

namespace GensQt4
{

class RecentRomsMenu;
class EmuManager;

class GensMenuBarPrivate
{
	public:
		GensMenuBarPrivate(GensMenuBar *q);
		~GensMenuBarPrivate();

	private:
		GensMenuBar *const q_ptr;
		Q_DECLARE_PUBLIC(GensMenuBar);
	private:
		Q_DISABLE_COPY(GensMenuBarPrivate)

	public:
		void init(EmuManager *initEmuManager = 0);

		void setEmuManager(EmuManager *newEmuManager);

		void retranslate(void);

		int lock(void);
		int unlock(void);
		bool isLocked(void) const;

	private:
		int lockCnt;		// Lock counter.
	
	public:
		// Menu parsing functions.
		struct MainMenuItem;
		struct MenuItem;
		void parseMainMenu(const GensMenuBar::MainMenuItem *mainMenu);
		void parseMenu(const GensMenuBar::MenuItem *menu, QMenu *parent);

		// Hash table of QActions.
		QHash<int, QAction*> hashActions;

		QMenu *popupMenu;	// Popup menu.
		EmuManager *emuManager;	// Emulation Manager.

		// Recent ROMs menu.
		RecentRomsMenu *recentRomsMenu;

		void syncConnect(void);	// Connect menu synchronization slots.

		void syncAll(void);		// Synchronize all menus.
		void syncRecent(void);		// Synchronize the "Recent ROMs" menu.
		void syncShowMenuBar(void);	// Synchronize the "Show Menu Bar" item.

	private:
		QSignalMapper *signalMapper;

		// Clear the hash tables.
		void clearHashTables(void);

		// List of menu separators.
		QVector<QAction*> lstSeparators;
};

}

#endif /* __GENS_QT4_ACTIONS_GENSMENUBAR_P_HPP__ */
