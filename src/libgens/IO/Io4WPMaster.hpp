/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io4WPMaster.hpp: EA 4-Way Play device. (Master; Port 2)                 *
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

#ifndef __LIBGENS_IO_IO4WPMASTER_HPP__
#define __LIBGENS_IO_IO4WPMASTER_HPP__

#include "IoBase.hpp"
#include "Io4WPSlave.hpp"

namespace LibGens
{

class Io4WPMaster : public IoBase
{
	public:
		Io4WPMaster();
		Io4WPMaster(const IoBase *other);
		virtual ~Io4WPMaster() { }
		
		/**
		 * reset(): Reset function.
		 * Called when the system is reset.
		 */
		void reset();
		
		void writeCtrl(uint8_t ctrl);
		
		void writeData(uint8_t data);
		uint8_t readData(void);
		
		// Controller configuration.
		IoType devType(void) const
			{ return IOT_4WP_MASTER; }
		int numButtons(void) const
			{ return 0; }
		
		// 4WP-specific: Set slave device.
		void setSlaveDevice(Io4WPSlave *slave);
	
	protected:
		Io4WPSlave *m_slave;
		int m_current;	// Current slave controller.
};

}

#endif /* __LIBGENS_IO_IOTEAMPLAYER_HPP__ */
