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

#include "GensMenuShortcuts.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Qt includes.
#include <QtCore/QSettings>
#include <QtCore/QSet>
#include <QtGui/QWidget>
#include <QtGui/QAction>
#include <QtGui/QMenu>

// Needed for KeyValMToQtKey().
#include "Input/KeyHandlerQt.hpp"

// TODO: Create a typedef GensKeyM_t to indicate "with modifiers"?

#include "GensMenuShortcuts_p.hpp"
namespace GensQt4 {

/** GensMenuShortcutsPrivate **/

GensMenuShortcutsPrivate::GensMenuShortcutsPrivate(GensMenuShortcuts *q)
	: q_ptr(q)
{
	// Make sure KeyBindings is the correct size.
	// Check for too small.
	assert(KeyBindings[ARRAY_SIZE(KeyBindings)-1].setting == nullptr);
	// Check for too big.
	assert(KeyBindings[ARRAY_SIZE(KeyBindings)-2].setting != nullptr);

	// Initialize actionToDefKeySetting.
	// TODO: Move to initDefaultKeys to save a loop?
	hashActionToKeyBinding.clear();
	for (int i = 0; i < (int)(ARRAY_SIZE(KeyBindings)-1); i++) {
		const GensMenuShortcutsPrivate::KeyBinding_t *key = &KeyBindings[i];
		if (key->qAction != nullptr) {
			hashActionToKeyBinding.insert(QLatin1String(key->qAction), i);
		}
	}

	// Initialize the default keys.
	initDefaultKeys();
}

/**
 * Initialize savedKeys with the default key configuration.
 */
void GensMenuShortcutsPrivate::initDefaultKeys(void)
{
	// Using the Gens/GS II key bindings by default.
	for (int i = 0; i < (int)(ARRAY_SIZE(KeyBindings)-1); i++) {
		savedKeys[i] = DefKeyBindings_gens[i];
	}

	updateActionMaps();
}

/**
 * Update a QAction.
 * @param action QAction to update.
 */
void GensMenuShortcutsPrivate::updateAction(QAction *action)
{
	// Find this action in KeyBindings..
	int idx = hashActionToKeyBinding.value(action->objectName(), -1);
	if (idx >= 0 && idx < (int)ARRAY_SIZE(savedKeys)-1) {
		// Action found.
		GensKey_t gensKey = savedKeys[idx];
		if (gensKey > 0) {
			hashActionToKey.insert(action, gensKey);
			hashKeyToAction.insert(gensKey, action);

			// Set the QAction's shortcut.
			action->setShortcut(KeyHandlerQt::KeyValMToQtKey(gensKey));
		}
	} else {
		// Action not found.
		// TODO: LOG_MSG?
		fprintf(stderr, "%s: Action '%s' not present in KeyBindings.\n",
			__func__, action->objectName().toUtf8().constData());
	}
}

/**
 * Update all QActions.
 */
void GensMenuShortcutsPrivate::updateActionMaps(void)
{
	hashActionToKey.clear();
	hashKeyToAction.clear();

	// Process all actions.
	foreach (QAction *action, this->actions) {
		// Update this action.
		updateAction(action);
	}

	// TODO: Check for missing actions?
}

/**
 * Add an action.
 * @param action Action to add.
 */
void GensMenuShortcutsPrivate::addAction(QAction* action)
{
	if (actions.contains(action)) {
		// Action is already part of the set.
		return;
	}

	// Action is not already part of the set.
	Q_Q(GensMenuShortcuts);
	actions.insert(action);
	QObject::connect(action, SIGNAL(destroyed(QObject*)),
			 q, SLOT(actionDestroyed_slot(QObject*)));

	// Update the action.
	updateAction(action);
}

/** GensMenuShortcuts **/

GensMenuShortcuts::GensMenuShortcuts(QObject *parent)
	: super(parent)
	, d_ptr(new GensMenuShortcutsPrivate(this))
{ }

GensMenuShortcuts::~GensMenuShortcuts()
{
	delete d_ptr;
}

/**
 * Clear all of the mapped actions.
 */
void GensMenuShortcuts::clear(void)
{
	Q_D(GensMenuShortcuts);

	// Disconnect all slots.
	foreach (QAction *action, d->actions) {
		disconnect(action, SIGNAL(destroyed(QObject*)),
			   this, SLOT(actionDestroyed_slot(QObject*)));
	}
	d->actions.clear();
}

/**
 * Add a QMenu and all of its actions and submenus.
 * @param menu QMenu.
 */
void GensMenuShortcuts::addMenu(QMenu *menu)
{
	Q_D(GensMenuShortcuts);
	foreach (QAction *action, menu->actions()) {
		if (action->isSeparator())
			continue;

		d->addAction(action);
		// If this action has a submenu, process it.
		if (action->menu() != nullptr) {
			addMenu(action->menu());
		}
	}
}

/**
 * Update action shortcuts again.
 * This may be needed if Qt Designer's retranslate function
 * wipes out the shortcuts due to shortcuts existing in the
 * UI file.
 */
void GensMenuShortcuts::updateActions(void)
{
	Q_D(GensMenuShortcuts);
	d->updateActionMaps();
}

/**
 * Look up an action based on a GensKey_t value.
 * @param key GensKey_t value. (WITH MODIFIERS)
 * @return QAction object, or nullptr if no action was found.
 */
QAction *GensMenuShortcuts::keyToAction(GensKey_t key) const
{
	Q_D(const GensMenuShortcuts);
	return d->hashKeyToAction.value(key, nullptr);
}

/**
 * Look up a GensKey_t based on a QAction object.
 * @param action QAction object.
 * @return GensKey_t (WITH MODIFIERS), or 0 if no key was found.
 */
int GensMenuShortcuts::actionToKey(QAction *action) const
{
	Q_D(const GensMenuShortcuts);
	return d->hashActionToKey.value(action, 0);
}

/**
 * Load key configuration from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return 0 on success; non-zero on error.
 */
int GensMenuShortcuts::load(const QSettings *qSettings)
{
	Q_D(GensMenuShortcuts);

	// Load the key configuration.
	for (int i = 0; i < (int)(ARRAY_SIZE(d->KeyBindings)-1); i++) {
		const GensMenuShortcutsPrivate::KeyBinding_t *key = &d->KeyBindings[i];
		assert(key->setting != nullptr);

		const GensKey_t gensKey = qSettings->value(
			QLatin1String(key->setting),
			GensMenuShortcutsPrivate::DefKeyBindings_gens[i]).toString().toUInt(nullptr, 0);
		d->savedKeys[i] = gensKey;
	}

	// Update the action maps.
	d->updateActionMaps();

	// Key configuration loaded.
	return 0;
}

/**
 * Save key configuration to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return 0 on success; non-zero on error.
 */
int GensMenuShortcuts::save(QSettings *qSettings) const
{
	Q_D(const GensMenuShortcuts);

	// Save the key configuration.
	for (int i = 0; i < (int)(ARRAY_SIZE(d->KeyBindings)-1); i++) {
		const GensMenuShortcutsPrivate::KeyBinding_t *key = &d->KeyBindings[i];
		assert(key->setting != nullptr);

		const GensKey_t gensKey = d->savedKeys[i];
		QString gensKey_str = QLatin1String("0x") +
				QString::number(gensKey, 16).toUpper().rightJustified(4, QChar(L'0'));

		qSettings->setValue(QLatin1String(key->setting), gensKey_str);
	}

	// Key configuration saved.
	return 0;
}

/** Private slots. **/

/**
 * Mapped QAction was destroyed.
 * @param obj QAction.
 */
void GensMenuShortcuts::actionDestroyed_slot(QObject *obj)
{
	Q_D(GensMenuShortcuts);
	GensKey_t gensKey = d->hashActionToKey.value((QAction*)obj);
	d->actions.remove((QAction*)obj);
	if (gensKey > 0) {
		// TODO: Multiple keys per action?
		d->hashActionToKey.remove((QAction*)obj);
		d->hashKeyToAction.remove(gensKey);
	}
}

}
