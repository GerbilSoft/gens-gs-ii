/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensMenuBar_menus.hpp: Gens Menu Bar class: Menu definitions.           *
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

#ifndef __GENS_QT4_GENSMENUBAR_MENUS_HPP__
#define __GENS_QT4_GENSMENUBAR_MENUS_HPP__

// Qt includes.
#include <QtCore/qglobal.h>

// Menu IDs.
// TODO: Convert to enum?
#define MNUID(menu, item) ((menu << 16) | (item))
#define MNUID_MENU(id) (id >> 16)
#define MNUID_ITEM(id) (id & 0xFFFF)

#define IDM_SEPARATOR		-1

#define IDM_FILE_MENU		1
#define IDM_FILE_OPEN		MNUID(IDM_FILE_MENU, 1)
#define IDM_FILE_RECENT		MNUID(IDM_FILE_MENU, 2)
#define IDM_FILE_CLOSE		MNUID(IDM_FILE_MENU, 3)
#define IDM_FILE_SAVESTATE	MNUID(IDM_FILE_MENU, 4)
#define IDM_FILE_LOADSTATE	MNUID(IDM_FILE_MENU, 5)
#define IDM_FILE_GENCONFIG	MNUID(IDM_FILE_MENU, 6)
#define IDM_FILE_MCDCONTROL	MNUID(IDM_FILE_MENU, 7)
#define IDM_FILE_QUIT		MNUID(IDM_FILE_MENU, 0xFFFF)

#define IDM_GRAPHICS_MENU	2
#ifndef Q_WS_MAC
#define IDM_GRAPHICS_MENUBAR	MNUID(IDM_GRAPHICS_MENU, 1)
#endif /* !Q_WS_MAC */
#define IDM_GRAPHICS_RES	MNUID(IDM_GRAPHICS_MENU, 2)
#define IDM_GRAPHICS_BPP	MNUID(IDM_GRAPHICS_MENU, 3)
#define IDM_GRAPHICS_STRETCH	MNUID(IDM_GRAPHICS_MENU, 4)
#define IDM_GRAPHICS_SCRSHOT	MNUID(IDM_GRAPHICS_MENU, 5)

#define IDM_GRAPHICS_RES_MENU	3
#define IDM_GRAPHICS_RES_1X	MNUID(IDM_GRAPHICS_RES_MENU, 1)
#define IDM_GRAPHICS_RES_2X	MNUID(IDM_GRAPHICS_RES_MENU, 2)
#define IDM_GRAPHICS_RES_3X	MNUID(IDM_GRAPHICS_RES_MENU, 3)
#define IDM_GRAPHICS_RES_4X	MNUID(IDM_GRAPHICS_RES_MENU, 4)

#define IDM_GRAPHICS_BPP_MENU	4
#define IDM_GRAPHICS_BPP_15	MNUID(IDM_GRAPHICS_BPP_MENU, 1)
#define IDM_GRAPHICS_BPP_16	MNUID(IDM_GRAPHICS_BPP_MENU, 2)
#define IDM_GRAPHICS_BPP_32	MNUID(IDM_GRAPHICS_BPP_MENU, 3)

#define IDM_GRAPHICS_STRETCH_MENU	5
#define IDM_GRAPHICS_STRETCH_NONE	MNUID(IDM_GRAPHICS_STRETCH_MENU, 1)
#define IDM_GRAPHICS_STRETCH_H		MNUID(IDM_GRAPHICS_STRETCH_MENU, 2)
#define IDM_GRAPHICS_STRETCH_V		MNUID(IDM_GRAPHICS_STRETCH_MENU, 3)
#define IDM_GRAPHICS_STRETCH_FULL	MNUID(IDM_GRAPHICS_STRETCH_MENU, 4)

#define IDM_SYSTEM_MENU			6
#define IDM_SYSTEM_REGION		MNUID(IDM_SYSTEM_MENU, 1)
#define IDM_SYSTEM_HARDRESET		MNUID(IDM_SYSTEM_MENU, 2)
#define IDM_SYSTEM_SOFTRESET		MNUID(IDM_SYSTEM_MENU, 3)
#define IDM_SYSTEM_PAUSE		MNUID(IDM_SYSTEM_MENU, 4)
#define IDM_SYSTEM_CPURESET_M68K	MNUID(IDM_SYSTEM_MENU, 21)
#define IDM_SYSTEM_CPURESET_Z80		MNUID(IDM_SYSTEM_MENU, 22)

#define IDM_SYSTEM_REGION_MENU		7
#define IDM_SYSTEM_REGION_AUTODETECT	MNUID(IDM_SYSTEM_REGION_MENU, 1)
#define IDM_SYSTEM_REGION_JAPAN		MNUID(IDM_SYSTEM_REGION_MENU, 2)
#define IDM_SYSTEM_REGION_ASIA		MNUID(IDM_SYSTEM_REGION_MENU, 3)
#define IDM_SYSTEM_REGION_USA		MNUID(IDM_SYSTEM_REGION_MENU, 4)
#define IDM_SYSTEM_REGION_EUROPE	MNUID(IDM_SYSTEM_REGION_MENU, 5)

#define IDM_HELP_MENU		15
#define IDM_HELP_ABOUT		MNUID(IDM_HELP_MENU, 1)

#define IDM_OPTIONS_MENU	8
#define IDM_OPTIONS_CONTROLLERS	MNUID(IDM_OPTIONS_MENU, 1)

#define IDM_SOUNDTEST_MENU	66
#define IDM_SOUNDTEST_11025	MNUID(IDM_SOUNDTEST_MENU, 1)
#define IDM_SOUNDTEST_16000	MNUID(IDM_SOUNDTEST_MENU, 2)
#define IDM_SOUNDTEST_22050	MNUID(IDM_SOUNDTEST_MENU, 3)
#define IDM_SOUNDTEST_32000	MNUID(IDM_SOUNDTEST_MENU, 4)
#define IDM_SOUNDTEST_44100	MNUID(IDM_SOUNDTEST_MENU, 5)
#define IDM_SOUNDTEST_48000	MNUID(IDM_SOUNDTEST_MENU, 6)
#define IDM_SOUNDTEST_MONO	MNUID(IDM_SOUNDTEST_MENU, 11)
#define IDM_SOUNDTEST_STEREO	MNUID(IDM_SOUNDTEST_MENU, 12)

// Non-menu actions.
#define IDM_NOMENU		127

// Graphics.
#define IDM_NOMENU_FASTBLUR	MNUID(IDM_NOMENU, 21)

// Savestates.
#define IDM_NOMENU_SAVESLOT_0		MNUID(IDM_NOMENU, 100)
#define IDM_NOMENU_SAVESLOT_1		MNUID(IDM_NOMENU, 101)
#define IDM_NOMENU_SAVESLOT_2		MNUID(IDM_NOMENU, 102)
#define IDM_NOMENU_SAVESLOT_3		MNUID(IDM_NOMENU, 103)
#define IDM_NOMENU_SAVESLOT_4		MNUID(IDM_NOMENU, 104)
#define IDM_NOMENU_SAVESLOT_5		MNUID(IDM_NOMENU, 105)
#define IDM_NOMENU_SAVESLOT_6		MNUID(IDM_NOMENU, 106)
#define IDM_NOMENU_SAVESLOT_7		MNUID(IDM_NOMENU, 107)
#define IDM_NOMENU_SAVESLOT_8		MNUID(IDM_NOMENU, 108)
#define IDM_NOMENU_SAVESLOT_9		MNUID(IDM_NOMENU, 109)
#define IDM_NOMENU_SAVESLOT_PREV	MNUID(IDM_NOMENU, 110)
#define IDM_NOMENU_SAVESLOT_NEXT	MNUID(IDM_NOMENU, 111)
#define IDM_NOMENU_SAVESLOT_LOADFROM	MNUID(IDM_NOMENU, 112)
#define IDM_NOMENU_SAVESLOT_SAVEAS	MNUID(IDM_NOMENU, 113)

#endif /* __GENS_QT4_GENSMENUBAR_MENUS_HPP__ */
