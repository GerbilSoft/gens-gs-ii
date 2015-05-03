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
	// High bits are always 001101.
	// Also, FIFO is empty on startup.
	m_status = 0x3400 | VDP_STATUS_EMPTY;
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
	// Save the original status register.
	// The original status register will be returned to the CPU.
	const uint16_t status_orig = m_status;
	
	// Toggle the VDP FIFO flags.
	// This is needed for some games that expect the FIFO
	// state to change instead of remaining perpetually
	// empty or half-full.
	// TODO: Implement the FIFO.
	m_status ^= (VDP_STATUS_FULL | VDP_STATUS_EMPTY);
	
	// Mask the SOVR ("Sprite Overflow") and C ("Collision between non-zero pixels in two sprites") bits.
	m_status &= ~(VDP_STATUS_SOVR | VDP_STATUS_COLLISION);
	
	// Return the original VDP status.
	return status_orig;
}

}
