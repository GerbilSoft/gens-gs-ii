/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * mcd_rom_db.c: Sega CD Boot ROM database.                                *
 *                                                                         *
 * Copyright (c) 2011 by David Korth                                       *
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

#ifndef __LIBGENS_MCD_ROM_DB_H__
#define __LIBGENS_MCD_ROM_DB_H__

// C includes.
#include <stdint.h>

// Common macros. (Used for utf8_str.)
#include "../macros/common.h"

#ifdef __cplusplus
extern "C" {
#endif

// MCD_ROM_UNKNOWN: Unknown ROM.
#define MCD_ROM_UNKNOWN -1

// Region codes.
typedef enum
{
	// Bit flags.
	Region_East	= 0x00,
	Region_West	= 0x01,
	Region_NTSC	= 0x00,
	Region_PAL	= 0x02,
	
	// Countries.
	Region_Japan_NTSC	= 0x00,
	Region_USA		= 0x01,
	Region_Japan_PAL	= 0x02,
	Region_Europe		= 0x03,
	
	Region_MAX		= -1
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
 * lg_mcd_rom_FindByCRC32(): Find a Sega CD Boot ROM by CRC32.
 * @param rom_crc32 Sega CD Boot ROM CRC32.
 * @return ROM ID, or -1 if not found.
 */
int lg_mcd_rom_FindByCRC32(uint32_t rom_crc32);

/**
 * lg_mcd_rom_GetDescription(): Get a Boot ROM's description.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM description, or NULL if the ID is invalid.
 */
const utf8_str *lg_mcd_rom_GetDescription(int rom_id);

/**
 * lg_mcd_rom_GetDescription(): Get a Boot ROM's notes.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM notes, or NULL if the ID is invalid.
 */
const utf8_str *lg_mcd_rom_GetNotes(int rom_id);

/**
 * lg_mcd_rom_GetRegion(): Get a Boot ROM's region.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM ID, or Region_MAX if the ID is invalid.
 */
MCD_RegionCode_t lg_mcd_rom_GetRegion(int rom_id);

/**
 * lg_mcd_rom_GetSupportStatus(): Get a Boot ROM's support status.
 * @param rom_id Boot ROM ID.
 * @return ROM support status, or RomStatus_MAX if the ID is invalid.
 */
MCD_RomStatus_t lg_mcd_rom_GetSupportStatus(int rom_id);

/**
 * lg_mcd_rom_GetRegionCodeString(): Get a string describing a region code.
 * @param region_code Region code.
 * @return Region code string, or NULL if the region code is invalid.
 */
const utf8_str *lg_mcd_rom_GetRegionCodeString(MCD_RegionCode_t region_code);

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_MCD_ROM_DB_H__ */
