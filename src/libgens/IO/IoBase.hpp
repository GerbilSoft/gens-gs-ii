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

// C includes.
#include <stdint.h>
#include <string.h>

// C++ includes.
#include <vector>

// Key Manager.
#include "KeyManager.hpp"

namespace LibGens
{

class IoBase
{
	public:
		IoBase();
		IoBase(const IoBase *other);
		virtual ~IoBase() { }
		
		/**
		 * reset(): Reset virtual function.
		 * Called when the system is reset.
		 */
		virtual void reset();
		
		// MD-side controller functions.
		// TODO: Trigger IRQ 2 if TH interrupt is enabled.
		virtual void writeCtrl(uint8_t ctrl)
		{
			m_ctrl = ctrl;
			updateSelectLine();
		}
		virtual uint8_t readCtrl(void)
		{
			return m_ctrl;
		}
		
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
		
		// Serial I/O virtual functions.
		// TODO: Baud rate delay handling, TL/TR handling.
		// TODO: Trigger IRQ 2 on data receive if interrupt is enabled.
		// NOTE: Serial mode used is 8n1: 1 start, 8 data, 1 stop = 10 baud per byte.
		virtual void writeSerCtrl(uint8_t serCtrl) { m_serCtrl = serCtrl; }
		virtual uint8_t readSerCtrl(void) { return m_serCtrl & 0xF8; }
		virtual void writeSerTx(uint8_t data) { m_serLastTx = data; }
		virtual uint8_t readSerTx(void) { return m_serLastTx; }
		virtual uint8_t readSerRx(void) { return 0xFF; }
		
		// I/O device update virtual function.
		// This needs to be reimplemented by derived classes.
		virtual void update(void) { }
		
		// Scanline counter virtual function.
		// This is used by the 6-button controller,
		// which resets its internal counter after
		// around 25 scanlines of no TH rising edges.
		virtual void doScanline(void) { }
		
		enum IoType
		{
			IOT_NONE	= 0,
			IOT_3BTN	= 1,
			IOT_6BTN	= 2,
			IOT_2BTN	= 3,
			IOT_MEGA_MOUSE	= 4,
			IOT_TEAMPLAYER	= 5,
			IOT_4WP_MASTER	= 6,
			IOT_4WP_SLAVE	= 7,
			
			IOT_MAX
		};
		
		// Controller configuration.
		virtual const char *devName(void) const { return "None"; }
		virtual IoType devType(void) const { return IOT_NONE; }
		virtual int numButtons(void) const { return 0; }
		virtual int nextLogicalButton(int button) const
		{
			((void)button);
			return -1;
		}
		virtual const char *buttonName(int button) const
		{
			((void)button);
			return NULL;
		}
	
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
		enum ButtonBitfield
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
			BTN_2		= 0x20,
			
			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTN_MOUSE_LEFT		= 0x01,
			BTN_MOUSE_RIGHT		= 0x02,
			BTN_MOUSE_MIDDLE	= 0x04,
			BTN_MOUSE_START		= 0x08	// Start
		};
		
		// Button index values.
		enum ButtonIndex
		{
			BTNI_UP		= 0,
			BTNI_DOWN	= 1,
			BTNI_LEFT	= 2,
			BTNI_RIGHT	= 3,
			BTNI_B		= 4,
			BTNI_C		= 5,
			BTNI_A		= 6,
			BTNI_START	= 7,
			BTNI_Z		= 8,
			BTNI_Y		= 9,
			BTNI_X		= 10,
			BTNI_MODE	= 11,
			
			// SMS/GG buttons.
			BTNI_1		= 4,
			BTNI_2		= 5,
			
			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTNI_MOUSE_LEFT		= 0,
			BTNI_MOUSE_RIGHT	= 1,
			BTNI_MOUSE_MIDDLE	= 2,
			BTNI_MOUSE_START	= 3	// Start
		};
		
		/** Serial I/O definitions and variables. **/
		
		/**
		 * @name Serial I/O control bitfield.
		 */
		enum SerCtrl
		{
			 SERCTRL_TFUL	= 0x01,		// TxdFull (1 == full)
			 SERCTRL_RRDY	= 0x02,		// RxdReady (1 == ready)
			 SERCTRL_RERR	= 0x04,		// RxdError (1 == error)
			 SERCTRL_RINT	= 0x08,		// Rxd Interrupt (1 == on)
			 SERCTRL_SOUT	= 0x10,		// TL mode. (1 == serial out; 0 == parallel)
			 SERCTRL_SIN	= 0x20,		// TR mode. (1 == serial in; 0 == parallel)
			 SERCTRL_BPS0	= 0x40,
			 SERCTRL_BPS1	= 0x80
		};
		
		/**
		 * @name Serial I/O baud rate values.
		 */
		enum SerBaud
		{
			SERBAUD_4800	= 0x00,
			SERBAUD_2400	= 0x01,
			SERBAUD_1200	= 0x02,
			SERBAUD_300	= 0x03
		};
		
		// Serial I/O variables.
		// TODO: Serial data buffer.
		uint8_t m_serCtrl;	// Serial control.
		uint8_t m_serLastTx;	// Last transmitted data byte.
		
		/** Controller configuration. **/
		std::vector<GensKey_t> m_keyMap;
	
	private:
		// Select line state.
		bool m_select;
};

}

#endif /* __LIBGENS_IO_BASE_HPP__ */
