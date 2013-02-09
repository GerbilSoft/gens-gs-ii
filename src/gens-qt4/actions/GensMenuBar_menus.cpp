/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar_menus.cpp: Gens Menu Bar class: Menu definitions.           *
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

#include "GensMenuBar.hpp"
#include "GensMenuBar_menus.hpp"

// Qt includes.
#include <QtCore/qglobal.h>
#include <QtGui/QAction>

namespace GensQt4
{

/** File menu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiFile[] =
{
	{IDM_FILE_OPEN, GMI_NORMAL, QT_TR_NOOP("&Open ROM..."), QAction::NoRole, 0, nullptr, "document-open"},
	{IDM_FILE_RECENT, GMI_NORMAL, QT_TR_NOOP("&Recent ROMs"), QAction::NoRole, 0, nullptr, "document-open-recent"},
	{IDM_FILE_CLOSE, GMI_NORMAL, QT_TR_NOOP("&Close ROM"), QAction::NoRole, 0, nullptr, "document-close"},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
	{IDM_FILE_SAVESTATE, GMI_NORMAL, QT_TR_NOOP("&Save State"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_FILE_LOADSTATE, GMI_NORMAL, QT_TR_NOOP("&Load State"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
#ifdef Q_WS_MAC
	// Qt/Mac moves the "Preferences" item to the Application menu.
	{IDM_FILE_GENCONFIG, GMI_NORMAL, QT_TR_NOOP("&Preferences..."), QAction::PreferencesRole, 0, nullptr, "configure"},
#else
	{IDM_FILE_GENCONFIG, GMI_NORMAL, QT_TR_NOOP("&General Configuration"), QAction::PreferencesRole, 0, nullptr, "configure"},
#endif
	{IDM_FILE_MCDCONTROL, GMI_NORMAL, QT_TR_NOOP("Sega C&D Control Panel"), QAction::NoRole, 0, nullptr, "media-optical"},

	// Qt/Mac moves the "Quit" item to the Application menu.
#ifndef Q_WS_MAC
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
#endif
	{IDM_FILE_QUIT, GMI_NORMAL, QT_TR_NOOP("&Quit"), QAction::QuitRole, 0, nullptr, "application-exit"},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Graphics menu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiGraphics[] =
{
#ifndef Q_WS_MAC
	{IDM_GRAPHICS_MENUBAR, GMI_CHECK, QT_TR_NOOP("Show &Menu Bar"), QAction::NoRole, IDM_GRAPHICS_MENUBAR, nullptr, nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
#endif /* !Q_WS_MAC */
	{IDM_GRAPHICS_RES, GMI_SUBMENU, QT_TR_NOOP("&Resolution"), QAction::NoRole,
		IDM_GRAPHICS_RES_MENU, &gmiGraphicsRes[0], nullptr},
	{IDM_GRAPHICS_BPP, GMI_SUBMENU, QT_TR_NOOP("&Color Depth"), QAction::NoRole,
		IDM_GRAPHICS_BPP_MENU, &gmiGraphicsBpp[0], nullptr},
	{IDM_GRAPHICS_STRETCH, GMI_SUBMENU, QT_TR_NOOP("S&tretch Mode"), QAction::NoRole,
		IDM_GRAPHICS_STRETCH_MENU, &gmiGraphicsStretch[0], nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_SCRSHOT, GMI_NORMAL, QT_TR_NOOP("&Screenshot"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Graphics, Resolution submenu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiGraphicsRes[] =
{
	{IDM_GRAPHICS_RES_1X, GMI_RADIO, QT_TR_NOOP("320x240 (&1x)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_RES_2X, GMI_RADIO, QT_TR_NOOP("640x480 (&2x)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_RES_3X, GMI_RADIO, QT_TR_NOOP("960x720 (&3x)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_RES_4X, GMI_RADIO, QT_TR_NOOP("1280x960 (&4x)"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Graphics, Color Depth submenu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiGraphicsBpp[] =
{
	{IDM_GRAPHICS_BPP_15, GMI_RADIO, QT_TR_NOOP("15-bit (555)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_BPP_16, GMI_RADIO, QT_TR_NOOP("16-bit (565)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_BPP_32, GMI_RADIO, QT_TR_NOOP("32-bit (888)"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Graphics, Stretch Mode submenu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiGraphicsStretch[] =
{
	{IDM_GRAPHICS_STRETCH_NONE, GMI_RADIO, QT_TR_NOOP("None"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_STRETCH_H, GMI_RADIO, QT_TR_NOOP("Horizontal Only"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_STRETCH_V, GMI_RADIO, QT_TR_NOOP("Vertical Only"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_GRAPHICS_STRETCH_FULL, GMI_RADIO, QT_TR_NOOP("Full Stretch"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** System menu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiSystem[] =
{
	{IDM_SYSTEM_REGION, GMI_SUBMENU, QT_TR_NOOP("&Region"), QAction::NoRole,
		IDM_SYSTEM_REGION_MENU, &gmiSystemRegion[0], nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_HARDRESET, GMI_NORMAL, QT_TR_NOOP("&Hard Reset"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_SOFTRESET, GMI_NORMAL, QT_TR_NOOP("&Soft Reset"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_PAUSE, GMI_CHECK, QT_TR_NOOP("&Pause"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
	// TODO: "Main 68000" for Sega CD?
	{IDM_SYSTEM_CPURESET_M68K, GMI_NORMAL, QT_TR_NOOP("Reset &68000"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_CPURESET_S68K, GMI_NORMAL, QT_TR_NOOP("Reset Sub 68000"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_CPURESET_MSH2, GMI_NORMAL, QT_TR_NOOP("Reset Master SH2"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_CPURESET_SSH2, GMI_NORMAL, QT_TR_NOOP("Reset Slave SH2"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_CPURESET_Z80, GMI_NORMAL, QT_TR_NOOP("Reset &Z80"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** System, Region submenu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiSystemRegion[] =
{
	{IDM_SYSTEM_REGION_AUTODETECT, GMI_RADIO, QT_TR_NOOP("&Auto Detect"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_REGION_JAPAN, GMI_RADIO, QT_TR_NOOP("&Japan (NTSC)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_REGION_ASIA, GMI_RADIO, QT_TR_NOOP("A&sia (PAL)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_REGION_USA, GMI_RADIO, QT_TR_NOOP("&USA (NTSC)"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SYSTEM_REGION_EUROPE, GMI_RADIO, QT_TR_NOOP("&Europe (PAL)"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Options menu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiOptions[] =
{
	{IDM_OPTIONS_ENABLESRAM, GMI_CHECK, QT_TR_NOOP("Enable &SRAM/EEPROM"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
	{IDM_OPTIONS_CONTROLLERS, GMI_NORMAL, QT_TR_NOOP("&Controllers..."), QAction::NoRole, 0, nullptr, "input-gaming"},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** SoundTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiSoundTest[] =
{
	// NOTE: No translation for the frequency items right now.
	{IDM_SOUNDTEST_11025, GMI_NORMAL, "&11,025 Hz", QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_16000, GMI_NORMAL, "1&6,000 Hz", QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_22050, GMI_NORMAL, "&22,050 Hz", QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_32000, GMI_NORMAL, "&32,000 Hz", QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_44100, GMI_NORMAL, "&44,100 Hz", QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_48000, GMI_NORMAL, "4&8,000 Hz", QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SEPARATOR, GMI_SEPARATOR, nullptr, QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_MONO, GMI_NORMAL, QT_TR_NOOP("Mono"), QAction::NoRole, 0, nullptr, nullptr},
	{IDM_SOUNDTEST_STEREO, GMI_NORMAL, QT_TR_NOOP("Stereo"), QAction::NoRole, 0, nullptr, nullptr},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Help menu. **/
const GensMenuBar::MenuItem GensMenuBar::gmiHelp[] =
{
	{IDM_HELP_ABOUT, GMI_NORMAL, QT_TR_NOOP("&About Gens/GS II"), QAction::AboutRole, 0, nullptr, "help-about"},

	{0, GMI_NORMAL, nullptr, QAction::NoRole, 0, nullptr, nullptr}
};

/** Main menu. **/
const GensMenuBar::MainMenuItem GensMenuBar::gmmiMain[] =
{
	{IDM_FILE_MENU, QT_TR_NOOP("&File"), &gmiFile[0]},
	{IDM_GRAPHICS_MENU, QT_TR_NOOP("&Graphics"), &gmiGraphics[0]},
	{IDM_SYSTEM_MENU, QT_TR_NOOP("&System"), &gmiSystem[0]},
	{IDM_OPTIONS_MENU, QT_TR_NOOP("&Options"), &gmiOptions[0]},
	// NOTE: No translation for SoundTest - it will be changed later.
	{IDM_SOUNDTEST_MENU, "&SoundTest", &gmiSoundTest[0]},
	{IDM_HELP_MENU, QT_TR_NOOP("&Help"), &gmiHelp[0]},

	{0, nullptr, nullptr}
};

}

