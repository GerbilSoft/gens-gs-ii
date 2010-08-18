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
	
	// CTRL is initialized to 0x40 for SMS compatibility.
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
	// Copy tristate control and data buffer from another controller.
	m_ctrl = other->m_ctrl;
	m_lastData = other->m_lastData;
	m_buttons = ~0;		// buttons are NOT copied!
	
	// Serial settings.
	m_serCtrl = other->m_serCtrl;
	
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
void IoBase::reset()
{
	m_ctrl = 0x00;
	m_lastData = 0xFF;
	m_buttons = ~0;
	updateSelectLine();
}

}
