/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensActions.cpp: Actions handler.                                       *
 * Handles menu events and non-menu actions.                               *
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

#include "GensActions.hpp"

// gqt4_main has gqt4_config.
#include "../gqt4_main.hpp"

// Menu actions.
#include "GensMenuBar_menus.hpp"

namespace GensQt4
{

/**
 * checkEventKey(): Check for non-menu event keys.
 * @param key Gens Keycode. (WITH MODIFIERS)
 * @return True if an event key was processed; false if not.
 */
bool GensActions::checkEventKey(GensKey_t key)
{
	// Look up the action from GensConfig.
	int action = gqt4_config->keyToAction(key);
	if (action == 0)
		return false;
	
	switch (action)
	{
		case IDM_NOMENU_HARDRESET:
			// Hard Reset.
			emit actionResetEmulator(true);
			return true;
		
		case IDM_NOMENU_SOFTRESET:
			// Soft Reset.
			emit actionResetEmulator(false);
			return true;
		
		case IDM_NOMENU_PAUSE:
			// Toggle Paused.
			emit actionTogglePaused();
			return true;
		
		case IDM_NOMENU_FASTBLUR:
			// Toggle Fast Blur.
			gqt4_config->setFastBlur(!gqt4_config->fastBlur());
			return true;
		
		case IDM_NOMENU_SAVESLOT_0:
		case IDM_NOMENU_SAVESLOT_1:
		case IDM_NOMENU_SAVESLOT_2:
		case IDM_NOMENU_SAVESLOT_3:
		case IDM_NOMENU_SAVESLOT_4:
		case IDM_NOMENU_SAVESLOT_5:
		case IDM_NOMENU_SAVESLOT_6:
		case IDM_NOMENU_SAVESLOT_7:
		case IDM_NOMENU_SAVESLOT_8:
		case IDM_NOMENU_SAVESLOT_9:
			// Save slot selection.
			gqt4_config->setSaveSlot(action - IDM_NOMENU_SAVESLOT_0);
			return true;
		
		case IDM_NOMENU_SAVESLOT_PREV:
			// Previous Save Slot.
			gqt4_config->setSaveSlot_Prev();
			return true;
			
		case IDM_NOMENU_SAVESLOT_NEXT:
			// Next Save Slot.
			gqt4_config->setSaveSlot_Next();
			return true;
		
		case IDM_NOMENU_SAVESLOT_LOADFROM:
		case IDM_NOMENU_SAVESLOT_SAVEAS:
			// TODO
			return false;
		
		default:
			break;
	}
	
	// Event key was not handled.
	return false;
}

}
