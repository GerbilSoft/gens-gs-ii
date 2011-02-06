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
	{IDM_FILE_OPEN, GMI_NORMAL, "&Open ROM...", NULL, MACCEL_OPEN, Qt::CTRL + Qt::Key_O, "document-open", ":/oxygen-16x16/document-open.png"},
	{IDM_FILE_CLOSE, GMI_NORMAL, "&Close ROM", NULL, MACCEL_CLOSE, Qt::CTRL + Qt::Key_W, "document-close", ":/oxygen-16x16/document-close.png"},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_FILE_SAVESTATE, GMI_NORMAL, "&Save State", NULL, MACCEL_NONE, Qt::Key_F5, NULL, NULL},
	{IDM_FILE_LOADSTATE, GMI_NORMAL, "&Load State", NULL, MACCEL_NONE, Qt::Key_F8, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_FILE_GENCONFIG, GMI_NORMAL, "&General Configuration", NULL, MACCEL_PREFERENCES, 0, "configure", ":/oxygen-16x16/configure.png"},
	{IDM_FILE_MCDCONTROL, GMI_NORMAL, "Sega C&D Control Panel", NULL, MACCEL_NONE, 0, "media-optical", ":/oxygen-16x16/media-optical.png"},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_FILE_QUIT, GMI_NORMAL, "&Quit", NULL, MACCEL_QUIT, Qt::CTRL + Qt::Key_Q, "application-exit", ":/oxygen-16x16/application-exit.png"},
	
	{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
};

/** ResBppTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiResBppTest[] =
{
	{IDM_RESBPPTEST_1X, GMI_NORMAL, "320x240 (&1x)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_2X, GMI_NORMAL, "640x480 (&2x)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_3X, GMI_NORMAL, "960x720 (&3x)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_4X, GMI_NORMAL, "1280x960 (&4x)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_15, GMI_NORMAL, "15-bit (555)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_16, GMI_NORMAL, "16-bit (565)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_32, GMI_NORMAL, "32-bit (888)", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_RESBPPTEST_SCRSHOT, GMI_NORMAL, "&Screenshot", NULL, MACCEL_NONE, Qt::SHIFT + Qt::Key_Backspace, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
};

/** CtrlTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiCtrlTest[] =
{
	{IDM_CTRLTEST_NONE, GMI_NORMAL, "&None", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_3BT, GMI_NORMAL, "&3-button", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_6BT, GMI_NORMAL, "&6-button", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_2BT, GMI_NORMAL, "&2-button", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_MEGAMOUSE, GMI_NORMAL, "&Mega Mouse", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_TEAMPLAYER, GMI_NORMAL, "&Teamplayer", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_4WP, GMI_NORMAL, "EA &4-Way Play", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_CTRLTEST_CONFIG, GMI_NORMAL, "&Configure...", NULL, MACCEL_NONE, 0, "input-gaming", NULL},
	
	{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
};

/** SoundTest menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiSoundTest[] =
{
	{IDM_SOUNDTEST_11025, GMI_NORMAL, "&11,025 Hz", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_16000, GMI_NORMAL, "1&6,000 Hz", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_22050, GMI_NORMAL, "&22,050 Hz", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_32000, GMI_NORMAL, "&32,000 Hz", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_44100, GMI_NORMAL, "&44,100 Hz", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_48000, GMI_NORMAL, "4&8,000 Hz", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SEPARATOR, GMI_SEPARATOR, NULL, NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_MONO, GMI_NORMAL, "Mono", NULL, MACCEL_NONE, 0, NULL, NULL},
	{IDM_SOUNDTEST_STEREO, GMI_NORMAL, "Stereo", NULL, MACCEL_NONE, 0, NULL, NULL},
	
	{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
};

/** Help menu. **/
const GensMenuBar::MenuItem GensMenuBar::ms_gmiHelp[] =
{
	{IDM_HELP_ABOUT, GMI_NORMAL, "&About Gens/GS II", NULL, MACCEL_NONE, 0, "help-about", ":/oxygen-16x16/help-about.png"},
	
	{0, GMI_NORMAL, NULL, NULL, MACCEL_NONE, 0, NULL, NULL}
};

/** Main menu. **/
const GensMenuBar::MainMenuItem GensMenuBar::ms_gmmiMain[] =
{
	{IDM_FILE_MENU, "&File", &ms_gmiFile[0]},
	{IDM_RESBPPTEST_MENU, "&ResBppTest", &ms_gmiResBppTest[0]},
	{IDM_CTRLTEST_MENU, "&CtrlTest", &ms_gmiCtrlTest[0]},
	{IDM_SOUNDTEST_MENU, "&SoundTest", &ms_gmiSoundTest[0]},
	{IDM_HELP_MENU, "&Help", &ms_gmiHelp[0]},
	
	{0, NULL, NULL}
};

}
