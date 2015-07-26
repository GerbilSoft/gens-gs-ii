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

// Map of QSettings entries to QActions.
const GensMenuShortcutsPrivate::KeyBinding_t GensMenuShortcutsPrivate::KeyBindings[KeyBinding_count+1] =
{
	// File menu.
	{"file/open",			"actionFileOpenROM"},
	{"file/recent",			"actionFileRecentROMs"},
	// File, Recent ROMs menu.
	// NOTE: Recent ROMs menu is dynamic.
	// The menu items will have these object names.
	{"file/recent/1",		"actionRecentROMs_1"},
	{"file/recent/2",		"actionRecentROMs_2"},
	{"file/recent/3",		"actionRecentROMs_3"},
	{"file/recent/4",		"actionRecentROMs_4"},
	{"file/recent/5",		"actionRecentROMs_5"},
	{"file/recent/6",		"actionRecentROMs_6"},
	{"file/recent/7",		"actionRecentROMs_7"},
	{"file/recent/8",		"actionRecentROMs_8"},
	{"file/recent/9",		"actionRecentROMs_9"},
	// File menu.
	{"file/close",			"actionFileCloseROM"},
	{"file/saveState",		"actionFileSaveState"},
	{"file/loadState",		"actionFileLoadState"},
	{"file/genConfig",		"actionFileGeneralConfiguration"},
	{"file/mcdControl",		"actionFileSegaCDControlPanel"},
	{"file/quit",			"actionFileQuit"},

	// Graphics menu.
	{"graphics/showMenuBar",	"actionGraphicsShowMenuBar"},
	// TODO: Fullscreen.
	//{"graphics/fullScreen",	"actionGraphicsFullScreen"},
	{"graphics/resolution",		"actionGraphicsResolution"},
	// Graphics, Resolution submenu.
	{"graphics/resolution/1x",	"actionGraphicsResolution1x"},
	{"graphics/resolution/2x",	"actionGraphicsResolution2x"},
	{"graphics/resolution/3x",	"actionGraphicsResolution3x"},
	{"graphics/resolution/4x",	"actionGraphicsResolution4x"},
	// Graphics menu.
	{"graphics/bpp",		"actionGraphicsBpp"},
	// Graphics, Color Depth submenu.
	{"graphics/bpp/15",		"actionGraphicsBpp15"},
	{"graphics/bpp/16",		"actionGraphicsBpp16"},
	{"graphics/bpp/32",		"actionGraphicsBpp32"},
	// Graphics menu.
	{"graphics/stretch",		"actionGraphicsStretch"},
	// Graphics, Stretch submenu.
	{"graphics/stretch/none",	"actionGraphicsStretchNone"},
	{"graphics/stretch/horizontal",	"actionGraphicsStretchHorizontal"},
	{"graphics/stretch/vertical",	"actionGraphicsStretchVertical"},
	{"graphics/stretch/full",	"actionGraphicsStretchFull"},
	// Graphics menu.
	{"graphics/screenShot",		"actionGraphicsScreenshot"},

	// System menu.
	{"system/region",		"actionSystemRegion"},
	// System, Region submenu.
	{"system/region/autoDetect",	"actionSystemRegionAuto"},
	{"system/region/japan",		"actionSystemRegionJPN"},
	{"system/region/asia",		"actionSystemRegionAsia"},
	{"system/region/usa",		"actionSystemRegionUSA"},
	{"system/region/europe",	"actionSystemRegionEUR"},
	// System menu.
	{"system/hardReset",		"actionSystemHardReset"},
	{"system/softReset",		"actionSystemSoftReset"},
	{"system/pause",		"actionSystemPause"},
	{"system/resetM68K",		"actionSystemResetM68K"},
	{"system/resetS68K",		"actionSystemResetS68K"},
	{"system/resetMSH2",		"actionSystemResetMSH2"},
	{"system/resetSSH2",		"actionSystemResetSSH2"},
	{"system/resetZ80",		"actionSystemResetZ80"},

	// Options menu.
	{"options/enableSRam",		"actionOptionsSRAM"},
	{"options/controllers",		"actionOptionsControllers"},

	// NOTE: Test menus aren't going to be added here.

	// Help menu.
	{"help/about",			"actionHelpAbout"},

	// Non-menu keys.
	{"other/fastBlur",		"actionNoMenuFastBlur"},

	// Savestates.
	// TODO: Change to saveSlot/0?
	{"other/saveSlot0",		"actionNoMenuSaveSlot0"},
	{"other/saveSlot1",		"actionNoMenuSaveSlot1"},
	{"other/saveSlot2",		"actionNoMenuSaveSlot2"},
	{"other/saveSlot3",		"actionNoMenuSaveSlot3"},
	{"other/saveSlot4",		"actionNoMenuSaveSlot4"},
	{"other/saveSlot5",		"actionNoMenuSaveSlot5"},
	{"other/saveSlot6",		"actionNoMenuSaveSlot6"},
	{"other/saveSlot7",		"actionNoMenuSaveSlot7"},
	{"other/saveSlot8",		"actionNoMenuSaveSlot8"},
	{"other/saveSlot9",		"actionNoMenuSaveSlot9"},
	{"other/saveSlotPrev",		"actionNoMenuSaveSlotPrev"},
	{"other/saveSlotNext",		"actionNoMenuSaveSlotNext"},
	// TODO: Swap these two?
	{"other/saveSlotPrev",		"actionNoMenuLoadStateFrom"},
	{"other/saveSlotNext",		"actionNoMenuSaveStateAs"},

	// End of key bindings.
	{nullptr, nullptr}
};

/**
 * Default key bindings for Gens/GS II.
 */
const GensKey_t GensMenuShortcutsPrivate::DefKeyBindings_gens[KeyBinding_count+1] = {
	// File menu.
	KEYM_CTRL | KEYV_o,		// actionFileOpenROM
	0,				// actionFileRecentROMs
	// File, Recent ROMs menu.
	// NOTE: Recent ROMs menu is dynamic.
	// The menu items will have these object names.
	KEYM_CTRL | KEYV_1,		// actionRecentROMs_1
	KEYM_CTRL | KEYV_2,		// actionRecentROMs_2
	KEYM_CTRL | KEYV_3,		// actionRecentROMs_3
	KEYM_CTRL | KEYV_4,		// actionRecentROMs_4
	KEYM_CTRL | KEYV_5,		// actionRecentROMs_5
	KEYM_CTRL | KEYV_6,		// actionRecentROMs_6
	KEYM_CTRL | KEYV_7,		// actionRecentROMs_7
	KEYM_CTRL | KEYV_8,		// actionRecentROMs_8
	KEYM_CTRL | KEYV_9,		// actionRecentROMs_9
	// File menu.
	KEYM_CTRL | KEYV_w,		// actionFileCloseROM
	KEYV_F5,			// actionFileSaveState
	KEYV_F8,			// actionFileLoadState
#ifdef Q_WS_MAC
	KEYM_CTRL | KEYV_COMMA,		// actionFileGeneralConfiguration
#else
	0,				// actionFileGeneralConfiguration
#endif
	0,				// actionFileSegaCDControlPanel
	KEYM_CTRL | KEYV_q,		// actionFileQuit

	// Graphics menu.
	KEYM_CTRL | KEYV_m,		// actionGraphicsShowMenuBar
	//KEYM_ALT | KEYV_RETURN,	// TODO: Fullscreen.
	0,				// actionGraphicsResolution
	// Graphics, Resolution submenu.
	0,				// actionGraphicsResolution1x
	0,				// actionGraphicsResolution2x
	0,				// actionGraphicsResolution3x
	0,				// actionGraphicsResolution4x
	// Graphics menu.
	0,				// actionGraphicsBpp
	// Graphics, Color Depth submenu.
	0,				// actionGraphicsBpp15
	0,				// actionGraphicsBpp16
	0,				// actionGraphicsBpp32
	// Graphics menu.
	KEYM_SHIFT | KEYV_F2,		// actionGraphicsStretch
	// Graphics, Stretch submenu.
	0,				// actionGraphicsStretchNone
	0,				// actionGraphicsStretchHorizontal
	0,				// actionGraphicsStretchVertical
	0,				// actionGraphicsStretchFull
	// Graphics menu.
	KEYM_SHIFT | KEYV_BACKSPACE,	// actionGraphicsScreenshot

	// System menu.
	KEYM_SHIFT | KEYV_F3,		// actionSystemRegion
	// System, Region submenu.
	0,				// actionSystemRegionAuto
	0,				// actionSystemRegionJPN
	0,				// actionSystemRegionAsia
	0,				// actionSystemRegionUSA
	0,				// actionSystemRegionEUR
	// System menu.
	KEYM_SHIFT | KEYV_TAB,		// actionSystemHardReset
	KEYV_TAB,			// actionSystemSoftReset
	KEYV_ESCAPE,			// actionSystemPause
	0,				// actionSystemResetM68K
	0,				// actionSystemResetS68K
	0,				// actionSystemResetMSH2
	0,				// actionSystemResetSSH2
	0,				// actionSystemResetZ80

	// Options menu.
	0,				// actionOptionsSRAM
	0,				// actionOptionsControllers

	// NOTE: Test menus aren't going to be added here.

	// Help menu.
	0,				// actionHelpAbout

	// Non-menu keys.
	KEYV_F9,			// actionNoMenuFastBlur

	// Savestates.
	KEYV_0,				// actionNoMenuSaveSlot0
	KEYV_1,				// actionNoMenuSaveSlot1
	KEYV_2,				// actionNoMenuSaveSlot2
	KEYV_3,				// actionNoMenuSaveSlot3
	KEYV_4,				// actionNoMenuSaveSlot4
	KEYV_5,				// actionNoMenuSaveSlot5
	KEYV_6,				// actionNoMenuSaveSlot6
	KEYV_7,				// actionNoMenuSaveSlot7
	KEYV_8,				// actionNoMenuSaveSlot8
	KEYV_9,				// actionNoMenuSaveSlot9
	KEYV_F6,			// actionNoMenuSaveSlotPrev
	KEYV_F7,			// actionNoMenuSaveSlotNext
	// TODO: Swap these two?
	KEYM_SHIFT | KEYV_F8,		// actionNoMenuLoadStateFrom
	KEYM_SHIFT | KEYV_F5,		// actionNoMenuSaveStateAs

	// End of key bindings.
	// TODO: Make this -1, or remove the last entry entirely?
	0
};

/**
 * Default key bindings for Kega Fusion 3.63x.
 */
const GensKey_t GensMenuShortcutsPrivate::DefKeyBindings_kega[KeyBinding_count+1] = {
	// File menu.
	// NOTE: Kega Fusion has separate menu items for
	// different systems. Gens/GS II does not, so use
	// the standard "Open" key shortcut.
	KEYM_CTRL | KEYV_o,		// actionFileOpenROM
	0,				// actionFileRecentROMs
	// File, Recent ROMs menu.
	// Kega Fusion only has a hotkey for the "last" ROM.
	KEYM_SHIFT | KEYM_CTRL | KEYV_l,	// actionRecentROMs_1
	0,		// actionRecentROMs_2
	0,		// actionRecentROMs_3
	0,		// actionRecentROMs_4
	0,		// actionRecentROMs_5
	0,		// actionRecentROMs_6
	0,		// actionRecentROMs_7
	0,		// actionRecentROMs_8
	0,		// actionRecentROMs_9
	// File menu.
	// Instead of "Close ROM", Kega Fusion has "Power Off".
	// The last ROM will be reloaded if "Hard Reset" is selected.
	// Gens/GS II doesn't support this, so use the "Power Off"
	// hotkey for "Close ROM".
	KEYM_SHIFT | KEYV_TAB,		// actionFileCloseROM
	KEYV_F5,			// actionFileSaveState
	KEYV_F8,			// actionFileLoadState
#ifdef Q_WS_MAC
	KEYM_CTRL | KEYV_COMMA,		// actionFileGeneralConfiguration
#else
	0,				// actionFileGeneralConfiguration
#endif
	0,				// actionFileSegaCDControlPanel
	KEYM_CTRL | KEYV_q,		// actionFileQuit

	// Graphics menu.
	KEYM_CTRL | KEYV_m,		// actionGraphicsShowMenuBar
	// NOTE: Both Esc and Alt+Enter work here. Add secondary shortcuts?
	//KEYM_ALT | KEYV_RETURN,	// TODO: Fullscreen.
	0,				// actionGraphicsResolution
	// Graphics, Resolution submenu.
	0,				// actionGraphicsResolution1x
	0,				// actionGraphicsResolution2x
	0,				// actionGraphicsResolution3x
	0,				// actionGraphicsResolution4x
	// Graphics menu.
	0,				// actionGraphicsBpp
	// Graphics, Color Depth submenu.
	0,				// actionGraphicsBpp15
	0,				// actionGraphicsBpp16
	0,				// actionGraphicsBpp32
	// Graphics menu.
	KEYM_SHIFT | KEYV_F2,		// actionGraphicsStretch
	// Graphics, Stretch submenu.
	0,				// actionGraphicsStretchNone
	0,				// actionGraphicsStretchHorizontal
	0,				// actionGraphicsStretchVertical
	0,				// actionGraphicsStretchFull
	// Graphics menu.
	KEYM_SHIFT | KEYV_F12,		// actionGraphicsScreenshot

	// System menu.
	KEYM_SHIFT | KEYV_F3,		// actionSystemRegion
	// System, Region submenu.
	0,				// actionSystemRegionAuto
	0,				// actionSystemRegionJPN
	0,				// actionSystemRegionAsia
	0,				// actionSystemRegionUSA
	0,				// actionSystemRegionEUR
	// System menu.
	KEYM_SHIFT | KEYV_TAB,		// actionSystemHardReset
	KEYV_TAB,			// actionSystemSoftReset
	KEYV_PAUSE,			// actionSystemPause
	0,				// actionSystemResetM68K
	0,				// actionSystemResetS68K
	0,				// actionSystemResetMSH2
	0,				// actionSystemResetSSH2
	0,				// actionSystemResetZ80

	// Options menu.
	0,				// actionOptionsSRAM
	0,				// actionOptionsControllers

	// NOTE: Test menus aren't going to be added here.

	// Help menu.
	0,				// actionHelpAbout

	// Non-menu keys.
	KEYV_F9,			// actionNoMenuFastBlur

	// Savestates.
	// NOTE: Kega doesn't map keys 0-9, but we'll
	// keep the mapping for convenience reasons.
	KEYV_0,				// actionNoMenuSaveSlot0
	KEYV_1,				// actionNoMenuSaveSlot1
	KEYV_2,				// actionNoMenuSaveSlot2
	KEYV_3,				// actionNoMenuSaveSlot3
	KEYV_4,				// actionNoMenuSaveSlot4
	KEYV_5,				// actionNoMenuSaveSlot5
	KEYV_6,				// actionNoMenuSaveSlot6
	KEYV_7,				// actionNoMenuSaveSlot7
	KEYV_8,				// actionNoMenuSaveSlot8
	KEYV_9,				// actionNoMenuSaveSlot9
	KEYV_F6,			// actionNoMenuSaveSlotPrev
	KEYV_F7,			// actionNoMenuSaveSlotNext
	// TODO: Swap these two?
	KEYM_SHIFT | KEYV_F8,		// actionNoMenuLoadStateFrom
	KEYM_SHIFT | KEYV_F5,		// actionNoMenuSaveStateAs

	// End of key bindings.
	// TODO: Make this -1, or remove the last entry entirely?
	0
};

}
