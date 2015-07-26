/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensMenuShortcuts_p.hpp: Shortcut handler for GensWindow QActions.         *
 * (PRIVATE CLASS)                                                            *
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

#ifndef __GENS_QT4_WINDOWS_GENSMENUSHORTCUTS_P_HPP__
#define __GENS_QT4_WINDOWS_GENSMENUSHORTCUTS_P_HPP__

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QSet>
class QAction;

// LibGensKeys.
#include "libgenskeys/GensKey_t.h"

// TODO: Common header file?
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

namespace GensQt4 {

class GensMenuShortcuts;
class GensMenuShortcutsPrivate
{
	public:
		GensMenuShortcutsPrivate(GensMenuShortcuts *q);

	private:
		GensMenuShortcuts *const q_ptr;
		Q_DECLARE_PUBLIC(GensMenuShortcuts)
	private:
		Q_DISABLE_COPY(GensMenuShortcutsPrivate);

	public:
		/**
		 * Key configuration.
		 *
		 * NOTE: Key configuration expects modifiers in high 7 bits of GensKey_t.
		 * See libgens/GensInput/GensKey_t.h for more information.
		 */

		/** Active QAction maps. **/

		static const int KeyBinding_count = 65;
		struct KeyBinding_t {
			const char *setting;	// QSettings name.
			const char *qAction;	// QAction object name.
		};

		/**
		 * Key bindings.
		 * This maps QSettings entries to QActions.
		 * Last entry is nullptr.
		 */
		static const KeyBinding_t KeyBindings[KeyBinding_count+1];

		/**
		 * Default key bindings for Gens/GS II.
		 */
		static const GensKey_t DefKeyBindings_gens[KeyBinding_count+1];

		/**
		 * Default key bindings for Kega Fusion 3.63x.
		 */
		static const GensKey_t DefKeyBindings_kega[KeyBinding_count+1];

		/// Converts a QAction's name to a KeyBindings index.
		QHash<QString, int> hashActionToKeyBinding;

		/// Set of all QActions.
		QSet<QAction*> actions;
		/// Converts a QAction to a GensKey_t.
		QHash<QAction*, GensKey_t> hashActionToKey;
		/// Converts a GensKey_t to a QAction.
		QHash<GensKey_t, QAction*> hashKeyToAction;

		/** Saved configuration. **/
		/// Key configuration. (Indexes match KeyBindings[].)
		GensKey_t savedKeys[KeyBinding_count];

		/**
		 * Initialize savedKeys with the default key configuration.
		 */
		void initDefaultKeys(void);

	protected:
		/**
		 * Update a QAction.
		 * @param action QAction to update.
		 */
		void updateAction(QAction *action);

	public:
		/**
		 * Update the QAction maps.
		 */
		void updateActionMaps(void);

		/**
		 * Add an action.
		 * @param action Action to add.
		 */
		void addAction(QAction *action);
};

}

#endif /* __GENS_QT4_WINDOWS_GENSMENUSHORTCUTS_P_HPP__ */
