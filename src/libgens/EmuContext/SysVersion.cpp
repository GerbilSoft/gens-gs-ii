/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SysVersion.cpp: MD System Version Code Register.                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2016 by David Korth.                                 *
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

#include "SysVersion.hpp"

#include "macros/common.h"

// C includes. (C++ namespace)
#include <cassert>

namespace LibGens {

// Valid region code auto-detect orders.
const uint16_t SysVersion::RegionCodeOrder_tbl[24] = {
	0x4812, 0x4821, 0x4182, 0x4128, 0x4281, 0x4218, 
	0x8412, 0x8421, 0x8124, 0x8142, 0x8241, 0x8214,
	0x1482, 0x1428, 0x1824, 0x1842, 0x1248, 0x1284,
	0x2481, 0x2418,	0x2814, 0x2841, 0x2148, 0x2184
};

/** Region code auto-detection. **/

/**
 * Detect the region code for an MD ROM.
 * @param mdHexRegionCode MD hex region code. (0x0-0xF)
 * @param regionCodeOrder Region code order.
 * @return RegionCode_t. (Returns REGION_AUTO if a parameter is invalid.)
 */
SysVersion::RegionCode_t SysVersion::DetectRegion(int mdHexRegionCode, uint16_t regionCodeOrder)
{
	assert(mdHexRegionCode >= 0x0 && mdHexRegionCode <= 0xF);
	if (mdHexRegionCode < 0x0 || mdHexRegionCode > 0xF)
		return REGION_AUTO;

	assert(IsRegionCodeOrderValid(regionCodeOrder));
	if (!IsRegionCodeOrderValid(regionCodeOrder))
		return REGION_AUTO;

	// Attempt to auto-detect the region from the ROM image.
	int regionMatch = 0;
	int orderTmp = regionCodeOrder;
	for (int i = 0; i < 4; i++, orderTmp <<= 4) {
		int orderN = ((orderTmp >> 12) & 0xF);
		if (mdHexRegionCode & orderN) {
			// Found a match.
			regionMatch = orderN;
			break;
		}
	}

	if (regionMatch == 0) {
		// No region matched.
		// Use the highest-priority region.
		regionMatch = ((regionCodeOrder >> 12) & 0xF);
	}

	switch (regionMatch & 0xF) {
		default:
		case 0x4:	return LibGens::SysVersion::REGION_US_NTSC;
		case 0x8:	return LibGens::SysVersion::REGION_EU_PAL;
		case 0x1:	return LibGens::SysVersion::REGION_JP_NTSC;
		case 0x2:	return LibGens::SysVersion::REGION_ASIA_PAL;
	}
}

/**
 * Is a region code auto-detect order valid?
 * @param regionCodeOrder Region code order.
 * @return True if valid; false if not.
 */
bool SysVersion::IsRegionCodeOrderValid(uint16_t regionCodeOrder)
{
	for (int i = 0; i < ARRAY_SIZE(RegionCodeOrder_tbl); i++) {
		if (regionCodeOrder == RegionCodeOrder_tbl[i])
			return true;
	}

	return false;
}

}
