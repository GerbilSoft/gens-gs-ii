/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoBase.cpp: I/O base class.                                             *
 * This class can be used to simulate a port with no device connected.     *
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

#include "IoBase.hpp"

namespace LibGens
{

IoBase::IoBase()
{
	// Initialize tristate control and data buffer.
	// TODO: What should 0x40 be initialized to?
	// - 0x40: SMS compatibility (breaks Multitap Test ROM when controllers are swapped)
	//   - Nemesis' Sprite Masking & Test ROM seems to expect this...
	// - 0x00: all zeroes (current option)
	// - 0xFF: all ones
	
	m_ctrl = 0x00;		// input
	m_lastData = 0xFF;	// all ones
	m_buttons = ~0;		// no buttons pressed
	
	// Serial settings.
	m_serCtrl = 0x00;	// parallel mode
	m_serLastTx = 0xFF;
	
	updateSelectLine();
	
	// Clear the keymap.
	// NOTE: Derived classes must resize the keymap themselves!
	// (We can't call virtual functions from the constructor.)
	m_keyMap.clear();
}


IoBase::IoBase(const IoBase *other)
{
	if (other)
	{
		// Copy tristate control and data buffer from another controller.
		m_ctrl = other->m_ctrl;
		m_lastData = other->m_lastData;
		
		// Serial settings.
		m_serCtrl = other->m_serCtrl;
		m_serLastTx = other->m_serLastTx;
	}
	else
	{
		// NULL device specified.
		m_ctrl = 0x00;		// input
		m_lastData = 0xFF;	// all ones
		
		// Serial settings.
		m_serCtrl = 0x00;	// parallel mode
		m_serLastTx = 0xFF;
	}
	
	// Buttons are NOT copied!
	m_buttons = ~0;
	
	updateSelectLine();
	
	// Clear the keymap.
	// NOTE: Derived classes must resize the keymap themselves!
	// (We can't call virtual functions from the constructor.)
	// TODO: Copy the keymap from the other controller?
	m_keyMap.clear();
}


/**
 * reset(): Reset virtual function.
 * Called when the system is reset.
 */
void IoBase::reset(void)
{
	m_ctrl = 0x00;
	m_lastData = 0xFF;
	m_buttons = ~0;
	updateSelectLine();
}


/**
 * update(): I/O device update function.
 */
void IoBase::update(void)
{
	m_buttons = 0;
	
	for (int i = (m_keyMap.size() - 1); i >= 0; i--)
	{
		m_buttons <<= 1;
		m_buttons |= DevManager::IsKeyPressed(m_keyMap[i]);
	}
	
	// Buttons typically use active-low logic.
	m_buttons = ~m_buttons;
}


/** ZOMG savestate functions. **/


/**
 * zomgSaveMD(): Save the controller port state. (MD version)
 * @param state Zomg_MD_IoSave_int_t struct to save to.
 */
void IoBase::zomgSaveMD(Zomg_MD_IoSave_int_t *state)
{
	state->data = m_lastData;
	state->ctrl = m_ctrl;
	
	// Serial I/O registers.
	state->ser_tx = m_serLastTx;
	state->ser_rx = 0xFF; // TODO
	state->ser_ctrl = m_serCtrl;
}


/**
 * zomgRestoreMD(): Restore the controller port state. (MD version)
 * @param state Zomg_MD_IoSave_int_t struct to restore from.
 */
void IoBase::zomgRestoreMD(const Zomg_MD_IoSave_int_t *state)
{
	// TODO: writeCtrl() / writeData() or just manually save it?
	this->writeCtrl(state->ctrl);
	this->writeData(state->data);
	
	// Serial I/O registers.
	m_serLastTx = state->ser_tx;
	//m_serRx = state->ser_rx; // TODO
	m_serCtrl = state->ser_ctrl;
}

}
