/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Rom.cpp: ROM loader.                                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include <libgens/config.libgens.h>

#include "Rom.hpp"
#include "Decompressor/DcMemFake.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cassert>

// C++ includes.
#include <algorithm>
#include <string>
using std::string;
using std::u16string;

// Character set conversion.
#include "libgenstext/Encoding.hpp"
#include "libgenstext/StringManip.hpp"

// LibGens includes.
#include "Util/byteswap.h"
#include "macros/common.h"
#include "lg_osd.h"

// Needed for checking CRC32s for "Xin Qi Gai Wang Zi" (Beggar Prince).
#include <zlib.h>

namespace LibGens {

/**
 * Rom private class.
 */
class RomPrivate
{
	public:
		RomPrivate(Rom *q, const utf8_str *filename,
				Rom::MDP_SYSTEM_ID sysOverride = Rom::MDP_SYSTEM_UNKNOWN,
				Rom::RomFormat fmtOverride = Rom::RFMT_UNKNOWN);
		RomPrivate(Rom *q, const uint8_t *rom_data, unsigned int rom_size,
				Rom::MDP_SYSTEM_ID sysOverride = Rom::MDP_SYSTEM_UNKNOWN,
				Rom::RomFormat fmtOverride = Rom::RFMT_UNKNOWN);
		~RomPrivate();

	private:
		Rom *const q;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		RomPrivate(const RomPrivate &);
		RomPrivate &operator=(const RomPrivate &);

	public:
		// System ID and ROM format detection functions.
		static Rom::RomFormat DetectFormat(const uint8_t *header, size_t header_size, size_t rom_size);
		// NOTE: 'header' must be deinterleaved.
		static Rom::MDP_SYSTEM_ID DetectSystem(const uint8_t *header, size_t header_size, Rom::RomFormat fmt);

		// ROM file and decompressor variables.
		FILE *file;
		Decompressor *decomp;
		mdp_z_entry_t *z_entry_list;
		const mdp_z_entry_t *z_entry_sel;

		/**
		 * Determine if the loaded ROM archive has multiple files.
		 * @return True if the ROM archive has multiple files; false if it doesn't.
		 */
		inline bool isMultiFile(void) const
			{ return (z_entry_list && z_entry_list->next); }

		// System ID and ROM format.
		Rom::MDP_SYSTEM_ID sysId;
		Rom::RomFormat romFormat;

		// System overrides specified in the constructor.
		Rom::MDP_SYSTEM_ID sysId_override;
		Rom::RomFormat romFormat_override;

		// ROM information.
		std::string filename;		// ROM filename.
		std::string filename_baseNoExt;	// ROM filename. (Basename, no extension.)
		unsigned int romSize;		// ROM size.

		// ROM names.
		// MD: Obtained from ROM header.
		std::string romNameJP;	// Domestic name.
		std::string romNameUS;	// Overseas name.

		/**
		 * Region code.
		 * Same format as the newer hex format used in MD ROMs.
		 */
		int regionCode;
		static int DetectRegionCodeMD(const char countryCodes[16]);

		/**
		 * Decode a Super Magic Drive interleaced block.
		 * @param dest Destination block. (Must be 16 KB.)
		 * @param src Source block. (Must be 16 KB.)
		 */
		void DecodeSMDBlock(uint8_t *dest, const uint8_t *src);

		/** ROM header functions. **/
		int loadRomHeader(Rom::MDP_SYSTEM_ID sysOverride, Rom::RomFormat fmtOverride);
		void readHeaderMD(const uint8_t *header, size_t header_size);

		/**
		 * Mega Drive ROM header.
		 * This matches the MD ROM header format exactly.
		 * 
		 * NOTE: Strings are NOT null-terminated!
		 */
		struct MD_RomHeader {
			char consoleName[16];
			char copyright[16];
			char romNameJP[48];	// Japanese ROM name.
			char romNameUS[48];	// US/Europe ROM name.
			char serialNumber[14];
			uint16_t checksum;
			char ioSupport[16];

			// ROM/RAM address information.
			uint32_t romStartAddr;
			uint32_t romEndAddr;
			uint32_t ramStartAddr;
			uint32_t ramEndAddr;

			// Save RAM information.
			// Info format: 'R', 'A', %1x1yz000, 0x20
			// x == 1 for backup (SRAM), 0 for not backup
			// yz == 10 for even addresses, 11 for odd addresses
			uint32_t sramInfo;
			uint32_t sramStartAddr;
			uint32_t sramEndAddr;

			// Miscellaneous.
			char modemInfo[12];
			char notes[40];
			char countryCodes[16];
		};

		MD_RomHeader m_mdHeader;

		uint32_t rom_crc32;	// ROM CRC32.
};

/**
 * Initialize a new RomPrivate object.
 * @param q Rom object that owns this RomPrivate object.
 * @param filename ROM filename.
 * @param sysOverride System override.
 * @param fmtOverride ROM format override.
 */
RomPrivate::RomPrivate(Rom *q, const utf8_str *filename,
			Rom::MDP_SYSTEM_ID sysOverride,
			Rom::RomFormat fmtOverride)
	: q(q)
	, file(nullptr)
	, decomp(nullptr)
	, z_entry_list(nullptr)
	, z_entry_sel(nullptr)
	, sysId(Rom::MDP_SYSTEM_UNKNOWN)
	, romFormat(Rom::RFMT_UNKNOWN)
	, sysId_override(sysOverride)
	, romFormat_override(fmtOverride)
	, romSize(0)
	, regionCode(0)
	, rom_crc32(0)
{
	// If filename is nullptr, don't do anything else.
	if (!filename)
		return;

	// Save the filename for later.
	this->filename = string(filename);

	// Remove the directories and extension from the ROM filename.
	filename_baseNoExt = LibGensText::FilenameBaseNoExt(this->filename);

	// Open the ROM file.
	file = fopen(filename, "rb");
	if (!file)
		return;

	// Determine which decompressor to use.
	decomp = Decompressor::GetDecompressor(file, filename);
	if (!decomp) {
		// Couldn't find a suitable decompressor.
		// TODO: Indicate that a suitable decompressor couldn't be found.
		fclose(file);
		file = nullptr;
		return;
	}

	// Get the list of files in the archive.
	int ret = decomp->getFileInfo(&z_entry_list);
	if (ret != 0) { // TODO: MDP_ERR_OK
		// Error getting the list of files.
		z_entry_list = nullptr;

		// Delete the decompressor.
		delete decomp;
		decomp = nullptr;

		// Close the file.
		fclose(file);
		file = nullptr;
		return;
	}

	if (!isMultiFile()) {
		// Archive is not multi-file.
		// Load the ROM header.
		z_entry_sel = z_entry_list;
		loadRomHeader(sysOverride, fmtOverride);
	} else {
		// Archive is multi-file.
		// We can't continue until the user selects a file to load.
		z_entry_sel = nullptr;
	}
}

/**
 * Initialize a new RomPrivate object.
 * @param q Rom object that owns this RomPrivate object.
 * @param rom_data ROM data in memory. (NOTE: Must remain valid as long as Rom is open.)
 * @param rom_size Size of rom_data.
 * @param sysOverride System override.
 * @param fmtOverride ROM format override.
 */
RomPrivate::RomPrivate(Rom *q, const uint8_t *rom_data, unsigned int rom_size,
			Rom::MDP_SYSTEM_ID sysOverride,
			Rom::RomFormat fmtOverride)
	: q(q)
	, file(nullptr)
	, decomp(nullptr)
	, z_entry_list(nullptr)
	, z_entry_sel(nullptr)
	, sysId(Rom::MDP_SYSTEM_UNKNOWN)
	, romFormat(Rom::RFMT_UNKNOWN)
	, sysId_override(sysOverride)
	, romFormat_override(fmtOverride)
	, romSize(0)
	, regionCode(0)
	, rom_crc32(0)
{
	// TODO: Support decompression from RAM.
	// For now, use a pseudo-decompressor that provides the same
	// interface, but just reads from memory.
	if (!rom_data || rom_size == 0)
		return;

	// "Open" the file using the fake decompressor.
	decomp = new DcMemFake(rom_data, rom_size);

	// Get the list of files in the archive.
	int ret = decomp->getFileInfo(&z_entry_list);
	if (ret != 0) { // TODO: MDP_ERR_OK
		// Error getting the list of files.
		z_entry_list = nullptr;

		// Delete the decompressor.
		delete decomp;
		decomp = nullptr;

		// Close the file.
		fclose(file);
		file = nullptr;
		return;
	}

	if (!isMultiFile()) {
		// Archive is not multi-file.
		// Load the ROM header.
		z_entry_sel = z_entry_list;
		loadRomHeader(sysOverride, fmtOverride);
	} else {
		// Archive is multi-file.
		// We can't continue until the user selects a file to load.
		z_entry_sel = nullptr;
	}
}

/**
 * Destroy the RomPrivate object.
 */
RomPrivate::~RomPrivate()
{
	// Free the mdp_z_entry_t list.
	Decompressor::z_entry_t_free(z_entry_list);

	// Delete the decompressor.
	delete decomp;

	// If the file is open, close it.
	if (file) {
		fclose(file);
	}
}

/**
 * Detect a ROM's format.
 * @param header ROM header.
 * @param header_size ROM header size.
 * @param rom_size ROM size. (Used for interleaved format detection.)
 * @return ROM format.
 */
Rom::RomFormat RomPrivate::DetectFormat(const uint8_t *header, size_t header_size, size_t rom_size)
{
	/** ISO-9660 (CD-ROM) check. **/
	// ISO-9660 magic from file-5.03: ftp://ftp.astron.com/pub/file/
	static const char iso9660_magic[] = {'C', 'D', '0', '0', '1'};
	static const char segacd_magic[] = {'S', 'E', 'G', 'A', 'D', 'I', 'S', 'C', 'S', 'Y', 'S', 'T', 'E', 'M'};

	if (header_size >= 65536) {
		// Check for Sega CD images.

		// ISO-9660 magic.
		if (!memcmp(&header[0x9311], iso9660_magic, sizeof(iso9660_magic)))
			return Rom::RFMT_CD_BIN_2352;
		if (!memcmp(&header[0x8011], iso9660_magic, sizeof(iso9660_magic)))
			return Rom::RFMT_CD_BIN_2048;
		if (!memcmp(&header[0x9301], iso9660_magic, sizeof(iso9660_magic)))
			return Rom::RFMT_CD_ISO_2352;
		if (!memcmp(&header[0x8001], iso9660_magic, sizeof(iso9660_magic)))
			return Rom::RFMT_CD_ISO_2048;

		// SEGADISCSYSTEM magic.
		// NOTE: We can't reliably detect the sector size using this method.
		// Assume 2352-byte sectors for BIN, 2048-byte sectors for ISO.
		if (!memcmp(&header[0x0010], segacd_magic, sizeof(segacd_magic)))
			return Rom::RFMT_CD_BIN_2352;
		if (!memcmp(&header[0x0000], segacd_magic, sizeof(segacd_magic)))
			return Rom::RFMT_CD_ISO_2048;
	}
	/** END: ISO-9660 (CD-ROM) check. **/

	/**
	 * SMD-format (.SMD) ROM check.
	 * ROM size must be a multiple of 16 KB,
	 * excluding the 512-byte header.
	 */
	if (header_size >= 0x4200 && ((rom_size - 512) % 16384) == 0) {
		// 16,384 bytes (one SMD bank) + 512 bytes (SMD header).
		const char sega_magic[] = {'S', 'E', 'G', 'A'};
		if (memcmp(&header[0x100], sega_magic, sizeof(sega_magic)) != 0) {
			// "SEGA" not found in the ROM header.
			// This is possibly an SMD-format ROM.
			if ((header[0x08] == 0xAA && header[0x09] == 0xBB && header[0x0A] == 0x06) ||
			    (header[0x280] == 'E' && header[0x281] == 'A'))
			{
				// This is an SMD-format ROM.
				// TODO: Add a property "headerSize" indicating how many
				// bytes to reserve for the header when loading a ROM.
				// TODO: Respect the "number of ROM banks" field?
				if (header[0x02] == 0x00) {
					return Rom::RFMT_SMD;
				} else {
					return Rom::RFMT_SMD_SPLIT;
				}
			}
		}
	}

	/** MGD-format (.MD) ROM check. */
	// NOTE: ".md" is used by No-Intro for plain binary dumps
	// TODO: Make sure rom_size is even?
	if (header_size >= 0x100) {
		// MGD format is interleaved without blocks.
		// The odd bytes in the MD header are located at 0x80.
		if (header[0x80] == 'E' && header[0x81] == 'A') {
			// Odd bytes of "SEGA" found in the ROM header.
			// This is probably an MGD ROM.
			return Rom::RFMT_MGD;
		}
	}

	// Assuming plain binary ROM.
	return Rom::RFMT_BINARY;
}

/**
 * Detect a ROM's system ID.
 * @param header ROM header. (deinterleaved)
 * @param header_size ROM header size.
 * @param fmt ROM format.
 * @return System ID.
 */
Rom::MDP_SYSTEM_ID RomPrivate::DetectSystem(const uint8_t *header, size_t header_size, Rom::RomFormat fmt)
{
	if (fmt >= Rom::RFMT_CD_CUE) {
		// CD-ROM. Assume Sega CD.
		// TODO: Sega CD 32X detection.
		return Rom::MDP_SYSTEM_MCD;
	}

	// TODO: SMS/GG/SG-1000 detection.

	// 'header' has already been deinterleaved by the calling function.

	// Check for 32X.
	static const char _32X_magic[] = {'3', '2', 'X'};
	static const char mars_magic[] = {'M', 'A', 'R', 'S'};
	if (header_size >= 0x412 && header[0x0200] == 0x4E) {
		if (!memcmp(&header[0x0105], _32X_magic, sizeof(_32X_magic)) ||
		    !memcmp(&header[0x040E], mars_magic, sizeof(mars_magic)))
		{
			// 32X ROM.
			return Rom::MDP_SYSTEM_32X;
		}
	}

	// Check for Pico.
	const char pico_magic[] = {'P', 'I', 'C', 'O'};
	if (header_size >= 0x200) {
		if (!memcmp(&header[0x0105], pico_magic, sizeof(pico_magic))) {
			// Pico ROM.
			return Rom::MDP_SYSTEM_PICO;
		}
	}

	// Assume MD.
	// TODO: Add support for SMS, GG, etc.
	return Rom::MDP_SYSTEM_MD;
}

/**
 * Detect an MD region code.
 * @param countryCodes Country codes section of MD ROM header.
 * @return Region code, or -1 on error.
 */
int RomPrivate::DetectRegionCodeMD(const char countryCodes[16])
{
	int code = 0;
	
	/**
	 * The Rom class internally uses the hex format used in later MD games.
	 * Format:
	 * - Bit 0: Japan (NTSC)
	 * - Bit 1: Asia (PAL)
	 * - Bit 2: USA (NTSC)
	 * - Bit 3: Europe (PAL)
	 */
	
	// Check for string codes first.
	// Some games incorrectly use these.
	if (!strncasecmp(countryCodes, "EUR", 3))
		code = (1 << 3);
	else if (!strncasecmp(countryCodes, "USA", 3))
		code = (1 << 2);
	else if (!strncasecmp(countryCodes, "JPN", 3))
		code = (1 << 0);
	else
	{
		// Check for other country codes.
		for (int i = 0; i < 4; i++)
		{
			char c = toupper(countryCodes[i]);
			
			// NOTE: 'E' is used for both Europe and hex codes,
			// but it's not seen often as a hex code, so we'll
			// assume its use indicates Europe.
			
			if (c == 'U')
				code |= (1 << 2);
			else if (c == 'J')
				code |= (1 << 0);
			else if (c == 'E')
				code |= (1 << 3);
			else if (c < 16)	// hex code not mapped to ASCII
				code |= c;
			else if ((c >= '0') && (c <= '9'))
				code |= (c - '0');
			else if ((c >= 'A' && c <= 'E'))
				code |= (c - 'A' + 10);
		}
	}
	
	// Country code detection complete.
	return code;
}

/**
 * Decode a Super Magic Drive interleaved block.
 * @param dest Destination block. (Must be 16 KB.)
 * @param src Source block. (Must be 16 KB.)
 */
void RomPrivate::DecodeSMDBlock(uint8_t *dest, const uint8_t *src)
{
	// First 8 KB of the source block is ODD bytes.
	const uint8_t *end_block = src + 8192;
	for (uint8_t *odd = dest + 1; src < end_block; odd += 16, src += 8) {
		*(odd +  0) = *(src + 0);
		*(odd +  2) = *(src + 1);
		*(odd +  4) = *(src + 2);
		*(odd +  6) = *(src + 3);
		*(odd +  8) = *(src + 4);
		*(odd + 10) = *(src + 5);
		*(odd + 12) = *(src + 6);
		*(odd + 14) = *(src + 7);
	}

	// Second 8 KB of the source block is EVEN bytes.
	end_block = src + 8192;
	for (uint8_t *even = dest; src < end_block; even += 16, src += 8) {
		*(even +  0) = *(src + 0);
		*(even +  2) = *(src + 1);
		*(even +  4) = *(src + 2);
		*(even +  6) = *(src + 3);
		*(even +  8) = *(src + 4);
		*(even + 10) = *(src + 5);
		*(even + 12) = *(src + 6);
		*(even + 14) = *(src + 7);
	}
}

/**
 * Load the ROM header from the selected ROM file.
 * @param sysOverride System override.
 * @param fmtOverride Format override.
 * @return 0 on success; non-zero on error.
 */
int RomPrivate::loadRomHeader(Rom::MDP_SYSTEM_ID sysOverride, Rom::RomFormat fmtOverride)
{
	if (!z_entry_sel) {
		// No file selected!
		return -1;
	}

	// Save the override values as the detected system and ROM format.
	// If specified as UNKNOWN, they will be detected later.
	sysId = sysOverride;
	romFormat = fmtOverride;
	
	// Get the ROM size.
	// TODO: Save the internal filename for multi-file archives.
	romSize = z_entry_sel->filesize;

	// Load the ROM header for detection purposes.
	static const size_t ROM_HEADER_SIZE = 65536+512;
	uint8_t *header = (uint8_t*)malloc(ROM_HEADER_SIZE);
	if (!header) {
		// Memory allocation error.
		// TODO: Error code constants.
		return -2;
	}

	size_t header_size;
	int ret = decomp->getFile(z_entry_sel, header, ROM_HEADER_SIZE, &header_size);
	if (ret != 0 || header_size == 0) {
		// File read error.
		// TODO: Error code constants.
		free(header);
		return -3;
	}

	// If the header size is smaller than the header buffer,
	// clear the rest of the header buffer.
	if (header_size < sizeof(header)) {
		memset(&header[header_size], 0x00, (ROM_HEADER_SIZE - header_size));
	}

	// Detect the ROM format first.
	if (romFormat == Rom::RFMT_UNKNOWN) {
		romFormat = DetectFormat(header, header_size, romSize);
	}

	// Adjust the ROM size for certain interleaved ROM formats.
	switch (romFormat) {
		case Rom::RFMT_SMD:
		case Rom::RFMT_SMD_SPLIT: {
			// Super Magic Drive.
			// NOTE: Split SMD isn't supported, but it has
			// the 512-byte header, so we should handle it
			// the same as regular SMD for now.
			if (romSize > 512) {
				romSize -= 512;
			}

			// Deinterleave the ROM header.
			// Note that the actual SMD data starts at byte 512.
			static const size_t BIN_HEADER_SIZE = 65536;
			uint8_t *smd_header = header;
			header = (uint8_t*)malloc(BIN_HEADER_SIZE);
			header_size = BIN_HEADER_SIZE;
			// TODO: Use pointer arithmetic?
			for (size_t i = 0; i < BIN_HEADER_SIZE; i += 16384) {
				DecodeSMDBlock(&header[i], &smd_header[i + 512]);
			}
			free(smd_header);
		}

		case Rom::RFMT_MGD:
			// Multi Game Doctor.
			// TODO: Need to read data from the second half of the ROM...
			break;

		default:
			break;
	}

	// Detect the system ID after adjusting for interleaved formats.
	if (sysId == Rom::MDP_SYSTEM_UNKNOWN) {
		sysId = DetectSystem(header, header_size, romFormat);
	}

	// TODO: If it's an MD ROM over 32 MB, return an error.

	/** MD-only stuff here. **/

	// Load the ROM header information.
	readHeaderMD(header, header_size);

	// ROM header loaded.
	free(header);
	return 0;
}

/**
 * Read the Mega Drive ROM header.
 * @param header ROM header.
 * @param header_size ROM header size.
 */
void RomPrivate::readHeaderMD(const uint8_t *header, size_t header_size)
{
	// Clear the internal MD header.
	memset(&m_mdHeader, 0x00, sizeof(m_mdHeader));

	if (header_size <= 0x100) {
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

	// Detect the ROM's region code.
	regionCode = DetectRegionCodeMD(m_mdHeader.countryCodes);

	// Attempt to convert the Domestic ROM header name from Shift-JIS to UTF-8.
	string header_sjis = string(m_mdHeader.romNameJP, sizeof(m_mdHeader.romNameJP));
	string header_utf8 = LibGensText::SJIS_to_Utf8(header_sjis);

	if (!header_utf8.empty()) {
		// Domestic ROM header name has been converted from Shift-JIS to UTF-8.
		romNameJP = LibGensText::SpaceElim(header_utf8);
	} else {
		// Domestic ROM header name was not converted.
		// Use it as-is.
		// TODO: Remove characters with the high bit set?
		romNameJP = LibGensText::SpaceElim(string(m_mdHeader.romNameJP, sizeof(m_mdHeader.romNameJP)));
	}

	// Attempt to convert the Overseas ROM header name from Shift-JIS to UTF-8.
	// (Columns uses Shift-JIS for both fields.)
	header_sjis = string(m_mdHeader.romNameUS, sizeof(m_mdHeader.romNameUS));
	header_utf8 = LibGensText::SJIS_to_Utf8(header_sjis);

	if (!header_utf8.empty()) {
		// Overseas ROM header name has been converted from Shift-JIS to UTF-8.
		romNameUS = LibGensText::SpaceElim(header_utf8);
	} else {
		// Overseas ROM header name was not converted.
		// Use it as-is.
		// TODO: Remove characters with the high bit set?
		romNameUS = LibGensText::SpaceElim(string(m_mdHeader.romNameUS, sizeof(m_mdHeader.romNameUS)));
	}
}

/** Rom class. **/

/**
 * Initialize a new RomPrivate object.
 * @param filename ROM filename.
 * @param sysOverride System override.
 * @param fmtOverride ROM format override.
 */
Rom::Rom(const utf8_str *filename, MDP_SYSTEM_ID sysOverride, RomFormat fmtOverride)
	: d(new RomPrivate(this, filename, sysOverride, fmtOverride))
{ }

/**
 * Initialize a new RomPrivate object.
 * @param rom_data ROM data in memory. (NOTE: Must remain valid as long as Rom is open.)
 * @param rom_size Size of rom_data.
 * @param sysOverride System override.
 * @param fmtOverride ROM format override.
 */
Rom::Rom(const uint8_t *rom_data, unsigned int rom_size, MDP_SYSTEM_ID sysOverride, RomFormat fmtOverride)
	: d(new RomPrivate(this, rom_data, rom_size, sysOverride, fmtOverride))
{ }

Rom::~Rom()
{
	delete d;
}

/**
 * Load the ROM image into a buffer.
 * @param buf Buffer.
 * @param siz Buffer size.
 * For most ROM formats, siz should be equal to the ROM size.
 * For SMD format, siz should be ROM size + 512, since the header
 * is read into the buffer as well. It's removed afterwards.
 * @return Positive value indicating amount of data read on success; 0 or negative on error.
 * TODO: Error code constants!
 */
int Rom::loadRom(void *buf, size_t siz)
{
	// TODO: Use error code constants.
	assert(buf);
	if (!isOpen())
		return -1;	// File is closed!
	else if (!d->decomp)
		return -2;	// Decompressor error!
	else if (!d->z_entry_sel)
		return -3;	// No file selected!

	if (siz == 0 || siz < d->romSize) {
		// ROM buffer isn't large enough for the ROM image.
		return -4;
	}

	// Load the ROM image.
	// TODO: Error handling.
	size_t ret_siz = 0;
	switch (d->romFormat) {
		case Rom::RFMT_BINARY:
			// Plain binary ROM file.
 			d->decomp->getFile(d->z_entry_sel, buf, siz, &ret_siz);
			break;

		case RFMT_SMD:
		case RFMT_SMD_SPLIT: {
			// TODO: Split SMD isn't supported.
			// Handling it as plain SMD for now.
			if (siz < (d->romSize + 512)) {
				// Not enough space for the SMD header.
				return -4;
			}

			// Read the SMD data.
			d->decomp->getFile(d->z_entry_sel, buf, siz, &ret_siz);
			if (ret_siz <= 512) {
				// ROM is too small.
				ret_siz = 0;
				break;
			} else {
				// Skip the header.
				ret_siz -= 512;
			}

			// Temporary SMD block buffer.
			uint8_t *smd_block = (uint8_t*)malloc(16384);

			// Process 16 KB blocks.
			// NOTE: If ret_siz isn't a multiple of 16 KB,
			// the last block will not be decoded properly.
			size_t remain = ret_siz;
			const uint8_t *buf_read = &((uint8_t*)buf)[512];
			uint8_t *buf_write = (uint8_t*)buf;
			for (; remain >= 16384; remain -= 16384, buf_read += 16384, buf_write += 16384) {
				memcpy(smd_block, buf_read, 16384);
				d->DecodeSMDBlock(buf_write, smd_block);
			}

			if (remain > 0) {
				// SMD isn't a multiple of 16 KB.
				// The last block will not be decoded properly.
				// (...or at all.)
				memmove(buf_write, buf_read, remain);
			}

			free(smd_block);
			break;
               }

		default:
			// Unsupported ROM format.
			return -5;
	}

	// Calculate the CRC32.
	// TODO: Also MD5?
	d->rom_crc32 = crc32(0, (const Bytef*)buf, siz);

	// Return the number of bytes read.
	return ret_siz;
}

/**
 * Property accessors.
 */

/**
 * Check if the ROM file is open.
 * @return True if the ROM file is open; false if not.
 */
bool Rom::isOpen(void) const
{
	// NOTE: We're checking decomp, since DcMemFake is
	// memory-backed and doesn't have an actual file.
	return (d->decomp != nullptr);
}

/**
 * Close the opened ROM file.
 */
void Rom::close(void)
{
	if (d->file) {
		fclose(d->file);
		d->file = NULL;
	}
}

/**
 * Get the ROM system ID.
 * @return ROM system ID.
 */
Rom::MDP_SYSTEM_ID Rom::sysId(void) const
	{ return d->sysId; }

/**
 * Get the ROM format.
 * @return ROM format.
 */
Rom::RomFormat Rom::romFormat(void) const
	{ return d->romFormat; }

/**
 * Get the ROM size.
 * @return ROM size, or 0 on error.
 */
int Rom::romSize(void) const
	{ return d->romSize; }

/**
 * Get the ROM filename.
 * @return ROM filename (UTF-8), or empty string on error.
 */
string Rom::filename(void) const
	{ return d->filename; }

/**
 * Get the ROM filename.
 * (Basename, no extension.)
 * @return ROM filename (UTF-8), or nullptr on error.
 */
string Rom::filename_baseNoExt(void) const
	{ return d->filename_baseNoExt; }

/**
 * Get the ROM filename of the selected file in a multi-file archive.
 * @return ROM filename (UTF-8), or empty string on error.
 */
string Rom::z_filename(void) const
{
	// TODO: Cache it?
	if (d->z_entry_sel && d->z_entry_sel->filename) {
		return string(d->z_entry_sel->filename);
	}
	return string();
}

/**
 * Get the ROM filename of the selected file in a multi-file archive.
 * (Basename, no extension.)
 * @return ROM filename (UTF-8), or empty string on error.
 */
string Rom::z_filename_baseNoExt(void) const
{
	// TODO: Cache it?
	string tmp = z_filename();
	if (!tmp.empty()) {
		tmp = LibGensText::FilenameBaseNoExt(tmp);
	}
	return tmp;
}

/**
 * Get the Japanese (domestic) ROM name.
 * @return Japanese (domestic) ROM name (UTF-8), or empty string on error.
 */
string Rom::romNameJP(void) const
	{ return d->romNameJP; }

/**
 * Get the American (overseas) ROM name.
 * @return American (overseas) ROM name (UTF-8), or empty string on error.
 */
string Rom::romNameUS(void) const
	{ return d->romNameUS; }

/**
 * Get the ROM checksum.
 * TODO: This is MD only for now...
 * @return ROM checksum.
 */
uint16_t Rom::checksum(void) const
	{ return d->m_mdHeader.checksum; }

/**
 * Get the ROM's CRC32.
 * NOTE: loadRom() must be called before using this function;
 * otherwise, it will return 0.
 * @return ROM CRC32.
 */
uint32_t Rom::rom_crc32(void) const
	{ return d->rom_crc32; }

/**
 * Get the ROM's serial number.
 * TODO: This is MD only for now...
 * @return ROM serial number.
 */
std::string Rom::rom_serial(void) const
	{ return std::string(d->m_mdHeader.serialNumber, sizeof(d->m_mdHeader.serialNumber)); }

/**
 * Get the ROM's SRAM information.
 * @param sramInfo	[out, opt] SRAM info field.
 * @param sramStartAddr	[out, opt] SRAM start address.
 * @param sramEndAddr	[out, opt] SRAM end address.
 * @return 0 on success; non-zero on error.
 */
int Rom::romSramInfo(uint32_t *sramInfo, uint32_t *sramStartAddr, uint32_t *sramEndAddr) const
{
	// TODO: Return non-zero if the ROM wasn't loaded at all.
	if (sramInfo) {
		*sramInfo = d->m_mdHeader.sramInfo;
	}
	if (sramStartAddr) {
		*sramStartAddr = d->m_mdHeader.sramStartAddr;
	}
	if (sramEndAddr) {
		*sramEndAddr = d->m_mdHeader.sramEndAddr;
	}
	return 0;
}

/**
 * Get the region code. (MD hex format)
 * TODO: Change to uint8_t?
 * @return ROM region code. (MD hex format)
 */
int Rom::regionCode(void) const
	{ return d->regionCode; }

/** Multi-file ROM archive support. **/

/**
 * Determine if the loaded ROM archive has multiple files.
 * @return True if the ROM archive has multiple files; false if it doesn't.
 */
bool Rom::isMultiFile(void) const
	{ return d->isMultiFile(); }

/**
 * Get the list of files in the ROM archive.
 * @return List of files in the ROM archive, or nullptr on error.
 */
const mdp_z_entry_t *Rom::get_z_entry_list(void) const
	{ return d->z_entry_list; }

/**
 * Select a file from a multi-file ROM archive to load.
 * @param sel File to load.
 * @return 0 on success; non-zero on error.
 */
int Rom::select_z_entry(const mdp_z_entry_t *sel)
{
	if (!isOpen())
		return -1;	// File is closed!
	if (!isMultiFile())
		return -2;	// Not multi-file.
	if (d->z_entry_sel != nullptr)
		return -3;	// ROM was already selected.

	// TODO: Verify that sel is actually in m_z_entry_list.
	if (!sel)
		return -4;	// Invalid selection.

	// Select the ROM from the archive.
	d->z_entry_sel = sel;

	// Load the ROM header.
	d->loadRomHeader(d->sysId_override, d->romFormat_override);
	return 0;
}

/**
 * Determine if a ROM has been selected.
 * NOTE: This is only correct if the ROM file hasn't been closed.
 * @return True if a ROM has been selected.
 */
bool Rom::isRomSelected(void) const
	{ return (d->z_entry_sel != nullptr); }

}
