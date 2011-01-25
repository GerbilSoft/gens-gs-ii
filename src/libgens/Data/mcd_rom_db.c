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

#include "mcd_rom_db.h"

// C includes.
#include <stdint.h>
#include <string.h>

/**
 * mcd_rom_db_t: Sega CD Boot ROM database entry.
 */
typedef struct _mcd_rom_db_t
{
	uint32_t crc32;			// ROM CRC32. (with original HINT vector)
	uint32_t crc32_nohint;		// ROM CRC32. (with 0xFFFF HINT vector)
	uint8_t md5sum[16];		// ROM md5sum. (with original HINT vector)
	uint8_t md5sum_nohint[16];	// ROM md5sum. (with 0xFFFF HINT vector)
	
	MCD_RegionCode_t region_code;	// Region code.
	MCD_RomStatus_t rom_status;	// ROM status.
	
	const utf8_str *description;	// ROM description.
	const utf8_str *notes;		// ROM notes. (Optional)
} mcd_rom_db_t;

#define MCD_ROM_DATABASE_ENTRIES ((int)((sizeof(McdRomDatabase) / sizeof(McdRomDatabase[0])) - 1))
static const mcd_rom_db_t McdRomDatabase[] =
{
	/** Sega CD: Model 1 **/
	
	// Mega CD (J) Boot ROM v1.00p (NTSC)
	// 1991/12/17 12:00
	{
		0x9D2DA8F2,		// CRC32 (with original HINT vector)
		0x0335E10D,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x27, 0x8A, 0x93, 0x97, 0xD1, 0x92, 0x14, 0x9E,
		 0x84, 0xE8, 0x20, 0xAC, 0x62, 0x1A, 0x8E, 0xDD},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xE8, 0xAC, 0xD8, 0x7F, 0xD4, 0xDF, 0x84, 0xF0,
		 0x37, 0x32, 0x6F, 0xF3, 0x01, 0x78, 0xC2, 0x1F},
		
		// Region code and ROM support status.
		Region_Japan_NTSC, RomStatus_Recommended,
		
		// Description and notes.
		"Mega CD (J) v1.00p (NTSC)",
		"Recommended boot ROM for Mega CD (J).\n"
		"This ROM will also work for Japan/PAL."
	},
	
	// Mega CD (J) Boot ROM v1.00S (PAL)
	// 1991/12/28 20:30
	{
		0x550F30BB,		// CRC32 (with original HINT vector)
		0xCB177944,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xBD, 0xEB, 0x4C, 0x47, 0xDA, 0x61, 0x39, 0x46,
		 0xD4, 0x22, 0xD9, 0x7D, 0x98, 0xB2, 0x1C, 0xDA},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x08, 0xDD, 0xE9, 0x4F, 0x5B, 0x35, 0x8C, 0x85,
		 0xDB, 0x00, 0x6E, 0xB2, 0x9B, 0x19, 0xE2, 0x3B},
		
		// Region code and ROM support status.
		Region_Japan_PAL, RomStatus_Supported,
		
		// Description and notes.
		"Mega CD (J) v1.00S (PAL)",
		"This ROM will also work for Japan/NTSC."
	},
	
	// Sega CD (U) Boot ROM v1.10
	// 1992/10/11 18:30
	{
		0xC6D10268,		// CRC32 (with original HINT vector)
		0x58C94B97,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x2E, 0xFD, 0x74, 0xE3, 0x23, 0x2F, 0xF2, 0x60,
		 0xE3, 0x71, 0xB9, 0x9F, 0x84, 0x02, 0x4F, 0x7F},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x74, 0x2B, 0x4B, 0x7A, 0xE5, 0x1A, 0xA1, 0x15,
		 0x51, 0xBD, 0xBF, 0x02, 0xE9, 0x41, 0x25, 0xCE},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Recommended,
		
		// Description and notes.
		"Sega CD (U) v1.10",
		"Recommended boot ROM for Sega CD (U)."
	},
	
	// Mega CD (E) Boot ROM v1.00
	// 1992/10/27 15:15
	{
		0x529AC15A,		// CRC32 (with original HINT vector)
		0xCC8288A5,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xE6, 0x6F, 0xA1, 0xDC, 0x58, 0x20, 0xD2, 0x54,
		 0x61, 0x1F, 0xDC, 0xDB, 0xA0, 0x66, 0x23, 0x72},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xC8, 0xFF, 0x26, 0x9D, 0x76, 0x4C, 0x3F, 0x94,
		 0xF4, 0x81, 0xFC, 0x3D, 0x2F ,0xE5, 0x1A, 0x95},
		
		// Region code and ROM support status.
		Region_Europe, RomStatus_Recommended,
		
		// Description and notes.
		"Mega CD (E) v1.00",
		"Recommended boot ROM for Mega CD (E)."
	},
	
	/** Sega CD: Model 2 **/
	
	// Mega CD 2 (J) Boot ROM v2.00c (NTSC)
	// 1992/12/22 14:00
	{
		0xDD6CC972,		// CRC32 (with original HINT vector)
		0x4374808D,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x68, 0x3A, 0x8A, 0x9E, 0x27, 0x36, 0x62, 0x56,
		 0x11, 0x72, 0x46, 0x8D, 0xFA, 0x28, 0x58, 0xEB},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x08, 0xB0, 0x8B, 0x1A, 0x95, 0xDC, 0x30, 0x58,
		 0x86, 0x55, 0xAF, 0x6A, 0xBE, 0x74, 0xE2, 0x73},
		
		// Region code and ROM support status.
		Region_Japan_NTSC, RomStatus_Unsupported,
		
		// Description and notes.
		"Mega CD 2 (J) v2.00c (NTSC)",
		"Gens/GS II does not support the hardware changes"
		"in the Mega CD model 2.\n"
		"This ROM will also work for Japan/PAL."
	},
	
	// Sega CD 2 (U) Boot ROM v2.00 (Bad Dump)
	// 1993/03/14 01:00
	{
		0x340B4BE4,		// CRC32 (with original HINT vector)
		0xAA13021B,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x68, 0x45, 0x57, 0x9B, 0xD2, 0x11, 0xE2, 0x4E,
		 0xAF, 0xE3, 0x13, 0x93, 0x3E, 0x6F, 0x8D, 0x7B},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xC3, 0x5C, 0xAF, 0x7C, 0x49, 0x2C, 0x15, 0xD0,
		 0x23, 0x43, 0x39, 0x6C, 0xC7, 0x4D, 0x26, 0x95},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Broken,
		
		// Description and notes.
		"Sega CD 2 (U) v2.00",
		"This is a known bad dump, and will not work properly"
		"in any Sega CD emulator."
	},
	
	// Sega CD 2 (U) Boot ROM v2.00
	// 1993/03/14 01:00
	{
		0x8AF65F58,		// CRC32 (with original HINT vector)
		0x14EE16A7,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x31, 0x0A, 0x90, 0x81, 0xD2, 0xED, 0xF2, 0xD3,
		 0x16, 0xAB, 0x38, 0x81, 0x31, 0x36, 0x72, 0x5E},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xBC, 0x36, 0xED, 0xAE, 0xE6, 0x34, 0x3F, 0xDA,
		 0x06, 0x88, 0x43, 0x23, 0x18, 0x21, 0x04, 0x0E},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"Sega CD 2 (U) v2.00",
		"Gens/GS II does not support the hardware changes"
		"in the Sega CD model 2."
	},
	
	// Mega CD 2 (E) Boot ROM v2.00
	// 1993/03/30 12:00
	{
		0x0507B590,		// CRC32 (with original HINT vector)
		0x9B1FFC6F,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x9B, 0x56, 0x2E, 0xBF, 0x2D, 0x09, 0x5B, 0xF1,
		 0xDA, 0xBA, 0xDB, 0xC1, 0x88, 0x1f, 0x51, 0x9A},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x8D, 0x07, 0xEB, 0x19, 0xCA, 0xBE, 0xCB, 0x6B,
		 0x51, 0x58, 0x34, 0x70, 0x5D, 0x36, 0xF7, 0xAD},
		
		// Region code and ROM support status.
		Region_Europe, RomStatus_Unsupported,
		
		// Description and notes.
		"Mega CD 2 (E) v2.00",
		"Gens/GS II does not support the hardware changes"
		"in the Mega CD model 2."
	},
	
	/** Sega CD: Model 2 (ROM revision W) **/
	
	// Sega CD 2 (U) Boot ROM v2.00W
	// 1993/06/01 20:00
	{
		0x9F6F6276,		// CRC32 (with original HINT vector)
		0x01772B89,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x85, 0x4B, 0x91, 0x50, 0x24, 0x0A, 0x19, 0x80,
		 0x70, 0x15, 0x0E, 0x45, 0x66, 0xAE, 0x12, 0x90},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x5F, 0x83, 0x18, 0x21, 0xB8, 0x72, 0x56, 0x55,
		 0xDF, 0xD6, 0x5F, 0x49, 0xA8, 0xAE, 0x7F, 0x2B},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"Sega CD 2 (U) v2.00W",
		"Gens/GS II does not support the hardware changes"
		"in the Sega CD model 2."
	},
	
	// Mega CD 2 (E) Boot ROM v2.00W
	// 1993/06/01 20:00
	{
		0x4D5CB8DA,		// CRC32 (with original HINT vector)
		0xD344F125,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xB1, 0x0C, 0x0A, 0x97, 0xAB, 0xC5, 0x7B, 0x75,
		 0x84, 0x97, 0xD3, 0xFA, 0xE6, 0xAB, 0x35, 0xA4},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xD8, 0xB8, 0xB7, 0x20, 0xDE, 0xA6, 0xC6, 0xBA,
		 0x25, 0xC3, 0x09, 0xED, 0x63, 0x39, 0x30, 0xF4},
		
		// Region code and ROM support status.
		Region_Europe, RomStatus_Unsupported,
		
		// Description and notes.
		"Mega CD 2 (E) v2.00W",
		"Gens/GS II does not support the hardware changes"
		"in the Mega CD model 2."
	},
	
	/** Sega CD: Model 2 (ROM revision X) **/
	
	// Sega CD 2 (U) Boot ROM v2.11X
	// 1993/06/21 16:00
	{
		0x2E49D72C,		// CRC32 (with original HINT vector)
		0xB0519ED3,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xEC, 0xC8, 0x37, 0xC3, 0x1D, 0x77, 0xB7, 0x74,
		 0xC6, 0xE2, 0x7E, 0x38, 0xF8, 0x28, 0xAA, 0x9A},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x3A, 0x4B, 0x4F, 0x8B, 0x13, 0x6D, 0x09, 0x5A,
		 0x5F, 0x85, 0x85, 0x3E, 0x0B, 0x84, 0x47, 0xFA},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"Sega CD 2 (U) v2.11X",
		"Gens/GS II does not support the hardware changes"
		"in the Sega CD model 2."
	},
	
	/** Sega MultiMega / CDX **/
	
	// Sega CDX (U) Boot ROM v2.21X
	// 1993/09/07 19:00
	{
		0xD48C44B5,		// CRC32 (with original HINT vector)
		0x4A940D4A,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xBA, 0xCA, 0x1D, 0xF2, 0x71, 0xD7, 0xC1, 0x1F,
		 0xE5, 0x00, 0x87, 0xC0, 0x35, 0x8F, 0x4E, 0xB5},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x85, 0x1E, 0xC2, 0xED, 0xDB, 0x88, 0xD2, 0x18,
		 0x61, 0xF5, 0x6E, 0x51, 0xEC, 0xA0, 0x74, 0x67},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"Sega CDX (U) v2.21X",
		"Gens/GS II does not support the hardware changes"
		"in the Sega CDX."
	},
	
	/** Pioneer LaserActive **/
	
	// Pioneer LaserActive (J) Boot ROM v0.98
	// 1993/03/29 17:00
	{
		0x00EEDB3A,		// CRC32 (with original HINT vector)
		0x9EF692C5,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xA5, 0xA2, 0xF9, 0xAA, 0xE5, 0x7D, 0x46, 0x4B,
		 0xC6, 0x6B, 0x80, 0xEE, 0x79, 0xC3, 0xDA, 0x6E},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x1C, 0x10, 0x06, 0x57, 0x44, 0x71, 0x6F, 0x14,
		 0x44, 0x78, 0x00, 0xDA, 0xB3, 0xF9, 0x33, 0x77},
		
		// Region code and ROM support status.
		Region_Japan_NTSC, RomStatus_Unsupported,
		
		// Description and notes.
		"Pioneer LaserActive (J) v0.98",
		"Gens/GS II does not support the hardware changes"
		"in the Pioneer LaserActive."
	},
	
	// Pioneer LaserActive (U) Boot ROM v0.98
	// 1993/03/29 17:00
	{
		0x3B10CF41,		// CRC32 (with original HINT vector)
		0xA50886BE,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x69, 0x1C, 0x3F, 0xD3, 0x68, 0x21, 0x12, 0x80,
		 0xD2, 0x68, 0x64, 0x5C, 0x0E, 0xFD, 0x2E, 0xFF},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x97, 0x9B, 0x8F, 0x04, 0x07, 0x22, 0x54, 0x27,
		 0x66, 0x79, 0x83, 0xEA, 0x09, 0x45, 0x9F, 0x5C},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"Pioneer LaserActive (U) v0.98",
		"Gens/GS II does not support the hardware changes"
		"in the Pioneer LaserActive."
	},
	
	// Pioneer LaserActive (U) Boot ROM v1.04
	// 1993/09/22 17:00
	{
		0x50CD3D23,		// CRC32 (with original HINT vector)
		0xCED574DC,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x0E, 0x73, 0x93, 0xCD, 0x09, 0x51, 0xD6, 0xDD,
		 0xE8, 0x18, 0xFC, 0xD4, 0xCD, 0x81, 0x94, 0x66},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xB6, 0xF1, 0x88, 0x64, 0x69, 0xF9, 0xD1, 0xFD,
		 0xF0, 0x37, 0x21, 0x9A, 0x56, 0xB7, 0xBA, 0xA8},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"Pioneer LaserActive (U) v1.04",
		"Gens/GS II does not support the hardware changes"
		"in the Pioneer LaserActive."
	},
	
	/** JVC Wondermega / X'Eye **/
	
	// Sega Wondermega (J) Boot ROM v1.00W
	// 1992/02/06 01:30
	// NOTE: This version has an incorrect Initial SP: 0xF4FFFFD0.
	// This won't affect operation due to 24-bit addressing, though.
	{
		0x2B10948D,		// CRC32 (with original HINT vector)
		0xB508DD72,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x0A, 0xC3, 0xF0, 0x20, 0x91, 0x06, 0xBA, 0x6B,
		 0x98, 0xC3, 0x10, 0x25, 0xE8, 0x9A, 0x27, 0x32},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x6D, 0x83, 0x01, 0x2C, 0x92, 0x1D, 0xAC, 0xF2,
		 0x0E, 0x7D, 0x4D, 0xAF, 0x48, 0x02, 0xCE, 0xB8},
		
		// Region code and ROM support status.
		Region_Japan_NTSC, RomStatus_Unsupported,
		
		// Description and notes.
		"Sega Wondermega (J) v1.00W",
		"Gens/GS II does not support the hardware changes"
		"in the Sega Wondermega."
	},
	
	// Sega Wondermega (J) Boot ROM v1.00W
	// 1992/02/06 01:30
	{
		0xD21FE71D,		// CRC32 (with original HINT vector)
		0x4C07AEE2,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x73, 0x2B, 0x60, 0x96, 0x02, 0x26, 0xCA, 0xF3,
		 0x3D, 0xF6, 0xB4, 0x87, 0xF3, 0x76, 0xDE, 0x69},
		
		// md5sum (with 0xFFFF HINT vector)
		{0xB6, 0x3D, 0x51, 0x38, 0xDF, 0x21, 0xF8, 0x3D,
		 0x21, 0xB5, 0x86, 0x09, 0xB6, 0x32, 0x22, 0x5F},
		
		// Region code and ROM support status.
		Region_Japan_NTSC, RomStatus_Unsupported,
		
		// Description and notes.
		"Sega Wondermega (J) v1.00W",
		"Gens/GS II does not support the hardware changes"
		"in the Sega Wondermega."
	},
	
	// JVC X'Eye (U) Boot ROM v2.00
	// 1993/12/27 16:30
	// NOTE: This version has an incorrect Initial SP: 0xF4FFFFD0.
	// This won't affect operation due to 24-bit addressing, though.
	{
		0xD000FDA3,		// CRC32 (with original HINT vector)
		0x4E18B45C,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0xF2, 0x09, 0x26, 0x07, 0xE4, 0x13, 0x22, 0x2B,
		 0x69, 0x63, 0x02, 0x8A, 0x6C, 0x7A, 0x05, 0x61},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x28, 0xA2, 0xBE, 0xCD, 0x63, 0xBC, 0xB7, 0x51,
		 0xE9, 0x3C, 0x13, 0xF4, 0xC3, 0x19, 0xBC, 0x3B},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"JVC X'Eye (U) v2.00",
		"Gens/GS II does not support the hardware changes"
		"in the JVC X'Eye."
	},
	
	// JVC X'Eye (U) Boot ROM v2.00
	// 1993/12/27 16:30
	{
		0x290F8E33,		// CRC32 (with original HINT vector)
		0xB717C7CC,		// CRC32 (with 0xFFFF HINT vector)
		
		// md5sum (with original HINT vector)
		{0x82, 0xCE, 0x23, 0x63, 0xF7, 0xDD, 0xC6, 0x20,
		 0xB9, 0xC2, 0x55, 0xD3, 0x58, 0x33, 0x11, 0x8F},
		
		// md5sum (with 0xFFFF HINT vector)
		{0x2D, 0x09, 0x35, 0x74, 0x65, 0xA3, 0x32, 0x8F,
		 0xFB, 0x65, 0x7E, 0xF3, 0x8F, 0x8D, 0x8D, 0xD0},
		
		// Region code and ROM support status.
		Region_USA, RomStatus_Unsupported,
		
		// Description and notes.
		"JVC X'Eye (U) v2.00",
		"Gens/GS II does not support the hardware changes"
		"in the JVC X'Eye."
	},
	
	// End of database.
	{0, 0, {0}, {0}, 0, 0, NULL, NULL}
};


/**
 * lg_mcd_rom_FindByCRC32(): Find a Sega CD Boot ROM by CRC32.
 * @param rom_crc32 Sega CD Boot ROM CRC32.
 * @return ROM ID, or -1 if not found.
 */
int lg_mcd_rom_FindByCRC32(uint32_t rom_crc32)
{
	const mcd_rom_db_t *db_entry = &McdRomDatabase[0];
	
	// TODO: Check CRC32 or the description?
	// CRC32 could be 0, but checking description is somewhat slower...
	for (int i = 0; db_entry->description != NULL; db_entry++, i++)
	{
		if (db_entry->crc32 == rom_crc32 ||
		    db_entry->crc32_nohint == rom_crc32)
		{
			// CRC32 match!
			return i;
		}
	}
	
	// CRC32 not matched.
	return MCD_ROM_UNKNOWN;
}


/**
 * lg_mcd_rom_GetDescription(): Get a Boot ROM's description.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM description, or NULL if the ID is invalid.
 */
const utf8_str *lg_mcd_rom_GetDescription(int rom_id)
{
	if (rom_id < 0 || rom_id >= MCD_ROM_DATABASE_ENTRIES)
		return NULL;
	
	return McdRomDatabase[rom_id].description;
}


/**
 * lg_mcd_rom_GetDescription(): Get a Boot ROM's notes.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM notes, or NULL if the ID is invalid.
 */
const utf8_str *lg_mcd_rom_GetNotes(int rom_id)
{
	if (rom_id < 0 || rom_id >= MCD_ROM_DATABASE_ENTRIES)
		return NULL;
	
	return McdRomDatabase[rom_id].notes;
}


/**
 * lg_mcd_rom_GetRegion(): Get a Boot ROM's region.
 * @param rom_id Boot ROM ID.
 * @return Boot ROM ID, or Region_MAX if the ID is invalid.
 */
MCD_RegionCode_t lg_mcd_rom_GetRegion(int rom_id)
{
	if (rom_id < 0 || rom_id >= MCD_ROM_DATABASE_ENTRIES)
		return Region_MAX;
	
	return McdRomDatabase[rom_id].region_code;
}


/**
 * lg_mcd_rom_GetSupportStatus(): Get a Boot ROM's support status.
 * @param rom_id Boot ROM ID.
 * @return ROM support status, or RomStatus_MAX if the ID is invalid.
 */
MCD_RomStatus_t lg_mcd_rom_GetSupportStatus(int rom_id)
{
	if (rom_id < 0 || rom_id >= MCD_ROM_DATABASE_ENTRIES)
		return RomStatus_MAX;
	
	return McdRomDatabase[rom_id].rom_status;
}


/**
 * lg_mcd_rom_GetRegionCodeString(): Get a string describing a region code.
 * @param region_code Region code.
 * @return Region code string, or NULL if the region code is invalid.
 */
const utf8_str *lg_mcd_rom_GetRegionCodeString(MCD_RegionCode_t region_code)
{
	switch (region_code)
	{
		case Region_Japan_NTSC:	return "Japan (NTSC)";
		case Region_USA: 	return "USA";
		case Region_Japan_PAL:	return "Japan (PAL)";
		case Region_Europe: 	return "Europe";
		default:		return NULL;
	}
}
