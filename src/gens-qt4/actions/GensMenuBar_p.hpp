/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar_p.hpp: Gens Menu Bar class.                                 *
 * (PRIVATE HEADER)                                                        *
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

#ifndef __GENS_QT4_ACTIONS_GENSMENUBAR_P_HPP__
#define __GENS_QT4_ACTIONS_GENSMENUBAR_P_HPP__

// Qt includes and classes.
#include <QtCore/QSignalMapper>
#include <QtCore/QHash>
#include <QtGui/QAction>

// LibGens includes.
#include "libgens/macros/common.h"

namespace GensQt4
{

class GensMenuBar;
class RecentRomsMenu;
class EmuManager;

class GensMenuBarPrivate
{
	public:
		GensMenuBarPrivate(GensMenuBar *q);
		~GensMenuBarPrivate();

		void init(EmuManager *initEmuManager = 0);

		void setEmuManager(EmuManager *newEmuManager);

		void retranslate(void);

		int lock(void);
		int unlock(void);
		bool isLocked(void) const;

	private:
		GensMenuBar *const q;
		Q_DISABLE_COPY(GensMenuBarPrivate)

		int lockCnt;		// Lock counter.
	
	public:
		// Menu parsing functions.
		struct MainMenuItem;
		struct MenuItem;
		void parseMainMenu(const MainMenuItem *mainMenu);
		void parseMenu(const MenuItem *menu, QMenu *parent);

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

	public:
		enum MenuItemType {
			GMI_NORMAL,
			GMI_SEPARATOR,
			GMI_SUBMENU,
			GMI_CHECK,
			GMI_RADIO,

			GMI_MAX
		};
		
		struct MenuItem {
			int id;				// Menu identifier. (-1 == separator)
			MenuItemType type;		// Menu item type.
			const utf8_str *text;		// Menu item text.
			QAction::MenuRole menuRole;	// (Mac OS X) Menu item role.

			int submenu_id;			// Submenu ID.
			const MenuItem *submenu;	// First element of submenu.

			const char *icon_fdo;		// FreeDesktop.org icon name.
		};

		struct MainMenuItem {
			int id;				// Menu identifier.
			const utf8_str *text;		// Menu text.
			const MenuItem *submenu;	// First element of submenu.
		};

		/**
		 * Menu definitions.
		 * These are located in GensMenuBar_menus.cpp.
		 */

		// Top-level menus.
		static const MenuItem gmiFile[];
		static const MenuItem gmiGraphics[];
			static const MenuItem gmiGraphicsRes[];
			static const MenuItem gmiGraphicsBpp[];
			static const MenuItem gmiGraphicsStretch[];
		static const MenuItem gmiSystem[];
			static const MenuItem gmiSystemRegion[];
		static const MenuItem gmiOptions[];
		static const MenuItem gmiSoundTest[];
		static const MenuItem gmiHelp[];

		// Main menu.
		static const MainMenuItem gmmiMain[];

		/** END: Menu definitions. **/
};

}

#endif /* __GENS_QT4_ACTIONS_GENSMENUBAR_P_HPP__ */
