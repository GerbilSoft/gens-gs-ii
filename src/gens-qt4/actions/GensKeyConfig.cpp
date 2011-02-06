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

// TODO: Create a typedef GensKeyM_t to indicate "with modifiers"?

namespace GensQt4
{

GensKeyConfig::GensKeyConfig()
{
	/** Key configuration. **/
	// TODO: Set default menu keys from GensMenuBar?
	// TODO: Move to another file?
	// TODO: Load from GensConfig?
	
	// TODO: Improve this!
	int keys[][2] =
	{
		{IDM_NOMENU_HARDRESET, KEYV_TAB | KEYM_SHIFT},
		{IDM_NOMENU_SOFTRESET, KEYV_TAB},
		{IDM_NOMENU_PAUSE, KEYV_ESCAPE},
		
		{IDM_NOMENU_FASTBLUR, KEYV_F9},
		
		{IDM_NOMENU_SAVESLOT_0, KEYV_0},
		{IDM_NOMENU_SAVESLOT_1, KEYV_1},
		{IDM_NOMENU_SAVESLOT_2, KEYV_2},
		{IDM_NOMENU_SAVESLOT_3, KEYV_3},
		{IDM_NOMENU_SAVESLOT_4, KEYV_4},
		{IDM_NOMENU_SAVESLOT_5, KEYV_5},
		{IDM_NOMENU_SAVESLOT_6, KEYV_6},
		{IDM_NOMENU_SAVESLOT_7, KEYV_7},
		{IDM_NOMENU_SAVESLOT_8, KEYV_8},
		{IDM_NOMENU_SAVESLOT_9, KEYV_9},
		{IDM_NOMENU_SAVESLOT_PREV, KEYV_F6},
		{IDM_NOMENU_SAVESLOT_NEXT, KEYV_F7},
		{IDM_NOMENU_SAVESLOT_LOADFROM, KEYV_F8 | KEYM_SHIFT},
		{IDM_NOMENU_SAVESLOT_SAVEAS, KEYV_F5 | KEYM_SHIFT},
		
		{0, 0}
	};
	
	for (int *key = &keys[0][0]; key[0]  != 0; key += 2)
	{
		m_hashActionToKey.insert(key[0], (uint32_t)key[1]);
		m_hashKeyToAction.insert((uint32_t)key[1], key[0]);
	}
}

}
