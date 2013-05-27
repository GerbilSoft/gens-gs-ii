/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SysVersion.hpp: MD System Version Code Register.                        *
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

#ifndef __LIBGENS_MD_SYSVERSION_HPP__
#define __LIBGENS_MD_SYSVERSION_HPP__

/**
 * System version register: $A10001
 * [MODE VMOD DISK RSV VER3 VER2 VER1 VER0]
 * MODE: Region. (0 == East; 1 == West)
 * VMOD: Video.  (0 == NTSC; 1 == PAL)
 * DISK: 0 == FDD/MCD connected; 1 == FDD/MCD not connected
 * RSV: Reserved. (assume 0)
 * VER: Version register. (currently 0)
 */

/**
 * TODO:
 * - Country code autodetection ordering.
 * - Set VER to 1 once TMSS support is added, if TMSS is enabled.
 */

// C includes.
#include <stdint.h>

namespace LibGens
{

class SysVersion
{
	public:
		enum RegionCode_t {
			REGION_AUTO	= -1,	// Auto-Detect
			REGION_JP_NTSC	= 0,	// Japan (NTSC)
			REGION_ASIA_PAL	= 1,	// Asia (PAL)
			REGION_US_NTSC	= 2,	// USA (NTSC)
			REGION_EU_PAL	= 3	// Europe (PAL)
		};

		SysVersion();
		SysVersion(RegionCode_t initRegion);

		/**
		 * Read the register contents.
		 * @return Register contents.
		 */
		uint8_t readData(void) const;

		RegionCode_t region(void) const;
		void setRegion(RegionCode_t newRegion);

		/**
		 * Region code: Convenience functions.
		 */
		bool isNtsc(void) const;
		bool isPal(void) const;
		bool isEast(void) const;
		bool isWest(void) const;

		/**
		 * /DISK line.
		 */
		bool hasDisk(void) const;
		void setDisk(bool newDisk);

		/**
		 * Version field.
		 */
		uint8_t version(void) const;
		void setVersion(uint8_t newVersion);

	private:
		RegionCode_t m_region;
		bool m_disk;		// True if MCD is connected.
		uint8_t m_version;	// 0x0 if no TMSS; 0x1 if TMSS is present.
};

inline SysVersion::SysVersion()
	: m_region(REGION_US_NTSC)
	, m_disk(false)
	, m_version(0)
{ }

inline SysVersion::SysVersion(RegionCode_t initRegion)
	: m_region(initRegion)
	, m_disk(false)
	, m_version(0)
{ }

inline SysVersion::RegionCode_t SysVersion::region(void) const
	{ return m_region; }
inline void SysVersion::setRegion(RegionCode_t newRegion)
	{ m_region = (RegionCode_t)((int)newRegion & 0x03); }

/**
 * Region code: Convenience functions.
 */
inline bool SysVersion::isNtsc(void) const
	{ return !((int)m_region & 1); }
inline bool SysVersion::isPal(void) const
	{ return ((int)m_region & 1); }
inline bool SysVersion::isEast(void) const
	{ return !((int)m_region & 2); }
inline bool SysVersion::isWest(void) const
	{ return !!((int)m_region & 2); }

/**
 * /DISK line.
 */
inline bool SysVersion::hasDisk(void) const
	{ return m_disk; }
inline void SysVersion::setDisk(bool newDisk)
	{ m_disk = newDisk; }

/**
 * Version field.
 */
inline uint8_t SysVersion::version(void) const
	{ return (m_version & 0xF); }
inline void SysVersion::setVersion(uint8_t newVersion)
	{ m_version = (newVersion & 0xF); }

/**
 * Read the register contents.
 * @return Register contents.
 */
inline uint8_t SysVersion::readData(void) const
{
	// NOTE: m_disk uses active-high logic.
	// The version register uses active-low logic for /DISK.
	return (((uint8_t)m_region) << 6 | (!m_disk << 5) | (m_version & 0xF));
}

}

#endif /* __LIBGENS_MD_SYSVERSION_HPP__ */
