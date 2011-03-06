/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io4WPMaster.cpp: EA 4-Way Play device. (Master; Port 2)                 *
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

#include "Io4WPMaster.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

Io4WPMaster::Io4WPMaster()
	: m_slave(NULL)
	, m_current(0)
{ }

Io4WPMaster::Io4WPMaster(const IoBase *other)
	: IoBase(other)
	, m_slave(NULL)
	, m_current(0)
{
	// TODO: Copy m_slave from another 4WP Master device.
	// TODO: Update m_current based on m_lastData.
}

/**
 * reset(): Reset function.
 * Called when the system is reset.
 */
void Io4WPMaster::reset()
{
	IoBase::reset();
	m_current = 0;	// TODO: Update based on m_lastData.
}


/**
 * writeCtrl(): Set the I/O tristate value.
 * TODO: Combine with writeData().
 * @param ctrl I/O tristate value.
 */
void Io4WPMaster::writeCtrl(uint8_t ctrl)
{
	m_ctrl = ctrl;
	updateSelectLine();
	
	// Update the current slave port.
	uint8_t tris = (~m_ctrl & 0x7F);
	m_current = (((m_lastData | tris) >> 4) & 0x07);
	
	// Update the slave device.
	if (m_slave)
		m_slave->setSlaveController(m_current);
}


/**
 * writeData(): Write data to the controller.
 * * TODO: Combine with writeCtrl().
 * @param data Data to the controller.
 */
void Io4WPMaster::writeData(uint8_t data)
{
	m_lastData = data;
	updateSelectLine();
	
	// Update the current slave port.
	uint8_t tris = (~m_ctrl & 0x7F);
	m_current = (((m_lastData | tris) >> 4) & 0x07);
	
	// Update the slave device.
	if (m_slave)
		m_slave->setSlaveController(m_current);
}

/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t Io4WPMaster::readData(void)
{
	// Master port always returns 0x7F.
	return applyTristate(0x7F);
}


/**
 * setSlaveDevice(): Set the slave device.
 * @param slave Slave device.
 */
void Io4WPMaster::setSlaveDevice(Io4WPSlave *slave)
{
	m_slave = slave;
	if (m_slave)
		m_slave->setSlaveController(m_current);
}

}
