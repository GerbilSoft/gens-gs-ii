/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRom_File.cpp: Serial EEPROM handler. (ROM database)                  *
 *                                                                         *
 * Copyright (C) 2007, 2008, 2009  Eke-Eke (Genesis Plus GCN/Wii port)     *
 * Copyright (c) 2010-2011 by David Korth.                                 *
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

#include "EEPRom.hpp"

namespace LibGens
{

/**
 * ms_Database[]: EEPROM information database.
 */
const EEPRom::GameEEPRomInfo EEPRom::ms_Database[29] =
{
	/** ACCLAIM mappers **/
	
	/* 24C02 (old mapper) */
	{"T-081326"   , 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200001, 0, 1, 1}},   /* NBA Jam (UE) */
	{"T-81033"    , 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200001, 0, 1, 1}},   /* NBA Jam (J) */
	
	/* 24C02 */
	{"T-81406"    , 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NBA Jam TE */
	{"T-081276"   , 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NFL Quarterback Club */
	
	/* 24C16 */
	{"T-081586"   , 0,      {8,  0x7FF,  0x7FF,  0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NFL Quarterback Club '96 */
	
	/* 24C65 */
	{"T-81576"    , 0,      {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* College Slam */
	{"T-81476"    , 0,      {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* Frank Thomas Big Hurt Baseball */
	
	/** EA mapper (24C01 only) **/
	{"T-50176"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Rings of Power */
	{"T-50396"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* NHLPA Hockey 93 */
	{"T-50446"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* John Madden Football 93 */
	{"T-50516"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* John Madden Football 93 (Championship Ed.) */
	{"T-50606"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Bill Walsh College Football */
	
	/** SEGA mapper (24C01 only) **/
	{"T-12046"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Megaman - The Wily Wars */
	{"T-12053"    , 0xEA80, {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Rockman Mega World (J) [A] */
	{"MK-1215"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Evander 'Real Deal' Holyfield's Boxing */
	{"MK-1228"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (U) */
	{"G-5538"     , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (J) */
	{"PR-1993"    , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (E) */
	{"G-4060"     , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Wonderboy in Monster World */
	{"00001211-00", 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Sports Talk Baseball */
	{"00004076-00", 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Honoo no Toukyuuji Dodge Danpei */
	{"G-4524"     , 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Ninja Burai Densetsu */
	
	/** CODEMASTERS mapper **/
	
	/* 24C01 */
	{"T-120106",    0,      {7,  0x7F,   0x7F,   0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Brian Lara Cricket */
	
	/* 24C08 */
	{"T-120096"   , 0,      {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines 2 - Turbo Tournament (E) */
	{"00000000-00", 0x168B, {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Military */
	{"00000000-00", 0xCEE0, {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Military (Bad)*/
	
	/* 24C16 */
	{"00000000-00", 0x165E, {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Turbo Tournament 96 */
	{"00000000-00", 0x2C41, {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Turbo Tournament 96 (Bad)*/
	
	/* 24C65 */
	{"T-120146-50", 0,      {16, 0x1FFF, 0x1FFF, 0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Brian Lara Cricket 96, Shane Warne Cricket */
};


/**
 * DetectEEPRomType(): Detect the EEPRom type used by the specified ROM.
 * @param serial Serial number. (NOTE: This does NOT include the "GM " prefix!)
 * @param serial_len Length of the serial number string.
 * @param checksum Checksum.
 * @return EEPRom type, or -1 if this ROM isn't known.
 */
int EEPRom::DetectEEPRomType(const char *serial, size_t serial_len, uint16_t checksum)
{
	// Scan the database for potential matches.
	for (size_t i = 0; i < (sizeof(ms_Database)/sizeof(ms_Database[0])); i++)
	{
		// TODO: Figure out how to get rid of the strlen().
		size_t dbSerial_len = strlen(ms_Database[i].game_id);
		if (dbSerial_len > serial_len)
		{
			// Serial number in the database is longer than
			// the given serial number data.
			continue;
		}
		
		if (!memcmp(serial, ms_Database[i].game_id, dbSerial_len))
		{
			// Serial number matches.
			if (ms_Database[i].checksum == 0)
			{
				// No checksum verification required.
				return i;
			}
			
			// Checksum verification is required.
			if (checksum == ms_Database[i].checksum)
			{
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
