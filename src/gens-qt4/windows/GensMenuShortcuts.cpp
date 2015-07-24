/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuShortcuts.hpp: Gens key configuration.                              *
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

#include "GensMenuShortcuts.hpp"

// Qt includes.
#include <QtCore/QSettings>
#include <QtCore/QSet>
#include <QtGui/QWidget>
#include <QtGui/QAction>
#include <QtGui/QMenu>

// Needed for KeyValMToQtKey().
#include "Input/KeyHandlerQt.hpp"

// TODO: Create a typedef GensKeyM_t to indicate "with modifiers"?

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

namespace GensQt4 {

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

		struct DefKeySetting_t {
			const char *setting;	// QSettings name.
			const char *qAction;	// QAction object name.

			// TODO: Remove from here; use separate array.
			GensKey_t gensKey;	// Default GensKey_t.
		};

		/**
		 * Default key settings.
		 * Last entry is nullptr.
		 */
		static const DefKeySetting_t DefKeySettings[66+1];

		/// Converts a QAction's name to a DefKeySettings index.
		QHash<QString, int> hashActionToDefKeySetting;

		/// Set of all QActions.
		QSet<QAction*> actions;
		/// Converts a QAction to a GensKey_t.
		QHash<QAction*, GensKey_t> hashActionToKey;
		/// Converts a GensKey_t to a QAction.
		QHash<GensKey_t, QAction*> hashKeyToAction;

		/** Saved configuration. **/
		/// Key configuration. (Indexes match DefKeySettings[].)
		GensKey_t savedKeys[ARRAY_SIZE(DefKeySettings)-1];

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

/** GensMenuShortcutsPrivate **/

// Default key settings.
const GensMenuShortcutsPrivate::DefKeySetting_t GensMenuShortcutsPrivate::DefKeySettings[] =
{
	// File menu.
	{"file/open",			"actionFileOpenROM",			KEYM_CTRL | KEYV_o},
	{"file/recent",			"actionFileRecentROMs",			0},
	// File, Recent ROMs menu.
	// NOTE: Recent ROMs menu  is dynamic.
	// The menu items will have these object names.
	{"file/recent/1",		"actionRecentROMs_1",			KEYM_CTRL | KEYV_1},
	{"file/recent/2",		"actionRecentROMs_2",			KEYM_CTRL | KEYV_2},
	{"file/recent/3",		"actionRecentROMs_3",			KEYM_CTRL | KEYV_3},
	{"file/recent/4",		"actionRecentROMs_4",			KEYM_CTRL | KEYV_4},
	{"file/recent/5",		"actionRecentROMs_5",			KEYM_CTRL | KEYV_5},
	{"file/recent/6",		"actionRecentROMs_6",			KEYM_CTRL | KEYV_6},
	{"file/recent/7",		"actionRecentROMs_7",			KEYM_CTRL | KEYV_7},
	{"file/recent/8",		"actionRecentROMs_8",			KEYM_CTRL | KEYV_8},
	{"file/recent/9",		"actionRecentROMs_9",			KEYM_CTRL | KEYV_9},
	// File menu.
	{"file/close",			"actionFileCloseROM",			KEYM_CTRL | KEYV_w},
	{"file/saveState",		"actionFileSaveState",			KEYV_F5},
	{"file/loadState",		"actionFileLoadState",			KEYV_F8},
#ifdef Q_WS_MAC
	{"file/genConfig",		"actionFileGeneralConfiguration",	KEYM_CTRL | KEYV_COMMA},
#else
	{"file/genConfig",		"actionFileGeneralConfiguration",	0},
#endif
	{"file/mcdControl",		"actionFileSegaCDControlPanel",		0},
	{"file/quit",			"actionFileQuit",			KEYM_CTRL | KEYV_q},

	// Graphics menu.
	{"graphics/showMenuBar",	"actionGraphicsShowMenuBar",		KEYM_CTRL | KEYV_m},
	{"graphics/resolution",		"actionGraphicsResolution",		0},
	// Graphics, Resolution submenu.
	{"graphics/resolution/1x",	"actionGraphicsResolution1x",		0},
	{"graphics/resolution/2x",	"actionGraphicsResolution2x",		0},
	{"graphics/resolution/3x",	"actionGraphicsResolution3x",		0},
	{"graphics/resolution/4x",	"actionGraphicsResolution4x",		0},
	// Graphics menu.
	{"graphics/bpp",		"actionGraphicsBpp",			0},
	// Graphics, Color Depth submenu.
	{"graphics/bpp/15",		"actionGraphicsBpp15",			0},
	{"graphics/bpp/16",		"actionGraphicsBpp16",			0},
	{"graphics/bpp/32",		"actionGraphicsBpp32",			0},
	// Graphics menu.
	{"graphics/stretch",		"actionGraphicsStretch",		0},
	// Graphics, Stretch submenu.
	{"graphics/stretch/none",	"actionGraphicsStretchNone",		0},
	{"graphics/stretch/horizontal",	"actionGraphicsStretchHorizontal",	0},
	{"graphics/stretch/vertical",	"actionGraphicsStretchVertical",	0},
	{"graphics/stretch/full",	"actionGraphicsStretchFull",		0},
	// Graphics menu.
	{"graphics/screenShot",		"actionGraphicsScreenshot",		KEYM_SHIFT | KEYV_BACKSPACE},

	// System menu.
	{"system/region",		"actionSystemRegion",			KEYM_SHIFT | KEYV_F3},
	// System, Region submenu.
	{"system/region/autoDetect",	"actionSystemRegionAuto",		0},
	{"system/region/japan",		"actionSystemRegionJPN",		0},
	{"system/region/asia",		"actionSystemRegionAsia",		0},
	{"system/region/usa",		"actionSystemRegionUSA",		0},
	{"system/region/europe",	"actionSystemRegionEUR",		0},
	// System menu.
	{"system/hardReset",		"actionSystemHardReset",		KEYM_SHIFT | KEYV_TAB},
	{"system/softReset",		"actionSystemSoftReset",		KEYV_TAB},
	{"system/pause",		"actionSystemPause",			KEYV_ESCAPE},
	{"system/resetM68K",		"actionSystemResetM68K",		0},
	{"system/resetS68K",		"actionSystemResetS68K",		0},
	{"system/resetMSH2",		"actionSystemResetMSH2",		0},
	{"system/resetSSH2",		"actionSystemResetSSH2",		0},
	{"system/resetZ80",		"actionSystemResetZ80",			0},

	// Options menu.
	{"options/enableSRam",		"actionOptionsSRAM",			0},
	{"options/controllers",		"actionOptionsControllers",		0},

	// NOTE: Test menus aren't going to be added here.

	// Help menu.
	{"help/about",			"actionHelpAbout",			0},

	// Non-menu keys.
	{"other/fastBlur",		"actionNoMenuFastBlur",			KEYV_F9},

	// TODO: Change to saveSlot/0?
	{"other/saveSlot0",		"actionNoMenuSaveSlot0",		KEYV_0},
	{"other/saveSlot1",		"actionNoMenuSaveSlot1",		KEYV_1},
	{"other/saveSlot2",		"actionNoMenuSaveSlot2",		KEYV_2},
	{"other/saveSlot3",		"actionNoMenuSaveSlot3",		KEYV_3},
	{"other/saveSlot4",		"actionNoMenuSaveSlot4",		KEYV_4},
	{"other/saveSlot5",		"actionNoMenuSaveSlot5",		KEYV_5},
	{"other/saveSlot6",		"actionNoMenuSaveSlot6",		KEYV_6},
	{"other/saveSlot7",		"actionNoMenuSaveSlot7",		KEYV_7},
	{"other/saveSlot8",		"actionNoMenuSaveSlot8",		KEYV_8},
	{"other/saveSlot9",		"actionNoMenuSaveSlot9",		KEYV_9},
	{"other/saveSlotPrev",		"actionNoMenuSaveSlotPrev",		KEYV_F6},
	{"other/saveSlotNext",		"actionNoMenuSaveSlotNext",		KEYV_F7},
	// TODO: Swap these two?
	{"other/saveSlotPrev",		"actionNoMenuLoadStateFrom",		KEYM_SHIFT | KEYV_F8},
	{"other/saveSlotNext",		"actionNoMenuSaveStateAs",		KEYM_SHIFT | KEYV_F5},

	// End of default keys.
	{nullptr, nullptr, 0}
};

GensMenuShortcutsPrivate::GensMenuShortcutsPrivate(GensMenuShortcuts *q)
	: q_ptr(q)
{
	// Initialize actionToDefKeySetting.
	// TODO: Move to initDefaultKeys to save a loop?
	hashActionToDefKeySetting.clear();
	int i = 0;
	for (const DefKeySetting_t *key = &DefKeySettings[0];
	     key->setting != nullptr; key++, i++)
	{
		if (key->qAction != nullptr) {
			hashActionToDefKeySetting.insert(QLatin1String(key->qAction), i);
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
	int i = 0;
	for (const DefKeySetting_t *key = &DefKeySettings[0];
	     key->qAction != 0; key++, i++)
	{
		savedKeys[i] = key->gensKey;
	}

	updateActionMaps();
}

/**
 * Update a QAction.
 * @param action QAction to update.
 */
void GensMenuShortcutsPrivate::updateAction(QAction *action)
{
	// Find this action in DefKeySettings..
	int idx = hashActionToDefKeySetting.value(action->objectName(), -1);
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
		fprintf(stderr, "%s: Action '%s' not present in DefKeySettings.\n",
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
	: QObject(parent)
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
	int i = 0;
	for (const GensMenuShortcutsPrivate::DefKeySetting_t *key = &d->DefKeySettings[0];
	     key->setting != nullptr; key++, i++)
	{
		const GensKey_t gensKey = qSettings->value(
			QLatin1String(key->setting), key->gensKey).toString().toUInt(nullptr, 0);
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
	int i = 0;
	for (const GensMenuShortcutsPrivate::DefKeySetting_t *key = &d->DefKeySettings[0];
	     key->setting != nullptr; key++, i++)
	{
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
