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

// Menu IDs.
// TODO: Convert to enum?
#define MNUID(menu, item) ((menu << 16) | (item))
#define MNUID_MENU(id) (id >> 16)
#define MNUID_ITEM(id) (id & 0xFFFF)

#define IDM_SEPARATOR		-1

#define IDM_FILE_MENU		1
#define IDM_FILE_OPEN		MNUID(IDM_FILE_MENU, 1)
#define IDM_FILE_CLOSE		MNUID(IDM_FILE_MENU, 2)
#define IDM_FILE_SAVESTATE	MNUID(IDM_FILE_MENU, 3)
#define IDM_FILE_LOADSTATE	MNUID(IDM_FILE_MENU, 4)
#define IDM_FILE_GENCONFIG	MNUID(IDM_FILE_MENU, 5)
#define IDM_FILE_MCDCONTROL	MNUID(IDM_FILE_MENU, 6)
#define IDM_FILE_QUIT		MNUID(IDM_FILE_MENU, 0xFFFF)

#define IDM_HELP_MENU		7
#define IDM_HELP_ABOUT		MNUID(IDM_HELP_MENU, 1)

#define IDM_RESBPPTEST_MENU	64
#define IDM_RESBPPTEST_1X	MNUID(IDM_RESBPPTEST_MENU, 1)
#define IDM_RESBPPTEST_2X	MNUID(IDM_RESBPPTEST_MENU, 2)
#define IDM_RESBPPTEST_3X	MNUID(IDM_RESBPPTEST_MENU, 3)
#define IDM_RESBPPTEST_4X	MNUID(IDM_RESBPPTEST_MENU, 4)
#define IDM_RESBPPTEST_15	MNUID(IDM_RESBPPTEST_MENU, 5)
#define IDM_RESBPPTEST_16	MNUID(IDM_RESBPPTEST_MENU, 6)
#define IDM_RESBPPTEST_32	MNUID(IDM_RESBPPTEST_MENU, 7)
#define IDM_RESBPPTEST_SCRSHOT	MNUID(IDM_RESBPPTEST_MENU, 8)

#define IDM_CTRLTEST_MENU	65
#define IDM_CTRLTEST_NONE	MNUID(IDM_CTRLTEST_MENU, 1)
#define IDM_CTRLTEST_3BT	MNUID(IDM_CTRLTEST_MENU, 2)
#define IDM_CTRLTEST_6BT	MNUID(IDM_CTRLTEST_MENU, 3)
#define IDM_CTRLTEST_2BT	MNUID(IDM_CTRLTEST_MENU, 4)
#define IDM_CTRLTEST_MEGAMOUSE	MNUID(IDM_CTRLTEST_MENU, 5)
#define IDM_CTRLTEST_TEAMPLAYER	MNUID(IDM_CTRLTEST_MENU, 6)
#define IDM_CTRLTEST_4WP	MNUID(IDM_CTRLTEST_MENU, 7)
#define IDM_CTRLTEST_CONFIG	MNUID(IDM_CTRLTEST_MENU, 0xFFFF)

#define IDM_SOUNDTEST_MENU	66
#define IDM_SOUNDTEST_11025	MNUID(IDM_SOUNDTEST_MENU, 1)
#define IDM_SOUNDTEST_16000	MNUID(IDM_SOUNDTEST_MENU, 2)
#define IDM_SOUNDTEST_22050	MNUID(IDM_SOUNDTEST_MENU, 3)
#define IDM_SOUNDTEST_32000	MNUID(IDM_SOUNDTEST_MENU, 4)
#define IDM_SOUNDTEST_44100	MNUID(IDM_SOUNDTEST_MENU, 5)
#define IDM_SOUNDTEST_48000	MNUID(IDM_SOUNDTEST_MENU, 6)
#define IDM_SOUNDTEST_MONO	MNUID(IDM_SOUNDTEST_MENU, 11)
#define IDM_SOUNDTEST_STEREO	MNUID(IDM_SOUNDTEST_MENU, 12)

#endif /* __GENS_QT4_GENSMENUBAR_MENUS_HPP__ */
