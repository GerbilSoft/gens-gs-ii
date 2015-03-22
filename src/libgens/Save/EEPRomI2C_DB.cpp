/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRomI2C_DB.hpp: I2C Serial EEPROM handler. (ROM database)             *
 *                                                                         *
 * Copyright (C) 2007, 2008, 2009  Eke-Eke (Genesis Plus GCN/Wii port)     *
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

/**
 * Based on cart_hw/eeprom.c from Genesis Plus GX.
 */
#include "EEPRomI2C.hpp"
#include "EEPRomI2C_p.hpp"

// ARRAY_SIZE(x)
#include "macros/common.h"

// C includes. (C++ namespace)
#include <cstring>

namespace LibGens {

/** ROM database. **/
const EEPRomI2CPrivate::GameEEPRomInfo_t EEPRomI2CPrivate::rom_db[27] =
{
	// TODO: Port other EEPROMs.

	/** ACCLAIM mappers **/

	/* 24C02 (old mapper) */
	/* NBA Jam (UE) */
	{"T-081326"   , 0,      {0x200001, 0x200001, 0x200001, 0, 1, 1}, {EEPRomI2C::EPR_MODE2, 0, 0xFF, 0x03}},
	/* NBA Jam (J) */
	{"T-81033"    , 0,      {0x200001, 0x200001, 0x200001, 0, 1, 1}, {EEPRomI2C::EPR_MODE2, 0, 0xFF, 0x03}},

	/* 24C02 */
	/* NFL Quarterback Club */
	{"T-081276"   , 0,      {0x200001, 0x200001, 0x200000, 0, 0, 0}, {EEPRomI2C::EPR_MODE2, 0, 0xFF, 0x03}},

	/* 24C04 */
	/* NBA Jam TE */
	{"T-81406"    , 0,      {0x200001, 0x200001, 0x200000, 0, 0, 0}, {EEPRomI2C::EPR_MODE2, 0, 0x1FF, 0x03}},

	/* 24C16 */
	/* NFL Quarterback Club '96 */
	{"T-081586"   , 0,      {0x200001, 0x200001, 0x200000, 0, 0, 0}, {EEPRomI2C::EPR_MODE2, 0, 0x7FF, 0x07}},

	/* 24C65 */
	// NOTE: Emulated as 24C64; same chip but without "smart" features.
	// TODO: Needs MODE 3.
	/* College Slam */
	//{"T-81576"    , 0,      {0x200001, 0x200001, 0x200000, 0, 0, 0}, {EEPRomI2C::EPR_MODE3, 0, 0x1FFF, 0x07}},
	/* Frank Thomas Big Hurt Baseball */
	//{"T-81476"    , 0,      {0x200001, 0x200001, 0x200000, 0, 0, 0}, {EEPRomI2C::EPR_MODE3, 0, 0x1FFF, 0x07}},

	/** EA mapper (X24C01 only) **/
	/* Rings of Power */
	{"T-50176"    , 0,      {0x200001, 0x200001, 0x200001, 7, 7, 6}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* NHLPA Hockey 93 */
	{"T-50396"    , 0,      {0x200001, 0x200001, 0x200001, 7, 7, 6}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* John Madden Football 93 */
	{"T-50446"    , 0,      {0x200001, 0x200001, 0x200001, 7, 7, 6}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* John Madden Football 93 (Championship Ed.) */
	{"T-50516"    , 0,      {0x200001, 0x200001, 0x200001, 7, 7, 6}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Bill Walsh College Football */
	{"T-50606"    , 0,      {0x200001, 0x200001, 0x200001, 7, 7, 6}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},

	/** SEGA mapper (X24C01) **/
	/* Megaman - The Wily Wars */
	{"T-12046"    , 0xAD23, {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Rockman Mega World (J) [A] */
	{"T-12053"    , 0xEA80, {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Evander 'Real Deal' Holyfield's Boxing */
	{"MK-1215"    , 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Greatest Heavyweights of the Ring (U) */
	{"MK-1228"    , 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Greatest Heavyweights of the Ring (J) */
	{"G-5538"     , 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Greatest Heavyweights of the Ring (E) */
	{"PR-1993"    , 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Wonderboy in Monster World */
	{"G-4060"     , 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Sports Talk Baseball */
	{"00001211-00", 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Honoo no Toukyuuji Dodge Danpei */
	{"00004076-00", 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Ninja Burai Densetsu */
	{"G-4524"     , 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},
	/* Game Toshokan */
	{"00054503-00", 0,      {0x200001, 0x200001, 0x200001, 0, 0, 1}, {EEPRomI2C::EPR_MODE1, 0, 0x7F, 0x03}},

	/** CODEMASTERS mapper **/
	
	/* 24C08 */
	/* Brian Lara Cricket */ // TODO: Verify page mask.
	{"T-120106",    0,      {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE2, 0, 0x3FF, 0x0F}},
	/* Micro Machines Military */
	{"00000000-00", 0x168B, {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE2, 0, 0x3FF, 0x0F}},
	/* Micro Machines Military (Bad?)*/
	{"00000000-00", 0xCEE0, {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE2, 0, 0x3FF, 0x0F}},

	/* 24C16 */
	/* Micro Machines 2 - Turbo Tournament (E) */
	{"T-120096"   , 0,      {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE2, 0, 0x7FF, 0x0F}},
	/* Micro Machines Turbo Tournament 96 */
	{"00000000-00", 0x165E, {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE2, 0, 0x7FF, 0x0F}},
	/* Micro Machines Turbo Tournament 96 (Bad?)*/
	{"00000000-00", 0x2C41, {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE2, 0, 0x7FF, 0x0F}},

	/* 24C65 */
	// NOTE: Emulated as 24C64; same chip but without "smart" features.
	// TODO: Needs MODE 3.
	// TODO: Verify page mask.
	/* Brian Lara Cricket 96, Shane Warne Cricket */
	//{"T-120146-50", 0,      {0x300000, 0x380001, 0x300000, 0, 7, 1}, {EEPRomI2C::EPR_MODE3, 0, 0x1FFF, 0x07}},
};

/**
 * Detect the EEPRom type used by the specified ROM.
 * @param serial Serial number. (NOTE: This does NOT include the "GM " prefix!)
 * @param serial_len Length of the serial number string.
 * @param checksum Checksum.
 * @return EEPRom type, or -1 if this ROM isn't known.
 */
int EEPRomI2C::DetectEEPRomType(const char *serial, size_t serial_len, uint16_t checksum)
{
	// Scan the database for potential matches.
	for (int i = 0; i < ARRAY_SIZE(EEPRomI2CPrivate::rom_db); i++) {
		// TODO: Figure out how to get rid of the strlen().
		size_t dbSerial_len = strlen(EEPRomI2CPrivate::rom_db[i].game_id);
		if (dbSerial_len > serial_len) {
			// Serial number in the database is longer than
			// the given serial number data.
			continue;
		}

		if (!memcmp(serial, EEPRomI2CPrivate::rom_db[i].game_id, dbSerial_len)) {
			// Serial number matches.
			if (EEPRomI2CPrivate::rom_db[i].checksum == 0) {
				// No checksum verification required.
				return i;
			}

			// Checksum verification is required.
			if (checksum == EEPRomI2CPrivate::rom_db[i].checksum) {
				// Checksum matches.
				return i;
			}

			// Checksum doesn't match. Wrong ROM.
		}
	}

	// The ROM wasn't found in the database.
	return -1;
}

}
