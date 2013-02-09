/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensKeyConfig.hpp: Gens key configuration.                              *
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

#include "GensKeyConfig.hpp"

// Menu actions.
#include "GensMenuBar_menus.hpp"

// Qt includes.
#include <QtCore/QSettings>

// TODO: Create a typedef GensKeyM_t to indicate "with modifiers"?

namespace GensQt4
{

class GensKeyConfigPrivate
{
	public:
		GensKeyConfigPrivate() { }

		/**
		 * Key configuration.
		 * 
		 * NOTE: Key configuration expects modifiers in high 7 bits of GensKey_t.
		 * See libgens/GensInput/GensKey_t.h for more information.
		 */

		/// Converts a GensMenuBar_menus.hpp value to a GensKey_t.
		QHash<int, GensKey_t> hashActionToKey;

		/// Converts a GensKey_t to a GensMenuBar_menus.hpp value.
		QHash<GensKey_t, int> hashKeyToAction;

		struct DefKeySetting_t
		{
			int action;		// GensMenuBar_menus.hpp value.
			GensKey_t gensKey;	// Default GensKey_t.
			
			const char *setting;	// Settings location.
			// TODO: Padding on 32-bit.
		};

		/**
		 * Default key settings.
		 */
		static const DefKeySetting_t DefKeySettings[];

	private:
		Q_DISABLE_COPY(GensKeyConfigPrivate);
};


/**
 * ms_DefKeySettings[]: Default key settings.
 */
const GensKeyConfigPrivate::DefKeySetting_t GensKeyConfigPrivate::DefKeySettings[] =
{
	// File menu.
	{IDM_FILE_OPEN, KEYM_CTRL | KEYV_o,			"file/open"},
	{IDM_FILE_RECENT, 0,					"file/recent"},
	// File, Recent ROMs menu.
	{IDM_FILE_RECENT_1, KEYM_CTRL | KEYV_1,			"file/recent/1"},
	{IDM_FILE_RECENT_2, KEYM_CTRL | KEYV_2,			"file/recent/2"},
	{IDM_FILE_RECENT_3, KEYM_CTRL | KEYV_3,			"file/recent/3"},
	{IDM_FILE_RECENT_4, KEYM_CTRL | KEYV_4,			"file/recent/4"},
	{IDM_FILE_RECENT_5, KEYM_CTRL | KEYV_5,			"file/recent/5"},
	{IDM_FILE_RECENT_6, KEYM_CTRL | KEYV_6,			"file/recent/6"},
	{IDM_FILE_RECENT_7, KEYM_CTRL | KEYV_7,			"file/recent/7"},
	{IDM_FILE_RECENT_8, KEYM_CTRL | KEYV_8,			"file/recent/8"},
	{IDM_FILE_RECENT_9, KEYM_CTRL | KEYV_9,			"file/recent/9"},
	// File menu.
	{IDM_FILE_CLOSE, KEYM_CTRL | KEYV_w,			"file/close"},
	{IDM_FILE_SAVESTATE, KEYV_F5,				"file/saveState"},
	{IDM_FILE_LOADSTATE, KEYV_F8,				"file/loadState"},
#ifdef Q_WS_MAC
	{IDM_FILE_GENCONFIG, KEYM_CTRL | KEYV_COMMA,		"file/genConfig"},
#else
	{IDM_FILE_GENCONFIG, 0,					"file/genConfig"},
#endif
	{IDM_FILE_MCDCONTROL, 0,				"file/mcdControl"},
	{IDM_FILE_QUIT, KEYM_CTRL | KEYV_q,			"file/quit"},

	// Graphics menu.
#ifndef Q_WS_MAC
	{IDM_GRAPHICS_MENUBAR, KEYM_CTRL | KEYV_m,		"graphics/showMenuBar"},
#endif /* !Q_WS_MAC */
	{IDM_GRAPHICS_RES, 0,					"graphics/resolution"},
	// Graphics, Resolution submenu.
	{IDM_GRAPHICS_RES_1X, 0,				"graphics/resolution/1x"},
	{IDM_GRAPHICS_RES_2X, 0,				"graphics/resolution/2x"},
	{IDM_GRAPHICS_RES_3X, 0,				"graphics/resolution/3x"},
	{IDM_GRAPHICS_RES_4X, 0,				"graphics/resolution/4x"},
	// Graphics menu.
	{IDM_GRAPHICS_BPP, 0,					"graphics/bpp"},
	// Graphics, Color Depth submenu.
	{IDM_GRAPHICS_BPP_15, 0,				"graphics/bpp/15"},
	{IDM_GRAPHICS_BPP_16, 0,				"graphics/bpp/16"},
	{IDM_GRAPHICS_BPP_32, 0,				"graphics/bpp/32"},
	// Graphics menu.
	{IDM_GRAPHICS_STRETCH, KEYM_SHIFT | KEYV_F2,		"graphics/stretch"},
	// Graphics, Stretch submenu.
	{IDM_GRAPHICS_STRETCH_NONE, 0,				"graphics/stretch/none"},
	{IDM_GRAPHICS_STRETCH_H, 0,				"graphics/stretch/horizontal"},
	{IDM_GRAPHICS_STRETCH_V, 0,				"graphics/stretch/vertical"},
	{IDM_GRAPHICS_STRETCH_FULL, 0,				"graphics/stretch/full"},
	// Graphics menu.
	{IDM_GRAPHICS_SCRSHOT, KEYM_SHIFT | KEYV_BACKSPACE,	"graphics/screenShot"},

	// System menu.
	{IDM_SYSTEM_REGION, KEYM_SHIFT | KEYV_F3,		"system/region"},
	// System, Region submenu.
	{IDM_SYSTEM_REGION_AUTODETECT, 0,			"system/region/autoDetect"},
	{IDM_SYSTEM_REGION_JAPAN, 0,				"system/region/japan"},
	{IDM_SYSTEM_REGION_ASIA, 0,				"system/region/asia"},
	{IDM_SYSTEM_REGION_USA, 0,				"system/region/usa"},
	{IDM_SYSTEM_REGION_EUROPE, 0,				"system/region/europe"},
	// System menu.
	{IDM_SYSTEM_HARDRESET, KEYM_SHIFT | KEYV_TAB,		"system/hardReset"},
	{IDM_SYSTEM_SOFTRESET, KEYV_TAB,			"system/softReset"},
	{IDM_SYSTEM_PAUSE, KEYV_ESCAPE,				"system/pause"},
	{IDM_SYSTEM_CPURESET_M68K, 0,				"system/resetM68K"},
	{IDM_SYSTEM_CPURESET_S68K, 0,				"system/resetS68K"},
	{IDM_SYSTEM_CPURESET_MSH2, 0,				"system/resetMSH2"},
	{IDM_SYSTEM_CPURESET_SSH2, 0,				"system/resetSSH2"},
	{IDM_SYSTEM_CPURESET_Z80, 0,				"system/resetZ80"},

	// Options menu.
	{IDM_OPTIONS_ENABLESRAM, 0,				"options/enableSRam"},
	{IDM_OPTIONS_CONTROLLERS, 0,				"options/controllers"},

	// NOTE: Test menus aren't going to be added here.

	// Help menu.
	{IDM_HELP_ABOUT, 0,					"help/about"},

	// Non-menu keys.
	{IDM_NOMENU_FASTBLUR, KEYV_F9,				"other/fastBlur"},

	{IDM_NOMENU_SAVESLOT_0, KEYV_0,				"other/saveSlot0"},
	{IDM_NOMENU_SAVESLOT_1, KEYV_1,				"other/saveSlot1"},
	{IDM_NOMENU_SAVESLOT_2, KEYV_2,				"other/saveSlot2"},
	{IDM_NOMENU_SAVESLOT_3, KEYV_3,				"other/saveSlot3"},
	{IDM_NOMENU_SAVESLOT_4, KEYV_4,				"other/saveSlot4"},
	{IDM_NOMENU_SAVESLOT_5, KEYV_5,				"other/saveSlot5"},
	{IDM_NOMENU_SAVESLOT_6, KEYV_6,				"other/saveSlot6"},
	{IDM_NOMENU_SAVESLOT_7, KEYV_7,				"other/saveSlot7"},
	{IDM_NOMENU_SAVESLOT_8, KEYV_8,				"other/saveSlot8"},
	{IDM_NOMENU_SAVESLOT_9, KEYV_9,				"other/saveSlot9"},
	{IDM_NOMENU_SAVESLOT_PREV, KEYV_F6,			"other/saveSlotPrev"},
	{IDM_NOMENU_SAVESLOT_NEXT, KEYV_F7,			"other/saveSlotNext"},
	{IDM_NOMENU_SAVESLOT_LOADFROM, KEYV_F8 | KEYM_SHIFT,	"other/saveLoadFrom"},
	{IDM_NOMENU_SAVESLOT_SAVEAS, KEYV_F5 | KEYM_SHIFT,	"other/saveSaveAs"},

	// End of default keys.
	{0, 0, nullptr}
};


GensKeyConfig::GensKeyConfig(QObject *parent)
	: QObject(parent)
	, d(new GensKeyConfigPrivate())
{
	// Load the default key configuration.
	for (const GensKeyConfigPrivate::DefKeySetting_t *key = &d->DefKeySettings[0];
	    key->action != 0; key++)
	{
		d->hashActionToKey.insert(key->action, key->gensKey);
		if (key->gensKey != KEYV_UNKNOWN)
			d->hashKeyToAction.insert(key->gensKey, key->action);
	}
}

GensKeyConfig::~GensKeyConfig()
{
	delete d;
}


/**
 * keyToAction(): Look up an action based on a GensKey_t value.
 * @param key GensKey_t value. (WITH MODIFIERS)
 * @return Action, or 0 if no action was found.
 */
int GensKeyConfig::keyToAction(GensKey_t key) const
{
	return d->hashKeyToAction.value(key, 0);
}


/**
 * actionToKey(): Look up a GensKey_t based on an action value.
 * @param action Action value.
 * @return GensKey_t (WITH MODIFIERS), or 0 if no key was found.
 */
int GensKeyConfig::actionToKey(int action) const
{
	return d->hashActionToKey.value(action, 0);
}


/**
 * Load key configuration from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return 0 on success; non-zero on error.
 */
int GensKeyConfig::load(const QSettings *qSettings)
{
	// Clear the hash tables before loading.
	d->hashActionToKey.clear();
	d->hashKeyToAction.clear();

	// Load the key configuration.
	for (const GensKeyConfigPrivate::DefKeySetting_t *key = &d->DefKeySettings[0];
	    key->action != 0; key++)
	{
		const GensKey_t gensKey = qSettings->value(
			QLatin1String(key->setting), key->gensKey).toString().toUInt(nullptr, 0);
		d->hashActionToKey.insert(key->action, gensKey);
		if (key->gensKey != KEYV_UNKNOWN)
			d->hashKeyToAction.insert(gensKey, key->action);
	}

	// Key configuration loaded.
	return 0;
}


/**
 * Save key configuration to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param qSettings Settings file.
 * @return 0 on success; non-zero on error.
 */
int GensKeyConfig::save(QSettings *qSettings) const
{
	// Save the key configuration.
	for (const GensKeyConfigPrivate::DefKeySetting_t *key = &d->DefKeySettings[0];
	    key->action != 0; key++)
	{
		const GensKey_t gensKey = d->hashActionToKey.value(key->action, 0);
		QString gensKey_str = QLatin1String("0x") +
				QString::number(gensKey, 16).toUpper().rightJustified(4, QChar(L'0'));

		qSettings->setValue(QLatin1String(key->setting), gensKey_str);
	}
	
	// Key configuration saved.
	return 0;
}

}
