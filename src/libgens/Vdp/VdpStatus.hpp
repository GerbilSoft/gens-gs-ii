/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpStatus.hpp: VDP status register.                                     *
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

#ifndef __LIBGENS_VDP_VDPSTATUS_HPP__
#define __LIBGENS_VDP_VDPSTATUS_HPP__

#include <stdint.h>

namespace LibGens
{

class VdpStatus
{
	public:
		VdpStatus(bool isPal = false);
		
		enum StatusBits
		{
			VDP_STATUS_PAL		= 0x0001,	// 0 = NTSC; 1 = PAL
			VDP_STATUS_DMA		= 0x0002,	// 1 == DMA BUSY
			VDP_STATUS_HBLANK	= 0x0004,	// 1 == in HBlank
			VDP_STATUS_VBLANK	= 0x0008,	// 1 == in VBlank
			VDP_STATUS_ODD		= 0x0010,	// 0 == even frame; 1 == odd frame (interlaced only)
			VDP_STATUS_COLLISION	= 0x0020,	// 1 == collision between non-zero sprite pixels
			VDP_STATUS_SOVR		= 0x0040,	// 1 == sprite overflow occurred
			VDP_STATUS_F		= 0x0080,	// 1 == VINT happened
			VDP_STATUS_FULL		= 0x0100,	// 1 == WRITE FIFO FULL (TODO: Not properly emulated!)
			VDP_STATUS_EMPTY	= 0x0200,	// 1 == WRITE FIFO EMPTY (TODO: Not properly emulated!)
		};
		
		/**
		 * reset(): Reset the VDP status register.
		 * @param isPal If true, sets the PAL bit.
		 */
		void reset(bool isPal = false);
		
		/**
		 * read(): Read the VDP status register.
		 * This function is for $C00004 emulation.
		 * NOTE: This function modifies some of the status register bits!
		 * @return VDP status register.
		 */
		uint16_t read(void);
		
		/**
		 * read_raw(): Read the raw VDP status register.
		 * This function is for savestates.
		 * @return VDP status register. (raw)
		 */
		uint16_t read_raw(void) const;
		
		/**
		 * write_raw(): Write the raw VDP status register.
		 * This function is for savestates.
		 * @param status New VDP status register. (raw)
		 */
		void write_raw(uint16_t status);
		
		/** Convenience functions. **/
		
		bool isPal(void) const;
		bool isNtsc(void) const;
		
		bool isOddFrame(void) const;
		void toggleOddFrame(void);
		void clearOddFrame(void);
		
		void setRegion(bool isPal);
		void setHBlank(bool HBlank);
		void setVBlank(bool VBlank);
		void setVIntHappened(bool VIntHappened);
		void setDMABusy(bool DMABusy);
		void setCollision(bool collision);
		void setSOvr(bool sovr);
	
	private:
		uint16_t m_status;
};

// Raw read/write functions for savestates.
inline uint16_t VdpStatus::read_raw(void) const
	{ return m_status; }
inline void VdpStatus::write_raw(uint16_t status)
	{ m_status = status; }

/** VDP status register functions. **/

// PAL/NTSC: Bit 0. 0 == NTSC; 1 == PAL.
inline bool VdpStatus::isPal(void) const
	{ return (m_status & VDP_STATUS_PAL); }
inline bool VdpStatus::isNtsc(void) const
	{ return (!(m_status & VDP_STATUS_PAL)); }

inline bool VdpStatus::isOddFrame(void) const
	{ return (!!(m_status & VDP_STATUS_ODD)); }
inline void VdpStatus::toggleOddFrame(void)
	{ m_status ^= VDP_STATUS_ODD; }
inline void VdpStatus::clearOddFrame(void)
	{ m_status &= ~VDP_STATUS_ODD; }

inline void VdpStatus::setRegion(bool isPal)
{
	// Bit 0: 0 == NTSC; 1 == PAL
	if (isPal)
		m_status |= VDP_STATUS_PAL;
	else
		m_status &= ~VDP_STATUS_PAL;
}

inline void VdpStatus::setHBlank(bool HBlank)
{
	// Bit 3: 0 == not in HBlank; 1 == in HBlank
	if (HBlank)
		m_status |= VDP_STATUS_HBLANK;
	else
		m_status &= ~VDP_STATUS_HBLANK;
}

inline void VdpStatus::setVBlank(bool VBlank)
{
	// Bit 4: 0 == not in HBlank; 1 == in HBlank
	if (VBlank)
		m_status |= VDP_STATUS_VBLANK;
	else
		m_status &= ~VDP_STATUS_VBLANK;
}

inline void VdpStatus::setVIntHappened(bool VIntHappened)
{
	// Bit 7: 0 == VInt processed; 1 == new VInt happened
	if (VIntHappened)
		m_status |= VDP_STATUS_F;
	else
		m_status &= ~VDP_STATUS_F;
}

inline void VdpStatus::setDMABusy(bool DMABusy)
{
	// Bit 1: 0 == not processing DMA; 1 == DMA busy
	if (DMABusy)
		m_status |= VDP_STATUS_DMA;
	else
		m_status &= ~VDP_STATUS_DMA;
}

inline void VdpStatus::setCollision(bool collision)
{
	// Bit 5: 0 == no sprite collision; 1 == sprite collision
	if (collision)
		m_status |= VDP_STATUS_COLLISION;
	else
		m_status &= ~VDP_STATUS_COLLISION;
}

inline void VdpStatus::setSOvr(bool sovr)
{
	// Bit 6: 0 == no sprite overflow; 1 == sprite overflow
	if (sovr)
		m_status |= VDP_STATUS_SOVR;
	else
		m_status &= ~VDP_STATUS_SOVR;
}

}

#endif /* __LIBGENS_VDP_VDPSTATUS_HPP__ */
