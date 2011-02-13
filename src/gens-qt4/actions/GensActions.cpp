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

// Gens Window.
#include "../GensWindow.hpp"

// Other windows.
#include "../AboutWindow.hpp"
#include "../CtrlConfigWindow.hpp"
#include "../GeneralConfigWindow.hpp"
#include "../McdControlWindow.hpp"

// LibGens includes.
#include "libgens/MD/VdpPalette.hpp"

namespace GensQt4
{

GensActions::GensActions(GensWindow *parent)
	: QObject(parent)
{
	m_parent = parent;
}


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
	
	// Do the action.
	return doAction(action);
}


/**
 * doAction(): Do an action.
 * @param action Action ID. (from GensMenuBar_menus.hpp)
 * @return True if handled; false if not.
 */
bool GensActions::doAction(int action)
{
	switch (MNUID_MENU(action))
	{
		case IDM_FILE_MENU:
			// File menu.
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_FILE_OPEN):
					m_parent->openRom();
					return true;
				
				case MNUID_ITEM(IDM_FILE_CLOSE):
					m_parent->closeRom();
					return true;
				
				case MNUID_ITEM(IDM_FILE_SAVESTATE):
					m_parent->saveState();
					return true;
				
				case MNUID_ITEM(IDM_FILE_LOADSTATE):
					m_parent->loadState();
					return true;
				
				case MNUID_ITEM(IDM_FILE_GENCONFIG):
					GeneralConfigWindow::ShowSingle(m_parent);
					return true;
				
				case MNUID_ITEM(IDM_FILE_MCDCONTROL):
					McdControlWindow::ShowSingle(m_parent);
					return true;
				
				case MNUID_ITEM(IDM_FILE_QUIT):
					// Quit.
					m_parent->closeRom();
					QuitGens();
					m_parent->close();
					return true;
				
				default:
					break;
			}
			break;
		
		case IDM_HELP_MENU:
			// Help menu.
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_HELP_ABOUT):
					// About Gens/GS II.
					AboutWindow::ShowSingle(m_parent);
					return true;
				
				default:
					break;
			}
			break;
		
		case IDM_GRAPHICS_MENU:
			// Graphics.
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_GRAPHICS_SCRSHOT):
					m_parent->screenShot();
					return true;
				
				default:
					break;
			}
			break;
		
		case IDM_GRAPHICS_RES_MENU:
			// Graphics, Resolution.
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_GRAPHICS_RES_1X):
					m_parent->rescale(1);
					return true;
				
				case MNUID_ITEM(IDM_GRAPHICS_RES_2X):
					m_parent->rescale(2);
					return true;
				
				case MNUID_ITEM(IDM_GRAPHICS_RES_3X):
					m_parent->rescale(3);
					return true;
				
				case MNUID_ITEM(IDM_GRAPHICS_RES_4X):
					m_parent->rescale(4);
					return true;
				
				default:
					break;
			}
			break;
		
		case IDM_GRAPHICS_BPP_MENU:
			// Graphics, Color Depth.
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_GRAPHICS_BPP_15):
					m_parent->setBpp(LibGens::VdpPalette::BPP_15);
					return true;
				
				case MNUID_ITEM(IDM_GRAPHICS_BPP_16):
					m_parent->setBpp(LibGens::VdpPalette::BPP_16);
					return true;
				
				case MNUID_ITEM(IDM_GRAPHICS_BPP_32):
					m_parent->setBpp(LibGens::VdpPalette::BPP_32);
					return true;
				
				default:
					break;
			}
			break;
		
		case IDM_CTRLTEST_MENU:
			// Controller Testing
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_CTRLTEST_NONE):
					m_parent->setController(0, LibGens::IoBase::IOT_NONE);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_3BT):
					m_parent->setController(0, LibGens::IoBase::IOT_3BTN);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_6BT):
					m_parent->setController(0, LibGens::IoBase::IOT_6BTN);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_2BT):
					m_parent->setController(0, LibGens::IoBase::IOT_2BTN);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_MEGAMOUSE):
					m_parent->setController(0, LibGens::IoBase::IOT_MEGA_MOUSE);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_TEAMPLAYER):
					m_parent->setController(0, LibGens::IoBase::IOT_TEAMPLAYER);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_4WP):
					// TODO
					m_parent->setController(0, LibGens::IoBase::IOT_4WP_SLAVE);
					m_parent->setController(1, LibGens::IoBase::IOT_4WP_MASTER);
					return true;
				
				case MNUID_ITEM(IDM_CTRLTEST_CONFIG):
					// Controller Configuration.
					CtrlConfigWindow::ShowSingle(m_parent);
					return true;
				
				default:
					break;
			}
			break;
		
		case IDM_SOUNDTEST_MENU:
			// Audio Testing
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_SOUNDTEST_11025):
					m_parent->setAudioRate(11025);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_16000):
					m_parent->setAudioRate(16000);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_22050):
					m_parent->setAudioRate(22050);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_32000):
					m_parent->setAudioRate(32000);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_44100):
					m_parent->setAudioRate(44100);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_48000):
					m_parent->setAudioRate(48000);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_MONO):
					m_parent->setStereo(false);
					return true;
				case MNUID_ITEM(IDM_SOUNDTEST_STEREO):
					m_parent->setStereo(true);
					return true;
				default:
					break;
			}
			break;
		
		case IDM_NOMENU:
			// Non-Menu Actions.
			switch (MNUID_ITEM(action))
			{
				case MNUID_ITEM(IDM_NOMENU_HARDRESET):
					// Hard Reset.
					emit actionResetEmulator(true);
					return true;
				
				case MNUID_ITEM(IDM_NOMENU_SOFTRESET):
					// Soft Reset.
					emit actionResetEmulator(false);
					return true;
				
				case MNUID_ITEM(IDM_NOMENU_PAUSE):
					// Toggle Paused.
					emit actionTogglePaused();
					return true;
				
				case MNUID_ITEM(IDM_NOMENU_FASTBLUR):
					// Toggle Fast Blur.
					gqt4_config->setFastBlur(!gqt4_config->fastBlur());
					return true;
				
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_0):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_1):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_2):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_3):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_4):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_5):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_6):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_7):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_8):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_9):
					// Save slot selection.
					gqt4_config->setSaveSlot(action - IDM_NOMENU_SAVESLOT_0);
					return true;
				
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_PREV):
					// Previous Save Slot.
					gqt4_config->setSaveSlot_Prev();
					return true;
					
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_NEXT):
					// Next Save Slot.
					gqt4_config->setSaveSlot_Next();
					return true;
				
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_LOADFROM):
				case MNUID_ITEM(IDM_NOMENU_SAVESLOT_SAVEAS):
					// TODO
					return false;
				
				default:
					break;
			}
		
		default:
			break;
	}
	
	// Action wasn't handled.
	return false;
}

}

