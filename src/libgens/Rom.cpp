/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Rom.cpp: ROM loader.                                                    *
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

#include "Rom.hpp"

// C includes.
#include <string.h>

// C++ includes.
#include <algorithm>

// Byteswapping macros.
#include "Util/byteswap.h"

namespace LibGens
{

Rom::Rom(FILE *f, MDP_SYSTEM_ID sysOverride, RomFormat fmtOverride)
{
	// TODO: Open the file ourselves instead of getting a FILE*.
	
	m_sysId = sysOverride;
	m_romFormat = fmtOverride;
	
	// Load the ROM header for detection purposes.
	// TODO: Determine if the file is compressed.
	uint8_t header[ROM_HEADER_SIZE];
	fseek(f, 0, SEEK_SET);
	size_t header_size = fread(header, 1, sizeof(header), f);
	printf("header_size: %d\n", header_size);
	
	if (m_romFormat == RFMT_UNKNOWN)
		m_romFormat = DetectFormat(header, header_size);
	if (m_sysId == MDP_SYSTEM_UNKNOWN)
		m_sysId = DetectSystem(header, header_size, m_romFormat);
	
	// Load the ROM header information.
	readHeaderMD(header, header_size);
	
	// Save the file handle for now.
	m_file = f;
}

Rom::~Rom()
{
	if (m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}
}


/**
 * DetectFormat(): Detect a ROM's format.
 * @param header ROM header.
 * @param header_size ROM header size.
 * @return ROM format.
 */
Rom::RomFormat Rom::DetectFormat(const uint8_t *header, size_t header_size)
{
	/** ISO-9660 (CD-ROM) check. **/
	// ISO-9660 magic from file-5.03: ftp://ftp.astron.com/pub/file/
	const char iso9660_magic[] = {'C', 'D', '0', '0', '1'};
	const char segacd_magic[] = {'S', 'E', 'G', 'A', 'D', 'I', 'S', 'C', 'S', 'Y', 'S', 'T', 'E', 'M'};
	
	if (header_size >= 65536)
	{
		// Check for Sega CD images.
		
		// ISO-9660 magic.
		if (!memcmp(&header[0x9311], iso9660_magic, sizeof(iso9660_magic)))
			return RFMT_CD_BIN_2352;
		if (!memcmp(&header[0x8011], iso9660_magic, sizeof(iso9660_magic)))
			return RFMT_CD_BIN_2048;
		if (!memcmp(&header[0x9301], iso9660_magic, sizeof(iso9660_magic)))
			return RFMT_CD_ISO_2352;
		if (!memcmp(&header[0x8001], iso9660_magic, sizeof(iso9660_magic)))
			return RFMT_CD_ISO_2048;
		
		// SEGADISCSYSTEM magic.
		// NOTE: We can't reliably detect the sector size using this method.
		// Assume 2352-byte sectors for BIN, 2048-byte sectors for ISO.
		if (!memcmp(&header[0x0010], segacd_magic, sizeof(segacd_magic)))
			return RFMT_CD_BIN_2352;
		if (!memcmp(&header[0x0000], segacd_magic, sizeof(segacd_magic)))
			return RFMT_CD_ISO_2048;
	}
	/** END: ISO-9660 (CD-ROM) check. **/
	
	/** SMD-format (.SMD) ROM check. **/
	if (header_size >= 0x4200)
	{
		// 16,384 bytes (one SMD bank) + 512 bytes (SMD header).
		const char sega_magic[] = {'S', 'E', 'G', 'A'};
		if (memcmp(&header[0x100], sega_magic, sizeof(sega_magic)) != 0)
		{
			// "SEGA" not found in the ROM header.
			// This is possibly an SMD-format ROM.
			if ((header[0x08] == 0xAA && header[0x09] == 0xBB && header[0x0A] == 0x06) ||
			    (header[0x280] == 'E' && header[0x281] == 'A'))
			{
				// This is an SMD-format ROM.
				if (header[0x02] == 0x00)
					return RFMT_SMD;
				else
					return RFMT_SMD_SPLIT;
			}
		}
	}
	
	/** MGD-format (.MD) ROM check. */
	// TODO
	
	// Assuming plain binary ROM.
	return RFMT_BINARY;
}


/**
 * DetectSystem(): Detect a ROM's system ID.
 * @param header ROM header.
 * @param header_size ROM header size.
 * @param fmt ROM format.
 * @return System ID.
 */
Rom::MDP_SYSTEM_ID Rom::DetectSystem(const uint8_t *header, size_t header_size, RomFormat fmt)
{
	if (fmt >= RFMT_CD_CUE)
	{
		// CD-ROM. Assume Sega CD.
		// TODO: Sega CD 32X detection.
		return MDP_SYSTEM_MCD;
	}
	
	// TODO: SMS/GG/SG-1000 detection.
	
	if (fmt == RFMT_SMD && header_size >= 0x4200)
	{
		// SMD format check.
		if (header[0x0300] == 0xF9)
		{
			if ((header[0x0280] == '3' && header[0x0281] == 'X') ||
			    (header[0x0407] == 'A' && header[0x0408] == 'S'))
			{
				// 32X ROM.
				return MDP_SYSTEM_32X;
			}
		}
		else
		{
			// Assume MD.
			return MDP_SYSTEM_MD;
		}
	}
	else
	{
		// Plain binary format check.
		const char _32X_magic[] = {'3', '2', 'X'};
		const char mars_magic[] = {'M', 'A', 'R', 'S'};
		if (header_size >= 0x412 && header[0x0200] == 0x4E)
		{
			if (!memcmp(&header[0x0105], _32X_magic, sizeof(_32X_magic)) ||
			    !memcmp(&header[0x040E], mars_magic, sizeof(mars_magic)))
			{
				// 32X ROM.
				return MDP_SYSTEM_32X;
			}
		}
		else
		{
			// Assume MD.
			return MDP_SYSTEM_MD;
		}
	}
	
	// Assume MD.
	return MDP_SYSTEM_MD;
}


/**
 * readHeaderMD(): Read the ROM header. (MD-style)
 * @param header ROM header.
 * @param header_size ROM header size.
 */
void Rom::readHeaderMD(const uint8_t *header, size_t header_size)
{
	// Clear the internal MD header.
	memset(&m_mdHeader, 0x00, sizeof(m_mdHeader));
	
	if (header_size <= 0x100)
	{
		// ROM header is too small. Assume it's blank.
		return;
	}
	
	// Copy the header data first.
	memcpy(&m_mdHeader, &header[0x100], (std::min(header_size, (size_t)0x200) - 0x100));
	
	// Byteswap numerical values.
	m_mdHeader.checksum		= be16_to_cpu(m_mdHeader.checksum);
	m_mdHeader.romStartAddr		= be32_to_cpu(m_mdHeader.romStartAddr);
	m_mdHeader.romEndAddr		= be32_to_cpu(m_mdHeader.romEndAddr);
	m_mdHeader.ramStartAddr		= be32_to_cpu(m_mdHeader.ramStartAddr);
	m_mdHeader.ramEndAddr		= be32_to_cpu(m_mdHeader.ramEndAddr);
	m_mdHeader.sramInfo		= be32_to_cpu(m_mdHeader.sramInfo);
	m_mdHeader.sramStartAddr	= be32_to_cpu(m_mdHeader.sramStartAddr);
	m_mdHeader.sramEndAddr		= be32_to_cpu(m_mdHeader.sramEndAddr);
	
	// Load the ROM names.
	m_romNameJP = SpaceElim(m_mdHeader.romNameJP, sizeof(m_mdHeader.romNameJP));
	m_romNameUS = SpaceElim(m_mdHeader.romNameUS, sizeof(m_mdHeader.romNameUS));
}


/**
 * SpaceElim(): Eliminate excess spaces from a ROM name.
 * TODO: Convert from cp1252/Shift-JIS to UTF-8 first.
 * TODO: Returning an std::string is a bit wasteful...
 * @param src ROM name.
 * @param len Length of ROM name.
 * @return ROM name with excess spaces eliminated.
 */
std::string Rom::SpaceElim(const char *src, size_t len)
{
	// Allocate enough space for the string initially.
	std::string s_elim;
	s_elim.resize(len);
	size_t i_dest = 0;
	
	// Was the last character a graphics character?
	bool lastCharIsGraph = false;
	
	for (size_t n = len; n != 0; n--)
	{
		char chr = *src++;
		
		if (!lastCharIsGraph && !IsGraphChar(chr))
		{
			// This is a space character, and the previous
			// character was not a space character.
			continue;
		}
		
		// This is not a space character,
		// or it is a space character and the previous character wasn't.
		s_elim[i_dest++] = chr;
		lastCharIsGraph = IsGraphChar(chr);
	}
	
	// Resize the string to the last written character.
	// (Make sure there's no space at the end, too.)
	if (i_dest == 0)
		s_elim.clear();
	else if (!IsGraphChar(s_elim[i_dest - 1]))
		s_elim.resize(i_dest - 1);
	else
		s_elim.resize(i_dest);
	
	// Return the string.
	return s_elim;
}

}
