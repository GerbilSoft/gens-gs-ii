/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensActions.hpp: Actions handler.                                       *
 * Handles menu events and non-menu actions.                               *
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

#ifndef __GENS_QT4_INPUT_ACTIONS_GENSACTIONS_HPP__
#define __GENS_QT4_INPUT_ACTIONS_GENSACTIONS_HPP__

// Gens Keys.
#include "libgens/GensInput/GensKey_t.h"

// Qt includes.
#include <QtCore/QObject>

namespace GensQt4
{

// Gens window.
class GensWindow;

class GensActions : public QObject
{
	Q_OBJECT
	
	public:
		GensActions(GensWindow *parent);
		
		/**
		 * checkEventKey(): Check for non-menu event keys.
		 * @param key Gens Keycode. (WITH MODIFIERS)
		 * @return True if an event key was processed; false if not.
		 */
		bool checkEventKey(GensKey_t key);
	
	public slots:
		/**
		 * doAction(): Do an action.
		 * @param id Action ID. (from GensMenuBar_menus.hpp)
		 * @return True if handled; false if not.
		 */
		bool doAction(int id);
	
	protected:
		GensWindow *m_parent;
	
	signals:
		void actionTogglePaused(void);
		
		/**
		 * actionResetEmulator(): Reset the emulator.
		 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
		 */
		void actionResetEmulator(bool hardReset);
};

}

#endif /* __GENS_QT4_INPUT_ACTIONS_GENSACTIONS_HPP__ */
