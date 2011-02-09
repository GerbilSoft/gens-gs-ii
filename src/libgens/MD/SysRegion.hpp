/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SysRegion.hpp: Region code management.                                  *
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

#ifndef __LIBGENS_MD_SYSREGION_HPP__
#define __LIBGENS_MD_SYSREGION_HPP__

/**
 * TODO:
 * - Country code autodetection ordering.
 * - Maybe add other MD version register bits?
 */

namespace LibGens
{

class SysRegion
{
	public:
		enum RegionCode
		{
			REGION_JP_NTSC	= 0,	// Japan (NTSC)
			REGION_ASIA_PAL	= 1,	// Asia (PAL)
			REGION_US_NTSC	= 2,	// USA (NTSC)
			REGION_EU_PAL	= 3	// Europe (PAL)
		};
		
		SysRegion()
			{ m_region = REGION_US_NTSC; }
		SysRegion(RegionCode initRegion)
			{ m_region = initRegion; }
		
		inline RegionCode region(void) const
			{ return m_region; }
		void setRegion(RegionCode newRegion)
			{ m_region = (RegionCode)((int)newRegion & 0x03); }
		
		// Convenience functions.
		inline bool isNtsc(void) const
			{ return !((int)m_region & 1); }
		inline bool isPal(void) const
			{ return ((int)m_region & 1); }
		inline bool isEast(void) const
			{ return !((int)m_region & 2); }
		inline bool isWest(void) const
			{ return !!((int)m_region & 2); }
	
	private:
		RegionCode m_region;
};

}

#endif /* __LIBGENS_MD_SYSREGION_HPP__ */
