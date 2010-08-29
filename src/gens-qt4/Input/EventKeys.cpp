/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EventKeys.cpp: Event key handler.                                       *
 * Used for mapping keys to non-controller events, e.g. savestates.        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "EventKeys.hpp"

// TODO: Customizable event keys.
// NOTE: Menu items currently have their own events.
// TODO: Remap them to EventKeys?

EventKeys::EventKeys()
{
	// TODO
}

EventKeys::~EventKeys()
{
	// TODO
}


/**
 * checkEventKey(): Check for event keys.
 * @param key Gens Keycode.
 * @param mod Modifier keys. (TODO)
 * @return True if an event key was processed; false if not.
 */
bool EventKeys::checkEventKey(GensKey_t key, int mod)
{
	// TODO: Customizable event keys.
	
	switch (key)
	{
		case KEYV_ESCAPE:
			if (mod == Qt::NoModifier)
			{
				// Toggle Paused.
				emit eventTogglePaused();
				return true;
			}
			break;
		
		case KEYV_F9:
			if (mod == Qt::NoModifier)
			{
				// Toggle Fast Blur.
				emit eventToggleFastBlur();
				return true;
			}
			break;
		
		case KEYV_TAB:
			if (mod == Qt::NoModifier)
			{
				// Soft Reset.
				emit eventResetEmulator(false);
				return true;
			}
			else if (mod == Qt::ShiftModifier)
			{
				// Hard Reset.
				emit eventResetEmulator(true);
				return true;
			}
			break;
		
		case KEYV_0:
		case KEYV_1:
		case KEYV_2:
		case KEYV_3:
		case KEYV_4:
		case KEYV_5:
		case KEYV_6:
		case KEYV_7:
		case KEYV_8:
		case KEYV_9:
			if (mod == Qt::NoModifier)
			{
				// Save Slot selection.
				emit eventSetSaveSlot(key - KEYV_0);
				return true;
			}
			break;
		
		case KEYV_F6:
			if (mod == Qt::NoModifier)
			{
				// Previous Save Slot.
				emit eventPrevSaveSlot();
				return true;
			}
			break;
			
		case KEYV_F7:
			if (mod == Qt::NoModifier)
			{
				// Next Save Slot.
				emit eventNextSaveSlot();
				return true;
			}
			break;
		
		default:
			break;
	}
	
	// Event key was not handled.
	return false;
}
