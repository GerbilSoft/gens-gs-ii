/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io4WPSlave.hpp: EA 4-Way Play device. (Slave; Port 1)                   *
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

#ifndef __LIBGENS_IO_IO4WPSLAVE_HPP__
#define __LIBGENS_IO_IO4WPSLAVE_HPP__

#include "IoBase.hpp"

namespace LibGens
{

class Io4WPSlave : public IoBase
{
	public:
		Io4WPSlave();
		Io4WPSlave(const IoBase *other);
		virtual ~Io4WPSlave();
		
		/**
		 * reset(): Reset function.
		 * Called when the system is reset.
		 */
		void reset();
		
		void writeCtrl(uint8_t ctrl);
		
		void writeData(uint8_t data);
		uint8_t readData(void);
		
		// I/O device update function.
		void update(void);
		
		// Controller configuration. (STATIC functions)
		static IoType DevType(void);
		static int NumButtons(void);
		
		// Controller configuration. (virtual functions)
		virtual IoType devType(void) const;
		virtual int numButtons(void) const;
		
		// 4WP-specific: Set slave controller.
		void setSlaveController(int id);
	
	protected:
		IoBase *m_devs[4];	// I/O devices.
		int m_current;		// Current slave controller.
};

// Controller configuration. (STATIC functions)
inline IoBase::IoType Io4WPSlave::DevType(void)
	{ return IOT_4WP_SLAVE; }
inline int Io4WPSlave::NumButtons(void)
	{ return 0; }

// Controller configuration. (virtual functions)
inline IoBase::IoType Io4WPSlave::devType(void) const
	{ return DevType(); }
inline int Io4WPSlave::numButtons(void) const
	{ return NumButtons(); }

}

#endif /* __LIBGENS_IO_IO4WPSLAVE_HPP__ */
