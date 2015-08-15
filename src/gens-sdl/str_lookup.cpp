/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * str_lookup.cpp: String lookups for some enumerations.                   *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "str_lookup.hpp"

// LibGens includes.
#include "libgens/Rom.hpp"
using LibGens::Rom;

namespace GensSdl {

/**
 * Convert a ROM format ID to a string.
 * @param romFormat ROM format ID.
 * @return String.
 */
const char *romFormatToString(Rom::RomFormat romFormat)
{
	const char *rom_format;
	switch (romFormat) {
		case Rom::RFMT_UNKNOWN:
		default:
			rom_format = "Unknown";
			break;
		case Rom::RFMT_BINARY:
			rom_format = "Binary";
			break;
		case Rom::RFMT_SMD:
			rom_format = "Super Magic Drive";
			break;
		case Rom::RFMT_SMD_SPLIT:
			rom_format = "Super Magic Drive (split)";
			break;
		case Rom::RFMT_MGD:
			rom_format = "Multi Game Doctor";
			break;
		case Rom::RFMT_CD_CUE:
			rom_format = "CUE sheet";
			break;
		case Rom::RFMT_CD_ISO_2048:
			rom_format = "ISO-9660(2048)";
			break;
		case Rom::RFMT_CD_ISO_2352:
			rom_format = "ISO-9660(2352)";
			break;
		case Rom::RFMT_CD_BIN_2048:
			rom_format = "BIN/CUE(2048)";
			break;
		case Rom::RFMT_CD_BIN_2352:
			rom_format = "BIN/CUE(2352)";
			break;
	}
	return rom_format;
}

/**
 * Convert a ROM system ID to a string.
 * @param sysId ROM system ID.
 * @return String.
 */
const char *sysIdToString(Rom::MDP_SYSTEM_ID sysId)
{
	const char *rom_sysId;
	switch (sysId) {
		case Rom::MDP_SYSTEM_UNKNOWN:
		default:
			rom_sysId = "Unknown";
			break;
		case Rom::MDP_SYSTEM_MD:
			rom_sysId = "Mega Drive";
			break;
		case Rom::MDP_SYSTEM_MCD:
			rom_sysId = "Mega CD";
			break;
		case Rom::MDP_SYSTEM_32X:
			rom_sysId = "32X";
			break;
		case Rom::MDP_SYSTEM_MCD32X:
			rom_sysId = "Mega CD 32X";
			break;
		case Rom::MDP_SYSTEM_SMS:
			rom_sysId = "Sega Master System";
			break;
		case Rom::MDP_SYSTEM_GG:
			rom_sysId = "Game Gear";
			break;
		case Rom::MDP_SYSTEM_SG1000:
			rom_sysId = "SG-1000";
			break;
		case Rom::MDP_SYSTEM_PICO:
			rom_sysId = "Pico";
			break;
	}
	return rom_sysId;
}

}
