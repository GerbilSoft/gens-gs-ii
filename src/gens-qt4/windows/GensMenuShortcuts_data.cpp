/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensMenuShortcuts_data.cpp: Default key bindings data.                     *
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

#include "GensMenuShortcuts_p.hpp"

namespace GensQt4 {

// Default key settings.
const GensMenuShortcutsPrivate::DefKeySetting_t GensMenuShortcutsPrivate::DefKeySettings[] =
{
	// File menu.
	{"file/open",			"actionFileOpenROM",			KEYM_CTRL | KEYV_o},
	{"file/recent",			"actionFileRecentROMs",			0},
	// File, Recent ROMs menu.
	// NOTE: Recent ROMs menu is dynamic.
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
	{"graphics/stretch",		"actionGraphicsStretch",		KEYM_SHIFT | KEYV_F2},
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

}
