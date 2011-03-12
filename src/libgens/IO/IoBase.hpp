/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoBase.hpp: I/O base class.                                             *
 * This class can be used to simulate a port with no device connected.     *
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

#ifndef __LIBGENS_IO_BASE_HPP__
#define __LIBGENS_IO_BASE_HPP__

// C includes.
#include <stdint.h>
#include <string.h>

// C++ includes.
#include <vector>

// Device Manager.
#include "../GensInput/DevManager.hpp"

namespace LibGens
{

class IoBase
{
	public:
		IoBase(const IoBase *other = NULL);
		virtual ~IoBase() { }
		
		/**
		 * reset(): Reset virtual function.
		 * Called when the system is reset.
		 */
		virtual void reset(void);
		
		/** MD-side controller functions. **/
		// TODO: Trigger IRQ 2 if TH interrupt is enabled.
		virtual void writeCtrl(uint8_t ctrl);
		virtual uint8_t readCtrl(void) const;
		virtual void writeData(uint8_t data);
		virtual uint8_t readData(void) const;
		
		// Serial I/O virtual functions.
		// TODO: Baud rate delay handling, TL/TR handling.
		// TODO: Trigger IRQ 2 on data receive if interrupt is enabled.
		// NOTE: Serial mode used is 8n1: 1 start, 8 data, 1 stop = 10 baud per byte.
		virtual void writeSerCtrl(uint8_t serCtrl);
		virtual uint8_t readSerCtrl(void) const;
		virtual void writeSerTx(uint8_t data);
		virtual uint8_t readSerTx(void) const;
		virtual uint8_t readSerRx(void) const;
		
		/**
		 * update(): I/O device update function.
		 * This will work for controllers that act like regular controllers,
		 * but will need to be reimplemented for multitaps and possibly
		 * the Mega Mouse.
		 */
		virtual void update(void);
		
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
		
		// Controller configuration. (STATIC functions)
		static IoType DevType(void);
		static int NumButtons(void);
		static int NextLogicalButton(int button);
		
		// Controller configuration. (virtual functions)
		virtual IoType devType(void) const;
		virtual int numButtons(void) const;
		virtual int nextLogicalButton(int button) const;
		
		// Logical button names.
		// These are used for button name trnaslation in the UI.
		enum ButtonName_t
		{
			BTNNAME_UNKNOWN	= -1,
			
			// Standard controller buttons.
			BTNNAME_UP	= 0,
			BTNNAME_DOWN	= 1,
			BTNNAME_LEFT	= 2,
			BTNNAME_RIGHT	= 3,
			BTNNAME_B	= 4,
			BTNNAME_C	= 5,
			BTNNAME_A	= 6,
			BTNNAME_START	= 7,
			BTNNAME_Z	= 8,
			BTNNAME_Y	= 9,
			BTNNAME_X	= 10,
			BTNNAME_MODE	= 11,
			
			// SMS/GG buttons.
			BTNNAME_1	= 12,
			BTNNAME_2	= 13,
			
			// Sega Mega Mouse buttons.
			BTNNAME_MOUSE_LEFT	= 14,
			BTNNAME_MOUSE_RIGHT	= 15,
			BTNNAME_MOUSE_MIDDLE	= 16,
			BTNNAME_MOUSE_START	= 17,
			
			BTNNAME_MAX
		};
		
		// Get button names.
		static ButtonName_t ButtonName(int button);
		virtual ButtonName_t buttonName(int button) const;
		
		// Set/get keymap.
		int setKeymap(const GensKey_t *keymap, int count);
		int keymap(GensKey_t *keymap, int siz) const;
		
		/** ZOMG savestate functions. **/
		struct Zomg_MD_IoSave_int_t
		{
			uint8_t data;
			uint8_t ctrl;
			uint8_t ser_tx;
			uint8_t ser_rx;
			uint8_t ser_ctrl;
		};
		void zomgSaveMD(Zomg_MD_IoSave_int_t *state) const;
		void zomgRestoreMD(const Zomg_MD_IoSave_int_t *state);
	
	protected:
		uint8_t m_ctrl;		// Tristate control.
		uint8_t m_lastData;	// Last data written to the device.
		
		/**
		 * updateSelectLine(): Determine the SELECT line state.
		 */
		inline void updateSelectLine(void)
			{ m_select = (!(m_ctrl & IOPIN_TH) || (m_lastData & IOPIN_TH)); }
		inline bool isSelect(void) const
			{ return m_select; }
		
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
			BTNI_UNKNOWN	= -1,
			
			// Standard controller buttons.
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


/** MD-side controller functions. **/

// TODO: Trigger IRQ 2 if TH interrupt is enabled.
inline void IoBase::writeCtrl(uint8_t ctrl)
	{ m_ctrl = ctrl; updateSelectLine(); }
inline uint8_t IoBase::readCtrl(void) const
	{ return m_ctrl; }

inline void IoBase::writeData(uint8_t data)
	{ m_lastData = data; updateSelectLine(); }
inline uint8_t IoBase::readData(void) const
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
inline void IoBase::writeSerCtrl(uint8_t serCtrl)
	{ m_serCtrl = serCtrl; }
inline uint8_t IoBase::readSerCtrl(void) const
	{ return m_serCtrl & 0xF8; }
inline void IoBase::writeSerTx(uint8_t data)
	{ m_serLastTx = data; }
inline uint8_t IoBase::readSerTx(void) const
	{ return m_serLastTx; }
inline uint8_t IoBase::readSerRx(void) const
	{ return 0xFF; }


/** Controller configuration. **/

// Controller configuration. (STATIC functions)
inline IoBase::IoType IoBase::DevType(void)
	{ return IOT_NONE; }
inline int IoBase::NumButtons(void)
	{ return 0; }
inline int IoBase::NextLogicalButton(int button)
	{ ((void)button); return -1; }

// Controller configuration. (virtual functions)
inline IoBase::IoType IoBase::devType(void) const
	{ return DevType(); }
inline int IoBase::numButtons(void) const
	{ return NumButtons(); }
inline int IoBase::nextLogicalButton(int button) const
	{ return NextLogicalButton(button); }

/**
 * ButtonName(): Get the name for a given button index. (STATIC function)
 * @param button Button index.
 * @return Button name, or BTNNAME_UNKNOWN if the button index is invalid.
 */
inline IoBase::ButtonName_t IoBase::ButtonName(int button)
{
	((void)button);
	return BTNNAME_UNKNOWN;
}

/**
 * buttonName(): Get the name for a given button index. (virtual function)
 * @param button Button index.
 * @return Button name, or BTNNAME_UNKNOWN if the button index is invalid.
 */
inline IoBase::ButtonName_t IoBase::buttonName(int button) const
	{ return ButtonName(button); }

}

#endif /* __LIBGENS_IO_BASE_HPP__ */
