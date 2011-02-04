/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar.hpp: Gens Menu Bar class.                                   *
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

#ifndef __GENS_QT4_GENSMENUBAR_HPP__
#define __GENS_QT4_GENSMENUBAR_HPP__

// LibGens includes. (utf8_str)
#include "libgens/macros/common.h"

// Qt includes.
#include <QtCore/QSignalMapper>
#include <QtCore/QHash>
#include <QtGui/QMenuBar>
#include <QtGui/QKeySequence>

namespace GensQt4
{

class GensMenuBar : public QMenuBar
{
	Q_OBJECT
	
	public:
		GensMenuBar(QWidget *parent = NULL);
		virtual ~GensMenuBar();
	
		bool menuItemCheckState(int id);
		int setMenuItemCheckState(int id, bool newCheck);
	
	protected:
		enum MenuItemType
		{
			GMI_NORMAL,
			GMI_SEPARATOR,
			GMI_SUBMENU,
			GMI_CHECK,
			
			GMI_MAX
		};
		
		enum MenuItemShortcut
		{
			MACCEL_NONE = 0,
			
#if QT_VERSION >= 0x040600
			// Keys defined in Qt 4.6.
			MACCEL_QUIT = (int)QKeySequence::Quit,
			MACCEL_PREFERENCES = (int)QKeySequence::Preferences,
#else
			MACCEL_QUIT = 0,
			MACCEL_PREFERENCES = 0,
#endif
#if QT_VERSION >= 0x040200
			// Keys defined in Qt 4.2.
			// (First version with StandardKey.)
			MACCEL_OPEN = (int)QKeySequence::Open,
			MACCEL_CLOSE = (int)QKeySequence::Close,
#else
			MACCEL_OPEN = 0,
			MACCEL_CLOSE = 0,
#endif
		};
		
		struct MenuItem
		{
			int id;				// Menu identifier. (-1 == separator)
			MenuItemType type;		// Menu item type.
			const utf8_str *text;		// Menu item text.
			const MenuItem *submenu;	// First element of submenu.
			
			MenuItemShortcut key_std;	// Menu item shortcut. (Wrapper around QKeySequence::StandardKey.)
			int key_custom;			// Custom key sequence. (Set key_std to QKeySequence::UnknownKey.)
			
			const char *icon_fdo;		// FreeDesktop.org icon.
			const char *icon_qrc;		// QRC icon. (Qt resources)
		};
		
		struct MainMenuItem
		{
			int id;				// Menu identifier.
			const utf8_str *text;		// Menu text.
			const MenuItem *submenu;	// First element of submenu.
		};
		
		/**
		 * Menu definitions.
		 * These are located in GensMenuBar_menus.cpp.
		 */
		
		// Top-level menus.
		static const MenuItem ms_gmiFile[];
		static const MenuItem ms_gmiResBppTest[];
		static const MenuItem ms_gmiCtrlTest[];
		static const MenuItem ms_gmiSoundTest[];
		static const MenuItem ms_gmiHelp[];
		
		// Main menu.
		static const MainMenuItem ms_gmmiMain[];
		
		/** END: Menu definitions. **/
		
		void parseMainMenu(const MainMenuItem *mainMenu);
		void parseMenu(const MenuItem *menu, QMenu *parent);
		
		QSignalMapper *m_signalMapper;
		
		// Hash tables of QActions and QMenus.
		// List of menu separators.
		QHash<int, QAction*> m_hashActions;
		QHash<int, QMenu*> m_hashMenus;
		QList<QAction*> m_lstSeparators;
		
		void clearHashTables(void);
	
	signals:
		void triggered(int id);
};

}

#endif /* __GENS_QT4_GENSMENUBAR_HPP__ */
