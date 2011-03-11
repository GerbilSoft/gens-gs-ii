/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io4WPSlave.cpp: EA 4-Way Play device. (Slave; Port 1)                   *
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

// Devices.
#include "Io3Button.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

Io4WPSlave::Io4WPSlave(const IoBase *other)
	: IoBase(other)
{
	// TODO: Copy slave devices from another 4WP Slave device.
	// TODO: Delete these devices when finished.
	m_current = 0;	// TODO: Update based on m_lastData.
	
	// Create I/O devices.
	// TODO: Make these configurable!
	m_devs[0] = new Io3Button();
	m_devs[1] = new IoBase();
	m_devs[2] = new IoBase();
	m_devs[3] = new IoBase();
}

Io4WPSlave::~Io4WPSlave()
{
	// Delete the I/O devices.
	for (int i = 0; i < 4; i++)
	{
		delete m_devs[i];
		m_devs[i] = NULL;
	}
}

/**
 * reset(): Reset function.
 * Called when the system is reset.
 */
void Io4WPSlave::reset()
{
	IoBase::reset();
	m_current = 0;	// TODO: Update based on m_lastData.
	
	m_devs[0]->reset();
	m_devs[1]->reset();
	m_devs[2]->reset();
	m_devs[3]->reset();
}


/**
 * writeCtrl(): Set the I/O tristate value.
 * TODO: Combine with writeData().
 * @param ctrl I/O tristate value.
 */
void Io4WPSlave::writeCtrl(uint8_t ctrl)
{
	m_ctrl = ctrl;
	updateSelectLine();
	
	// Update the current slave port.
	// (TODO: Do we need to check >= 0?)
	if (m_current >= 0 && m_current < 4)
		m_devs[m_current]->writeCtrl(ctrl);
}


/**
 * writeData(): Write data to the controller.
 * * TODO: Combine with writeCtrl().
 * @param data Data to the controller.
 */
void Io4WPSlave::writeData(uint8_t data)
{
	m_lastData = data;
	updateSelectLine();
	
	// Update the current slave port.
	// (TODO: Do we need to check >= 0?)
	if (m_current >= 0 && m_current < 4)
		m_devs[m_current]->writeData(data);
}

/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t Io4WPSlave::readData(void)
{
	if (m_current >= 4)
		return 0x70;	// Multitap detection: TH2 = 1
	
	// Read data from the selected gamepad.
	return m_devs[m_current]->readData();
}


/**
 * update(): I/O device update function.
 */
void Io4WPSlave::update(void)
{
	// Update the slave devices.
	m_devs[0]->update();
	m_devs[1]->update();
	m_devs[2]->update();
	m_devs[3]->update();
}


/**
 * setSlaveController(): Set the controller number.
 * @param id Controller number.
 */
void Io4WPSlave::setSlaveController(int id)
{
	m_current = id;
	if (id >= 0 && id < 4)
	{
		m_devs[id]->writeCtrl(m_ctrl);
		m_devs[id]->writeData(m_ctrl);
	}
}

}
