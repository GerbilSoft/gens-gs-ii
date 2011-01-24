/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuContext.cpp: Emulation context base class.                           *
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

#include "EmuContext.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

namespace LibGens
{

// Reference counter.
// We're only allowing one emulation context at the moment.
int EmuContext::ms_RefCount = 0;

// Controllers.
// TODO: Figure out a better place to put these!
// TODO: Make these non-static!
IoBase *EmuContext::m_port1;		// Player 1.
IoBase *EmuContext::m_port2;		// Player 2.
IoBase *EmuContext::m_portE;		// EXT port.

EmuContext::EmuContext(Rom *rom)
{
	ms_RefCount++;
	assert(ms_RefCount == 1);
	
	// Initialize variables.
	m_rom = rom;
	
	// Create base I/O devices that do nothing.
	// TODO: For now, we'll treat them as static.
	if (!m_port1)
		m_port1 = new IoBase();
	if (!m_port2)
		m_port2 = new IoBase();
	if (!m_portE)
		m_portE = new IoBase();
}

EmuContext::~EmuContext()
{
	ms_RefCount--;
	assert(ms_RefCount == 0);
	
	// Delete the I/O devices.
	// TODO: Don't do this right now.
#if 0
	delete m_port1;
	m_port1 = NULL;
	delete m_port2;
	m_port2 = NULL;
	delete m_portE;
	m_portE = NULL;
#endif
}

}
