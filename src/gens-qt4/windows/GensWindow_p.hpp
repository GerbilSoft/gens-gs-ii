/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow_p.hpp: Gens Window. (PRIVATE CLASS)                             *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2015 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#ifndef __GENS_QT4_WINDOWS_GENSWINDOW_P_HPP__
#define __GENS_QT4_WINDOWS_GENSWINDOW_P_HPP__

#include "gqt4_datatypes.h"
#include "libgens/MD/SysVersion.hpp"

#include "ui_GensWindow.h"
namespace GensQt4 {

class EmuManager;
class KeyHandlerQt;
class VBackend;
class RecentRomsMenu;

class GensWindowPrivate
{
	public:
		GensWindowPrivate(GensWindow *q);
		~GensWindowPrivate();

	private:
		GensWindow *const q_ptr;
		Q_DECLARE_PUBLIC(GensWindow)
	private:
		Q_DISABLE_COPY(GensWindowPrivate)

	public:
		Ui::GensWindow ui;

		// TODO: Remove this once the new menu bar is fully working.
		//GensMenuBar *gensMenuBar;

		EmuManager *emuManager;
		KeyHandlerQt *keyHandler;
		VBackend *vBackend;

		// Menu bar.
		void initMenuBar(void);
		void updateMenuBarVisibility(void);
		bool isGlobalMenuBar(void) const;
		bool isShowMenuBar(void) const;

		// Popup menu.
		// Used when the menu bar is hidden on
		// systems without a global menu bar.
		QMenu *popupMenu;

		// Recent ROMs menu.
		RecentRomsMenu *recentRomsMenu;

		int scale;		// Temporary scaling variable.
		bool hasInitResize;	// Has the initial resize occurred?

		// Resize the window.
		void gensResize(void);

		// Set the Gens window title.
		void setGensTitle(void);

		/** Configuration items. **/
		bool cfg_autoPause;
		int cfg_introStyle;
		bool cfg_showMenuBar;

		/** Idle thread. **/
		IdleThread *idleThread;
		bool idleThreadAllowed;
		void checkIdleThread(void);

	public:
		/** Menu synchronization. (GensWindow_menu_sync.cpp) **/
		void syncAll(void);		// Synchronize all menus.
		void syncRecent(void);		// Synchronize the "Recent ROMs" menu.
		void syncShowMenuBar(void);	// Synchronize the "Show Menu Bar" item.
		void updateMenusForStateChanged(void);

		// Internal functions for synchronization slots.
		void stretchMode_changed_slot_int(StretchMode_t stretchMode);
		void regionCode_changed_slot_int(LibGens::SysVersion::RegionCode_t regionCode);
		void enableSRam_changed_slot_int(bool enableSRam);
};

/**
 * Do we have a global menu bar?
 * @return True if yes; false if no.
 */
inline bool GensWindowPrivate::isGlobalMenuBar(void) const
{
	// TODO: Support Unity and Qt global menu bars.
#ifdef Q_WS_MAC
	return true;
#else
	return false;
#endif
}

/**
 * Is the menu bar visible?
 * @return True if yes; false if no.
 */
inline bool GensWindowPrivate::isShowMenuBar(void) const
{
	return (isGlobalMenuBar() || cfg_showMenuBar);
}

}

#endif /* __GENS_QT4_WINDOWS_GENSWINDOW_P_HPP__ */
