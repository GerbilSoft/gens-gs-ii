/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuShortcuts.hpp: Shortcut handler for GensWindow QActions.        *
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

#ifndef __GENS_QT4_WINDOWS_GENSMENUSHORTCUTS_HPP__
#define __GENS_QT4_WINDOWS_GENSMENUSHORTCUTS_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QHash>

// Qt classes.
class QSettings;
class QAction;
class QWidget;
class QMenu;

// LibGensKeys.
#include "libgenskeys/GensKey_t.h"

namespace GensQt4 {

class GensMenuShortcutsPrivate;
class GensMenuShortcuts : public QObject
{
	Q_OBJECT

	public:
		GensMenuShortcuts(QObject *parent = 0);
		virtual ~GensMenuShortcuts();

	private:
		typedef QObject super;
		GensMenuShortcutsPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GensMenuShortcuts)
	private:
		Q_DISABLE_COPY(GensMenuShortcuts)

	public:
		/**
		 * Clear all of the mapped actions.
		 */
		void clear(void);

		/**
		 * Add a QMenu and all of its actions and submenus.
		 * @param menu QMenu.
		 */
		void addMenu(QMenu *menu);

		/**
		 * Update action shortcuts again.
		 * This may be needed if Qt Designer's retranslate function
		 * wipes out the shortcuts due to shortcuts existing in the
		 * UI file.
		 */
		void updateActions(void);

		/**
		 * Look up an action based on a GensKey_t value.
		 * @param key GensKey_t value. (WITH MODIFIERS)
		 * @return QAction object, or nullptr if no action was found.
		 */
		QAction *keyToAction(GensKey_t key) const;

		/**
		 * Look up a GensKey_t based on a QAction object.
		 * @param action QAction object.
		 * @return GensKey_t (WITH MODIFIERS), or 0 if no key was found.
		 */
		int actionToKey(QAction *action) const;

		/**
		 * Load key configuration from a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QSettings *qSettings);

		/**
		 * Save key configuration to a settings file.
		 * NOTE: The group must be selected in the QSettings before calling this function!
		 * @param qSettings Settings file.
		 * @return 0 on success; non-zero on error.
		 */
		int save(QSettings *qSettings) const;

	private slots:
		/**
		 * Mapped QAction was destroyed.
		 * @param obj QAction.
		 */
		void actionDestroyed_slot(QObject *obj);
};

}

#endif /* __GENS_QT4_WINDOWS_GENSMENUSHORTCUTS_HPP__ */
