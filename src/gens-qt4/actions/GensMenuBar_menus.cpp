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

namespace GensQt4
{

/** File menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiFile[] =
{
	{IDM_FILE_OPEN, GMI_NORMAL, "&Open ROM...", 0, NULL, "document-open"},
	{IDM_FILE_CLOSE, GMI_NORMAL, "&Close ROM", 0, NULL, "document-close"},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
	{IDM_FILE_SAVESTATE, GMI_NORMAL, "&Save State", 0, NULL, NULL},
	{IDM_FILE_LOADSTATE, GMI_NORMAL, "&Load State", 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
#ifdef Q_WS_MAC
	// Qt/Mac moves the "Preferences" item to the Application menu.
	{IDM_FILE_GENCONFIG, GMI_NORMAL, "&Preferences...", 0, NULL, "configure"},
#else
	{IDM_FILE_GENCONFIG, GMI_NORMAL, "&General Configuration", 0, NULL, "configure"},
#endif
	{IDM_FILE_MCDCONTROL, GMI_NORMAL, "Sega C&D Control Panel", 0, NULL, "media-optical"},
	
	// Qt/Mac moves the "Quit" item to the Application menu.
#ifndef Q_WS_MAC
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
#endif
	{IDM_FILE_QUIT, GMI_NORMAL, "&Quit", 0, NULL, "application-exit"},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** Graphics menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphics[] =
{
#ifndef Q_WS_MAC
	{IDM_GRAPHICS_MENUBAR, GMI_CHECK, "Show &Menu Bar", IDM_GRAPHICS_MENUBAR, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
#endif /* !Q_WS_MAC */
	{IDM_GRAPHICS_RES, GMI_SUBMENU, "&Resolution", IDM_GRAPHICS_RES_MENU, &ms_gmiGraphicsRes[0], NULL},
	{IDM_GRAPHICS_BPP, GMI_SUBMENU, "&Color Depth", IDM_GRAPHICS_BPP_MENU, &ms_gmiGraphicsBpp[0], NULL},
	{IDM_GRAPHICS_STRETCH, GMI_SUBMENU, "S&tretch Mode", IDM_GRAPHICS_STRETCH_MENU,
		&ms_gmiGraphicsStretch[0], NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
	{IDM_GRAPHICS_SCRSHOT, GMI_NORMAL, "&Screenshot", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** Graphics, Resolution submenu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphicsRes[] =
{
	{IDM_GRAPHICS_RES_1X, GMI_RADIO, "320x240 (&1x)", 0, NULL, NULL},
	{IDM_GRAPHICS_RES_2X, GMI_RADIO, "640x480 (&2x)", 0, NULL, NULL},
	{IDM_GRAPHICS_RES_3X, GMI_RADIO, "960x720 (&3x)", 0, NULL, NULL},
	{IDM_GRAPHICS_RES_4X, GMI_RADIO, "1280x960 (&41x)", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** Graphics, Color Depth submenu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphicsBpp[] =
{
	{IDM_GRAPHICS_BPP_15, GMI_RADIO, "15-bit (555)", 0, NULL, NULL},
	{IDM_GRAPHICS_BPP_16, GMI_RADIO, "16-bit (565)", 0, NULL, NULL},
	{IDM_GRAPHICS_BPP_32, GMI_RADIO, "32-bit (888)", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** Graphics, Stretch Mode submenu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphicsStretch[] =
{
	{IDM_GRAPHICS_STRETCH_NONE, GMI_RADIO, "None", 0, NULL, NULL},
	{IDM_GRAPHICS_STRETCH_H, GMI_RADIO, "Horizontal Only", 0, NULL, NULL},
	{IDM_GRAPHICS_STRETCH_V, GMI_RADIO, "Vertical Only", 0, NULL, NULL},
	{IDM_GRAPHICS_STRETCH_FULL, GMI_RADIO, "Full Stretch", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** System menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiSystem[] =
{
	{IDM_SYSTEM_REGION, GMI_SUBMENU, "&Region", IDM_SYSTEM_REGION_MENU, &ms_gmiSystemRegion[0], NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
	{IDM_SYSTEM_HARDRESET, GMI_NORMAL, "&Hard Reset", 0, NULL, NULL},
	{IDM_SYSTEM_SOFTRESET, GMI_NORMAL, "&Soft Reset", 0, NULL, NULL},
	{IDM_SYSTEM_PAUSE, GMI_CHECK, "&Pause", 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
	{IDM_SYSTEM_CPURESET_M68K, GMI_NORMAL, "Reset &68000", 0, NULL, NULL},
	{IDM_SYSTEM_CPURESET_Z80, GMI_NORMAL, "Reset &Z80", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** System, Region submenu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiSystemRegion[] =
{
	{IDM_SYSTEM_REGION_AUTODETECT, GMI_RADIO, "&Auto Detect", 0, NULL, NULL},
	{IDM_SYSTEM_REGION_JAPAN, GMI_RADIO, "&Japan (NTSC)", 0, NULL, NULL},
	{IDM_SYSTEM_REGION_ASIA, GMI_RADIO, "A&sia (PAL)", 0, NULL, NULL},
	{IDM_SYSTEM_REGION_USA, GMI_RADIO, "&USA (NTSC)", 0, NULL, NULL},
	{IDM_SYSTEM_REGION_EUROPE, GMI_RADIO, "&Europe (PAL)", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** CtrlTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiCtrlTest[] =
{
	{IDM_CTRLTEST_NONE, GMI_NORMAL, "&None", 0, NULL, NULL},
	{IDM_CTRLTEST_3BT, GMI_NORMAL, "&3-button", 0, NULL, NULL},
	{IDM_CTRLTEST_6BT, GMI_NORMAL, "&6-button", 0, NULL, NULL},
	{IDM_CTRLTEST_2BT, GMI_NORMAL, "&2-button", 0, NULL, NULL},
	{IDM_CTRLTEST_MEGAMOUSE, GMI_NORMAL, "&Mega Mouse", 0, NULL, NULL},
	{IDM_CTRLTEST_TEAMPLAYER, GMI_NORMAL, "&Teamplayer", 0, NULL, NULL},
	{IDM_CTRLTEST_4WP, GMI_NORMAL, "EA &4-Way Play", 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
	{IDM_CTRLTEST_CONFIG, GMI_NORMAL, "&CtrlConfig...", 0, NULL, "input-gaming"},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** SoundTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiSoundTest[] =
{
	{IDM_SOUNDTEST_11025, GMI_NORMAL, "&11,025 Hz", 0, NULL, NULL},
	{IDM_SOUNDTEST_16000, GMI_NORMAL, "1&6,000 Hz", 0, NULL, NULL},
	{IDM_SOUNDTEST_22050, GMI_NORMAL, "&22,050 Hz", 0, NULL, NULL},
	{IDM_SOUNDTEST_32000, GMI_NORMAL, "&32,000 Hz", 0, NULL, NULL},
	{IDM_SOUNDTEST_44100, GMI_NORMAL, "&44,100 Hz", 0, NULL, NULL},
	{IDM_SOUNDTEST_48000, GMI_NORMAL, "4&8,000 Hz", 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, NULL},
	{IDM_SOUNDTEST_MONO, GMI_NORMAL, "Mono", 0, NULL, NULL},
	{IDM_SOUNDTEST_STEREO, GMI_NORMAL, "Stereo", 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** Help menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiHelp[] =
{
	{IDM_HELP_ABOUT, GMI_NORMAL, "&About Gens/GS II", 0, NULL, "help-about"},
	
	{0, GMI_NORMAL, NULL, 0, NULL, NULL}
};

/** Main menu. **/
const GensMenuBar::MainMenuItem GensMenuBar::ms_gmmiMain[] =
{
	{IDM_FILE_MENU, "&File", &ms_gmiFile[0]},
	{IDM_GRAPHICS_MENU, "&Graphics", &ms_gmiGraphics[0]},
	{IDM_SYSTEM_MENU, "&System", &ms_gmiSystem[0]},
	{IDM_CTRLTEST_MENU, "&CtrlTest", &ms_gmiCtrlTest[0]},
	{IDM_SOUNDTEST_MENU, "&SoundTest", &ms_gmiSoundTest[0]},
	{IDM_HELP_MENU, "&Help", &ms_gmiHelp[0]},
	
	{0, NULL, NULL}
};

}
