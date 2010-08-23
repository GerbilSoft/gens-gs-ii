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

#include <config.h>

#include "Rom.hpp"

// C includes.
#include <string.h>

// C++ includes.
#include <algorithm>
using std::string;

// Win32 Unicode Translation Layer.
#ifdef _WIN32
#include "Win32/W32U_mini.h"
#endif

// LibGens includes.
#include "Util/byteswap.h"
#include "lg_osd.h"

namespace LibGens
{

Rom::Rom(const utf8_str *filename, MDP_SYSTEM_ID sysOverride, RomFormat fmtOverride)
{
	// Save the filename for later.
	m_filename = string(filename);
	m_romSize = 0;
	
	// Save the system and ROM format overrides.
	m_sysId_override = sysOverride;
	m_romFormat_override = fmtOverride;
	
	// Open the ROM file.
	m_file = fopen(filename, "rb");
	if (!m_file)
	{
		// Error opening the file.
		m_decomp = NULL;
		m_z_entry_list = NULL;
		m_z_entry_sel = NULL;
		return;
	}
	
	// Determine which decompressor to use.
	m_decomp = Decompressor::GetDecompressor(m_file, filename);
	if (!m_decomp)
	{
		// Couldn't find a suitable decompressor.
		// TODO: Indicate that a suitable decompressor couldn't be found.
		fclose(m_file);
		m_file = NULL;
		m_decomp = NULL;
		m_z_entry_list = NULL;
		m_z_entry_sel = NULL;
		return;
	}
	
	// Get the list of files in the archive.
	int ret = m_decomp->getFileInfo(&m_z_entry_list);
	if (ret != 0) // TODO: MDP_ERR_OK
	{
		// Error getting the list of files.
		m_z_entry_list = NULL;
		m_z_entry_sel = NULL;
		
		// Delete the decompressor.
		delete m_decomp;
		m_decomp = NULL;
		
		// Close the file.
		fclose(m_file);
		m_file = NULL;
		return;
	}
	
	if (!isMultiFile())
	{
		// Archive is not multi-file.
		// Load the ROM header.
		m_z_entry_sel = m_z_entry_list;
		loadRomHeader(sysOverride, fmtOverride);
	}
	else
	{
		// Archive is multi-file.
		// We can't continue until the user selects a file to load.
		m_z_entry_sel = NULL;
		
		// Initialize system and format to unknown for now.
		m_sysId = MDP_SYSTEM_UNKNOWN;
		m_romFormat = RFMT_UNKNOWN;
	}
}

Rom::~Rom()
{
	// Free the mdp_z_entry_t list.
	Decompressor::z_entry_t_free(m_z_entry_list);
	m_z_entry_list = NULL;
	m_z_entry_sel = NULL;
	
	// Delete the decompressor.
	delete m_decomp;
	m_decomp = NULL;
	
	// If the file is open, close it.
	if (m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}
}


/**
 * loadRomHeader(): Load the ROM header from the selected ROM file.
 * @param sysOverride System override.
 * @param fmtOverride Format override.
 * @return 0 on success; non-zero on error.
 */
int Rom::loadRomHeader(MDP_SYSTEM_ID sysOverride, RomFormat fmtOverride)
{
	if (!m_z_entry_sel)
	{
		// No file selected!
		return -1;
	}
	
	m_sysId = sysOverride;
	m_romFormat = fmtOverride;
	
	// Get the ROM size.
	// TODO: If it's an MD ROM over 6 MB, return an error.
	// TODO: Save the internal filename for multi-file archives.
	m_romSize = m_z_entry_sel->filesize;
	
	// Load the ROM header for detection purposes.
	uint8_t header[ROM_HEADER_SIZE];
	size_t header_size;
	int ret = m_decomp->getFile(m_z_entry_sel, header, sizeof(header), &header_size);
	if (ret != 0 || header_size == 0)
	{
		// File read error.
		// TODO: Error code constants.
		return -2;
	}
	printf("header_size: %d\n", header_size);
	
	if (m_romFormat == RFMT_UNKNOWN)
		m_romFormat = DetectFormat(header, header_size);
	if (m_sysId == MDP_SYSTEM_UNKNOWN)
		m_sysId = DetectSystem(header, header_size, m_romFormat);
	
	/** MD-only stuff here. **/
	
	// Load the ROM header information.
	readHeaderMD(header, header_size);
	
	// Detect the EEPRom type from the ROM serial number and checksum.
	m_eprType = EEPRom::DetectEEPRomType(
			&m_mdHeader.serialNumber[3],
			(sizeof(m_mdHeader.serialNumber) - 3),
			m_mdHeader.checksum);
	
	// ROM header loaded.
	return 0;
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


/**
 * initSRam(): Initialize an SRam class using the ROM's header information.
 * @param sram Pointer to SRam class.
 * @return Positive value indicating SRam size on success; negative on error.
 */
int Rom::initSRam(SRam *sram) const
{
	if (!isOpen())
		return -1;
	
	// Reset SRam before applying any settings.
	sram->reset();
	
	uint32_t start, end;
	// Check if the ROM header has SRam information.
	// Mask the SRam info value with 0xFFFF4000 and check
	// if it matches the Magic Number.
	// Magic Number: 0x52414000 ('R', 'A', 0x40, 0x00)
	if ((m_mdHeader.sramInfo & 0xFFFF4000) == 0x52414000)
	{
		// ROM header has SRam information. Use these addresses..
		// SRam starting position must be a multiple of 0xF80000.
		start = m_mdHeader.sramStartAddr & 0xF80000;
		end = m_mdHeader.sramEndAddr & 0xFFFFFF;
	}
	else
	{
		// ROM header does not have SRam information.
		// Use default settings.
		start = 0x200000;
		end = 0x203FFF;	// 64 KB
	}
	
	// Check for invalid SRam addresses.
	if ((start > end) || ((end - start) > 0x3FFF))
	{
		// Invalid ending address.
		// Set the end address to the start + 0x3FFF.
		end = start + 0x3FFF;
	}
	
	// Make sure sRam starts on an even byte and ends on an odd byte.
	start &= ~1;
	end |= 1;
	
	// Set the addresses.
	sram->setStart(start);
	sram->setEnd(end);
	
	// If the ROM is smaller than the SRam starting address, always enable SRam.
	// TODO: Instead of hardcoding 2 MB, use SRAM_Start.
	if (m_romSize <= 0x200000)
	{
		sram->setOn(true);
		sram->setWrite(true);
	}
	
	// Load the SRam file.
	// TODO: Use internal filename for multi-file?
	sram->setFilename(m_filename.c_str());
	return sram->load();
}


/**
 * initEEPRom(): Initialize an EEPRom class using the ROM's header information.
 * @param eeprom Pointer to EEPRom class.
 * @return Positive value indicating SRam size on success; negative on error.
 */
int Rom::initEEPRom(EEPRom *eeprom) const
{
	// TODO: Load EEPRom from a file.
	// TODO: Should that be implemented here or in SRam.cpp?
	if (!isOpen())
		return -1;
	
	// Reset the EEPRom and set the type.
	eeprom->reset();
	eeprom->setEEPRomType(m_eprType);
	
	// Load the EEProm file.
	// TODO: Use internal filename for multi-file?
	eeprom->setFilename(m_filename.c_str());
	return eeprom->load();
}


/**
 * loadRom(): Load the ROM image into a buffer.
 * @param buf Buffer.
 * @param siz Buffer size.
 * @return Positive value indicating amount of data read on success; 0 or negative on error.
 */
int Rom::loadRom(void *buf, size_t siz)
{
	// TODO: Use error code constants.
	// NOTE: Don't check for a NULL pointer, since a crash helps diagnose problems.
	if (!isOpen())
		return -1;	// File is closed!
	else if (!m_decomp)
		return -2;	// Decompressor error!
	else if (!m_z_entry_sel)
		return -3;	// No file selected!
	
	if (siz == 0 || siz < m_romSize)
	{
		// ROM buffer isn't large enough for the ROM image.
		return -2;
	}
	
	// Load the ROM image.
	// TODO: Error handling.
	size_t ret_siz = 0;
	m_decomp->getFile(m_z_entry_sel, buf, siz, &ret_siz);
	return ret_siz;
}


/** Multi-file ROM archive support. **/


/**
 * select_z_entry(): Select a file from a multi-file ROM archive to load.
 * @param sel File to load.
 * @return 0 on success; non-zero on error.
 */
int Rom::select_z_entry(const mdp_z_entry_t *sel)
{
	if (!isOpen())
		return -1;	// File is closed!
	if (!isMultiFile())
		return -2;	// Not multi-file.
	if (m_z_entry_sel != NULL)
		return -3;	// ROM was already selected.
	
	// TODO: Verify that sel is actually in m_z_entry_list.
	if (!sel)
		return -4;	// Invalid selection.
	
	// Select the ROM from the archive.
	m_z_entry_sel = sel;
	
	// Load the ROM header.
	loadRomHeader(m_sysId_override, m_romFormat_override);
	return 0;
}

}
