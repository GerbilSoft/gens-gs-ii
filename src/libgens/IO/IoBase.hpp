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
			m_buttons = ~0;		// no buttons pressed
			updateSelectLine();
		}
		IoBase(const IoBase *other)
		{
			// Copy tristate control and data buffer from another controller.
			m_ctrl = other->m_ctrl;
			m_lastData = other->m_lastData;
			m_buttons = other->m_buttons;
			updateSelectLine();
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
		virtual void writeCtrl(uint8_t ctrl)
		{
			m_ctrl = ctrl;
			updateSelectLine();
		}
		virtual uint8_t readCtrl(void) { return m_ctrl; }
		
		virtual void writeData(uint8_t data)
		{
			m_lastData = data;
			updateSelectLine();
		}
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
		
		/**
		 * updateSelectLine(): Determine the SELECT line state.
		 */
		inline void updateSelectLine(void)
		{
			m_select = (!(m_ctrl & IOPIN_TH) || (m_lastData & IOPIN_TH));
		}
		inline bool isSelect(void) { return m_select; }
		
		/**
		 * applyTristate(): Apply the Tristate settings to the data value.
		 * @param data Data value.
		 * @return Data value with tristate settings applied.
		 */
		inline uint8_t applyTristate(uint8_t data) const
		{
			data &= (~m_ctrl & 0x7F);		// Mask output bits.
			data |= (m_lastData & (m_ctrl | 0x80));	// Apply data buffer.
			return data;
		}
		
		/**
		 * m_buttons: Controller bitfield.
		 * Format:
		 * - 2-button:          ??CBRLDU
		 * - 3-button:          SACBRLDU
		 * - 6-button: ????MXYZ SACBRLDU
		 * NOTE: ACTIVE LOW! (1 == released; 0 == pressed)
		 */
		unsigned int m_buttons;
		
		// I/O pin definitions.
		enum IoPinDefs
		{
			IOPIN_UP	= 0x01,	// D0
			IOPIN_DOWN	= 0x02,	// D1
			IOPIN_LEFT	= 0x04,	// D2
			IOPIN_RIGHT	= 0x08,	// D3
			IOPIN_TL	= 0x10,	// D4
			IOPIN_TR	= 0x20,	// D5
			IOPIN_TH	= 0x40	// D6
		};
		
		// Button bitfield values.
		enum CtrlButtons
		{
			BTN_UP		= 0x01,
			BTN_DOWN	= 0x02,
			BTN_LEFT	= 0x04,
			BTN_RIGHT	= 0x08,
			BTN_B		= 0x10,
			BTN_C		= 0x20,
			BTN_A		= 0x40,
			BTN_START	= 0x80,
			BTN_Z		= 0x100,
			BTN_Y		= 0x200,
			BTN_X		= 0x400,
			BTN_MODE	= 0x800,
			
			// SMS/GG buttons.
			BTN_1		= 0x10,
			BTN_2		= 0x20
		};
		
#if 0
		std::vector<uint16_t> m_btnMap;
#endif
	
	private:
		// Select line state.
		bool m_select;
};

}

#endif /* __LIBGENS_IO_BASE_HPP__ */
