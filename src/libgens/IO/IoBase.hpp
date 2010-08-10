/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoBase.hpp: I/O base class.                                             *
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

#ifndef __LIBGENS_IO_BASE_HPP__
#define __LIBGENS_IO_BASE_HPP__

#include <stdint.h>
#include <vector>

namespace LibGens
{

class IoBase
{
	public:
		IoBase()
		{
#if 0
			// Initialize the button mapping vector.
			m_btnMap.resize(GetNumButtons(), 0);
#endif
			// Initialize tristate control and data buffer.
			// TODO: Initialize to 0xFF or 0x00?
			m_ctrl = 0x00;		// input
			m_lastData = 0xFF;	// all ones
		}
		virtual ~IoBase() { }
		
#if 0
		// Number of buttons on the controller.
		virtual static int GetNumButtons(void) = 0;
		virtual static const char* GetButtonName(int button) = 0;
		
		// Controller modes.
		virtual static int GetNumModes(void) = 0;
		virtual static const char* GetModeName(int mode) = 0;
#endif
		
		// Set/get button mapping.
		
		// MD-side controller functions.
		virtual void writeCtrl(uint8_t ctrl) { m_ctrl = ctrl; }
		virtual uint8_t readCtrl(void) { return m_ctrl; }
		
		virtual void writeData(uint8_t data) { m_lastData = data; }
		virtual uint8_t readData(void)
		{
			// Mask the data according to the tristate control.
			// Tristate is 0 for input and 1 for output.
			// Note that tristate bit 7 is used for TH interrupt.
			// All input bits should read 1.
			// All output bits should read the last value written.
			uint8_t tris = (~m_ctrl & 0x7F);
			return (m_lastData | tris);
		}
		
		// Keypress handling virtual functions.
		// These need to be reimplemented by derived classes.
		virtual void keyPress(int key) { ((void)key); }
		virtual void keyRelease(int key) { ((void)key); }
	
	protected:
		uint8_t m_ctrl;		// Tristate control.
		uint8_t m_lastData;	// Last data written to the device.
		
#if 0
		std::vector<uint16_t> m_btnMap;
#endif
};

}

#endif /* __LIBGENS_IO_BASE_HPP__ */
