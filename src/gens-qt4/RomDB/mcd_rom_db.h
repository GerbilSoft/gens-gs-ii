/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * mcd_rom_db.h: Sega CD Boot ROM database.                                *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth                                  *
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

#ifndef __GENSQT4_MCD_ROM_DB_H__
#define __GENSQT4_MCD_ROM_DB_H__

// C includes.
#include <stdint.h>

// Common macros. (Used for utf8_str.)
#include "libgens/macros/common.h"

#ifdef __cplusplus
extern "C" {
#endif

// MCD_ROM_UNKNOWN: Unknown ROM.
#define MCD_ROM_UNKNOWN -1

/**
 * ROM fixup data.
 * lg_mcd_rom_InitSP: Initial SP. (0xFFFFFD00)
 * lg_mcd_rom_InitHINT: Initial HINT vector. (0xFFFFFD0C)
 */
extern const uint8_t mcd_rom_InitSP[4];
extern const uint8_t mcd_rom_InitHINT[4];

// Region codes.
// Matches the country code found in later MD games.
// TODO: Move this somewhere else and make it not MCD-specific.
typedef enum
{
	// Bit flags.
	MCD_REGION_JAPAN	= 0x01,
	MCD_REGION_ASIA		= 0x02,
	MCD_REGION_USA		= 0x04,
	MCD_REGION_EUROPE	= 0x08,
	
	MCD_REGION_INVALID	= -1
} MCD_RegionCode_t;

// ROM status.
typedef enum
{
	RomStatus_Recommended	= 0x00,
	RomStatus_Supported	= 0x01,
	RomStatus_Unsupported	= 0x02,
	RomStatus_Broken	= 0x03,
	
	RomStatus_MAX
} MCD_RomStatus_t;

/**
 * Find a Sega CD Boot ROM by CRC32.
 * @param rom_crc32 Sega CD Boot ROM CRC32.
 * @return ROM ID, or -1 if not found.
 */
int mcd_rom_FindByCRC32(uint32_t rom_crc32);

/**
 * Get a Boot ROM's description.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM description, or NULL if the ID is invalid.
 */
const utf8_str *mcd_rom_GetDescription(int rom_id);

/**
 * Get a Boot ROM's notes.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM notes, or NULL if the ID is invalid.
 */
const utf8_str *mcd_rom_GetNotes(int rom_id);

/**
 * Get a Boot ROM's region code.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM region code, or MCD_REGION_INVALID if the ID is invalid.
 */
int mcd_rom_GetRegion(int rom_id);

/**
 * Get a Boot ROM's primary region code.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM primary region code, or MCD_REGION_INVALID if the ID is invalid.
 */
int mcd_rom_GetPrimaryRegion(int rom_id);

/**
 * Get a Boot ROM's support status.
 * @param rom_id Boot ROM ID.
 * @return ROM support status, or RomStatus_MAX if the ID is invalid.
 */
MCD_RomStatus_t mcd_rom_GetSupportStatus(int rom_id);

/**
 * Get a string describing a primary region code.
 * @param region_code Primary region code.
 * @return Region code string, or NULL if the region code is invalid.
 */
const utf8_str *mcd_rom_GetRegionCodeString(int region_code);

#ifdef __cplusplus
}
#endif

#endif /* __GENSQT4_MCD_ROM_DB_H__ */
