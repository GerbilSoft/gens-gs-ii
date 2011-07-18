/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpStatus.cpp: VDP status register.                                     *
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

#include "VdpStatus.hpp"

namespace LibGens
{

VdpStatus::VdpStatus(bool isPal)
{
	// Initialize the status register.
	reset(isPal);
}

/**
 * reset(): Reset the VDP status register.
 * @param isPal If true, sets the PAL bit.
 */
void VdpStatus::reset(bool isPal)
{
	m_status = VDP_STATUS_EMPTY;
	if (isPal)
		m_status |= VDP_STATUS_PAL;
}

/**
 * read(): Read the VDP status register.
 * This function is for $C00004 emulation.
 * NOTE: This function modifies some of the status register bits!
 * @return VDP status register.
 */
uint16_t VdpStatus::read(void)
{
	// Toggle the upper 8 bits of VDP_Status. (TODO: Is this correct?)
	m_status ^= 0xFF00;
	
	// Mask the SOVR ("Sprite Overflow") and C ("Collision between non-zero pixels in two sprites") bits.
	// TODO: Should these be masked? This might be why some games are broken...
	m_status &= ~(VDP_STATUS_SOVR | VDP_STATUS_COLLISION);
	
	// Check if we're currently in VBlank.
	if (!(m_status & VDP_STATUS_VBLANK))
	{
		// Not in VBlank. Mask the F bit. ("Vertical Interrupt Happened")
		m_status &= ~VDP_STATUS_F;
	}
	
	// Return the VDP status.
	return m_status;
}

}
