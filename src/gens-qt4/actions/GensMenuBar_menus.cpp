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
#include <QtCore/qnamespace.h>

namespace GensQt4
{

/** File menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiFile[] =
{
	{IDM_FILE_OPEN, GMI_NORMAL, "&Open ROM...", 0, NULL, MACCEL_OPEN, Qt::CTRL + Qt::Key_O, "document-open"},
	{IDM_FILE_CLOSE, GMI_NORMAL, "&Close ROM", 0, NULL, MACCEL_CLOSE, Qt::CTRL + Qt::Key_W, "document-close"},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_FILE_SAVESTATE, GMI_NORMAL, "&Save State", 0, NULL, MACCEL_NONE, Qt::Key_F5, NULL},
	{IDM_FILE_LOADSTATE, GMI_NORMAL, "&Load State", 0, NULL, MACCEL_NONE, Qt::Key_F8, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_FILE_GENCONFIG, GMI_NORMAL, "&General Configuration", 0, NULL, MACCEL_PREFERENCES, 0, "configure"},
	{IDM_FILE_MCDCONTROL, GMI_NORMAL, "Sega C&D Control Panel", 0, NULL, MACCEL_NONE, 0, "media-optical"},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_FILE_QUIT, GMI_NORMAL, "&Quit", 0, NULL, MACCEL_QUIT, Qt::CTRL + Qt::Key_Q, "application-exit"},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** Graphics menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphics[] =
{
	{IDM_GRAPHICS_RES, GMI_SUBMENU, "&Resolution", IDM_GRAPHICS_RES_MENU, &ms_gmiGraphicsRes[0], MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_BPP, GMI_SUBMENU, "&Color Depth", IDM_GRAPHICS_BPP_MENU, &ms_gmiGraphicsBpp[0], MACCEL_NONE, 0, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_SCRSHOT, GMI_NORMAL, "&Screenshot", 0, NULL, MACCEL_NONE, Qt::SHIFT + Qt::Key_Backspace, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** Graphics, Resolution submenu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphicsRes[] =
{
	{IDM_GRAPHICS_RES_1X, GMI_RADIO, "320x240 (&1x)", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_RES_2X, GMI_RADIO, "640x480 (&2x)", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_RES_3X, GMI_RADIO, "960x720 (&3x)", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_RES_4X, GMI_RADIO, "1280x960 (&41x)", 0, NULL, MACCEL_NONE, 0, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** Graphics, Color Depth submenu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiGraphicsBpp[] =
{
	{IDM_GRAPHICS_BPP_15, GMI_RADIO, "15-bit (555)", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_BPP_16, GMI_RADIO, "16-bit (565)", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_GRAPHICS_BPP_32, GMI_RADIO, "32-bit (888)", 0, NULL, MACCEL_NONE, 0, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** CtrlTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiCtrlTest[] =
{
	{IDM_CTRLTEST_NONE, GMI_NORMAL, "&None", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_3BT, GMI_NORMAL, "&3-button", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_6BT, GMI_NORMAL, "&6-button", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_2BT, GMI_NORMAL, "&2-button", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_MEGAMOUSE, GMI_NORMAL, "&Mega Mouse", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_TEAMPLAYER, GMI_NORMAL, "&Teamplayer", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_4WP, GMI_NORMAL, "EA &4-Way Play", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_CTRLTEST_CONFIG, GMI_NORMAL, "&Configure...", 0, NULL, MACCEL_NONE, 0, "input-gaming"},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** SoundTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiSoundTest[] =
{
	{IDM_SOUNDTEST_11025, GMI_NORMAL, "&11,025 Hz", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_16000, GMI_NORMAL, "1&6,000 Hz", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_22050, GMI_NORMAL, "&22,050 Hz", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_32000, GMI_NORMAL, "&32,000 Hz", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_44100, GMI_NORMAL, "&44,100 Hz", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_48000, GMI_NORMAL, "4&8,000 Hz", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_MONO, GMI_NORMAL, "Mono", 0, NULL, MACCEL_NONE, 0, NULL},
	{IDM_SOUNDTEST_STEREO, GMI_NORMAL, "Stereo", 0, NULL, MACCEL_NONE, 0, NULL},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** Help menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiHelp[] =
{
	{IDM_HELP_ABOUT, GMI_NORMAL, "&About Gens/GS II", 0, NULL, MACCEL_NONE, 0, "help-about"},
	
	{0, GMI_NORMAL, NULL, 0, NULL, MACCEL_NONE, 0, NULL}
};

/** Main menu. **/
const GensMenuBar::MainMenuItem GensMenuBar::ms_gmmiMain[] =
{
	{IDM_FILE_MENU, "&File", &ms_gmiFile[0]},
	{IDM_GRAPHICS_MENU, "&Graphics", &ms_gmiGraphics[0]},
	{IDM_CTRLTEST_MENU, "&CtrlTest", &ms_gmiCtrlTest[0]},
	{IDM_SOUNDTEST_MENU, "&SoundTest", &ms_gmiSoundTest[0]},
	{IDM_HELP_MENU, "&Help", &ms_gmiHelp[0]},
	
	{0, NULL, NULL}
};

}
