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
#include <QtGui/QMenuBar>
#include <QtGui/QKeySequence>

// EmuManager is needed for some settings.
#include "../EmuManager.hpp"

namespace GensQt4
{

class GensMenuBarPrivate;

class GensMenuBar : public QObject
{
	Q_OBJECT
	
	public:
		GensMenuBar(QObject *parent = NULL, EmuManager *emuManager = NULL);
		virtual ~GensMenuBar();
		
		QMenuBar *createMenuBar(void);
		QMenu *popupMenu(void);
		
		void retranslate(void);
		
		bool menuItemCheckState(int id);
		int setMenuItemCheckState(int id, bool newCheck);
		
		bool isLocked(void);
	
	private:
		enum MenuItemType
		{
			GMI_NORMAL,
			GMI_SEPARATOR,
			GMI_SUBMENU,
			GMI_CHECK,
			GMI_RADIO,
			
			GMI_MAX
		};
		
		struct MenuItem
		{
			int id;				// Menu identifier. (-1 == separator)
			MenuItemType type;		// Menu item type.
			const utf8_str *text;		// Menu item text.
			QAction::MenuRole menuRole;	// (Mac OS X) Menu item role.
			
			int submenu_id;			// Submenu ID.
			const MenuItem *submenu;	// First element of submenu.
			
			const char *icon_fdo;		// FreeDesktop.org icon name.
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
		static const MenuItem ms_gmiGraphics[];
			static const MenuItem ms_gmiGraphicsRes[];
			static const MenuItem ms_gmiGraphicsBpp[];
			static const MenuItem ms_gmiGraphicsStretch[];
		static const MenuItem ms_gmiSystem[];
			static const MenuItem ms_gmiSystemRegion[];
		static const MenuItem ms_gmiOptions[];
		static const MenuItem ms_gmiSoundTest[];
		static const MenuItem ms_gmiHelp[];
		
		// Main menu.
		static const MainMenuItem ms_gmmiMain[];
		
		/** END: Menu definitions. **/
	
	signals:
		void triggered(int id, bool state);
	
	protected:
		/**
		 * lock(), unlock(): Temporarily lock menu actions.
		 * Calls are cumulative; 2 locks requires 2 unlocks.
		 * Calling unlock() when not locked will return an error.
		 * @return 0 on success; non-zero on error.
		 */
		int lock(void);
		int unlock(void);
	
	private:
		friend class GensMenuBarPrivate;
		GensMenuBarPrivate *const d;
		
		Q_DISABLE_COPY(GensMenuBar);
		
	/** Menu synchronization. **/
	
	public:
		void setEmuManager(EmuManager *newEmuManager);
	
	public slots:
		/** Emulation state has changed. **/
		void stateChanged(void);
	
	private slots:
		/** Menu item selection slot. **/
		void menuItemSelected(int id);
		
		/** Menu synchronization slots. **/
		void recentRoms_updated(void);
		// TODO: Port to ConfigStore.
		//void stretchMode_changed_slot(GensConfig::StretchMode_t newStretchMode);
		void regionCode_changed_slot(int newRegionCode); // LibGens::SysVersion::RegionCode_t
		void enableSRam_changed_slot(bool newEnableSRam);
};

}

#endif /* __GENS_QT4_GENSMENUBAR_HPP__ */
