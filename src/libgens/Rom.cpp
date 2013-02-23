/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Rom.cpp: ROM loader.                                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

// LibGens includes.
#include "Util/byteswap.h"
#include "macros/common.h"
#include "lg_osd.h"

// Needed for checking CRC32s for "Xin Qi Gai Wang Zi" (Beggar Prince).
#include <zlib.h>

namespace LibGens
{

/**
 * Rom private class.
 */
class RomPrivate
{
	public:
		RomPrivate(Rom *q, const utf8_str *filename,
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
		// Space elimination algorithm.
		static inline FUNC_PURE bool IsGraphChar(uint16_t wchr);
		static std::string SpaceElim(const std::string& src);

		// System ID and ROM format detection functions.
		static Rom::RomFormat DetectFormat(const uint8_t *header, size_t header_size);
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
		std::string filenameBaseNoExt;	// ROM filename. (basename; no extension)
		unsigned int romSize;		// ROM size.
		int eprType;			// EEPRom type.

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

		/** ROM header functions. **/
		int loadRomHeader(Rom::MDP_SYSTEM_ID sysOverride, Rom::RomFormat fmtOverride);
		void readHeaderMD(const uint8_t *header, size_t header_size);

		/**
		 * Mega Drive ROM header.
		 * This matches the MD ROM header format exactly.
		 * 
		 * NOTE: Strings are NOT null-terminated!
		 */
		struct MD_RomHeader
		{
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

		/**
		 * ROM fixups table entry. (Mega Drive)
		 */
		struct MD_RomFixup
		{
			// ROM identification.
			// If any value is 0 or nullptr, that field is ignored.
			struct
			{
				const char *serial;	// ROM serial number.
				uint16_t checksum;	// MD checksum.
				uint32_t crc32;		// CRC32.
			} id;

			// Fixups.
			Rom::Mapper mapper;		// ROM mapper.

			// SRAM fixups.
			// If any value is 0, that field is ignored.
			struct
			{
				uint32_t start_addr;	// SRAM start address.
				uint32_t end_addr;	// SRAM end address.
				bool force_off;		// Force SRAM off. (Puggsy)
			} sram;
		};

		static const MD_RomFixup MD_RomFixups[5];

		uint32_t rom_crc32;	// ROM CRC32.
		int romFixup;		// ROM fixup index. (-1 == no fixups)
		Rom::Mapper mapper;	// ROM mapper.

		static int CheckRomFixupsMD(const MD_RomHeader *mdRomHeader, uint32_t crc32);
};


/**
 * ROM fixup table. (Mega Drive)
 */
const RomPrivate::MD_RomFixup RomPrivate::MD_RomFixups[5] =
{
	// Puggsy: Shows an anti-piracy message after the third level if SRAM is detected.
	{{"GM T-113016", 0, 0}, Rom::MAPPER_STANDARD, {0, 0, true}},
	{{"GM T-550055", 0, 0}, Rom::MAPPER_STANDARD, {0, 0, true}},	// Puggsy (Beta)

	// Psy-O-Blade: Incorrect SRAM header.
	{{"GM T-26013 ", 0, 0}, Rom::MAPPER_STANDARD, {0x200000, 0x203FFF, false}},

	/**
	 * Xin Qi Gai Wang Zi (original version of Beggar Prince):
	 * SRAM is located at 0x400000-0x40FFFF; ROM header is invalid.
	 * 
	 * CRC32s:
	 * - Xin Qi Gai Wang Zi (Ch).gen:	DD2F38B5
	 * - Xin Qi Gai Wang Zi (Ch) [a1].gen:	DA5A4BFE
	 */
	{{nullptr, 0, 0xDD2F38B5}, Rom::MAPPER_STANDARD, {0x400000, 0x40FFFF, false}},
	{{nullptr, 0, 0xDA5A4BFE}, Rom::MAPPER_STANDARD, {0x400000, 0x40FFFF, false}},
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
	, eprType(-1)
	, regionCode(0)
	, rom_crc32(0)
	, romFixup(-1)
	, mapper(Rom::MAPPER_STANDARD)
{
	// If filename is nullptr, don't do anything else.
	if (!filename)
		return;

	// Save the filename for later.
	this->filename = string(filename);

	// Remove the directories and extension from the ROM filename.
	// TODO: Remove all extensions (e.g. ".gen.gz")?
	string tmpFilename = this->filename;

	// Get the filename portion.
	size_t dirSep = tmpFilename.rfind(LG_PATH_SEP_CHR);
	if (dirSep != string::npos)
		tmpFilename.erase(0, dirSep+1);

	// Remove the file extension.
	size_t extSep = tmpFilename.rfind('.');
	if (extSep != string::npos)
		tmpFilename.erase(extSep, (tmpFilename.size() - extSep));

	// Save the truncated filename.
	filenameBaseNoExt = tmpFilename;

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
 * Destroy the RomPrivate object.
 */
RomPrivate::~RomPrivate()
{
	// Free the mdp_z_entry_t list.
	Decompressor::z_entry_t_free(z_entry_list);

	// Delete the decompressor.
	delete decomp;

	// If the file is open, close it.
	if (file)
		fclose(file);
}


/**
 * Determine if a character is a graphics character.
 * @param wchr Character to check.
 * @return True if this is a graphics character; false otherwise.
 */
inline FUNC_PURE bool RomPrivate::IsGraphChar(uint16_t wchr)
{
	// TODO: Figure out why iswgraph() and iswspace() are useless.
	
	if (wchr < 0x7F)
		return isgraph(wchr);
	else if (wchr == 0x3000)
	{
		// U+3000: IDEOGRAPHIC SPACE
		// Used in "Columns"' ROM headers.
		return false;
	}
	
	// Assume graphical character by default.
	return true;
}


/**
 * Eliminate excess spaces from a ROM name.
 * TODO: Returning an std::string is a bit wasteful...
 * TODO: Move to a separate string handling class.
 * @param src ROM name. (UTF-8)
 * @return ROM name with excess spaces eliminated. (UTF-8)
 */
std::string RomPrivate::SpaceElim(const string& src)
{
	// Convert the string to UTF-16 first.
	// TODO: Check for invalid UTF-8 sequences and handle them as cp1252?
	u16string wcs_src = LibGensText::Utf8_to_Utf16(src);
	if (wcs_src.empty()) {
		// Error converting the string. Assume the string is ASCII.
		wcs_src.resize(src.size());
		for (size_t i = 0; i < src.size(); i++) {
			wcs_src[i] = (src[i] & 0x7F);
		}
	}

	// Allocate the destination string. (UTF-16)
	u16string wcs_dest(src.size(), 0);
	int i_dest = 0;

	// Was the last character a graphics character?
	bool lastCharIsGraph = false;

	// Process the string.
	for (size_t i = 0; i < wcs_src.size(); i++) {
		char16_t wchr = wcs_src[i];
		if (!lastCharIsGraph && !IsGraphChar(wchr)) {
			// This is a space character, and the previous
			// character was not a space character.
			continue;
		}

		// This is not a space character,
		// or it is a space character and the previous character wasn't.
		wcs_dest[i_dest++] = wchr;
		lastCharIsGraph = IsGraphChar(wchr);
	}

	if (i_dest == 0) {
		// Empty string.
		return string();
	}

	// Make sure there's no space at the end of the string.
	if (!IsGraphChar(wcs_dest[i_dest - 1]))
		wcs_dest.resize(i_dest - 2 + 1);
	else
		wcs_dest.resize(i_dest - 1 + 1);

	// Convert the string back to UTF-8.
	return LibGensText::Utf16_to_Utf8(wcs_dest);
}


/**
 * Detect a ROM's format.
 * @param header ROM header.
 * @param header_size ROM header size.
 * @return ROM format.
 */
Rom::RomFormat RomPrivate::DetectFormat(const uint8_t *header, size_t header_size)
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
					return Rom::RFMT_SMD;
				else
					return Rom::RFMT_SMD_SPLIT;
			}
		}
	}
	
	/** MGD-format (.MD) ROM check. */
	if (header_size >= 0x100)
	{
		// MGD format is interleaved without blocks.
		// The odd bytes in the MD header are located at 0x80.
		if (header[0x80] == 'E' && header[0x81] == 'A')
		{
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
 * @param header ROM header.
 * @param header_size ROM header size.
 * @param fmt ROM format.
 * @return System ID.
 */
Rom::MDP_SYSTEM_ID RomPrivate::DetectSystem(const uint8_t *header, size_t header_size, Rom::RomFormat fmt)
{
	if (fmt >= Rom::RFMT_CD_CUE)
	{
		// CD-ROM. Assume Sega CD.
		// TODO: Sega CD 32X detection.
		return Rom::MDP_SYSTEM_MCD;
	}
	
	// TODO: SMS/GG/SG-1000 detection.
	
	if (fmt == Rom::RFMT_SMD && header_size >= 0x4200)
	{
		// SMD format check.
		if (header[0x0300] == 0xF9)
		{
			if ((header[0x0282] == '3' && header[0x0283] == 'X') ||
			    (header[0x0407] == 'A' && header[0x0408] == 'S'))
			{
				// 32X ROM.
				return Rom::MDP_SYSTEM_32X;
			}
		}
		else
		{
			// Assume MD.
			return Rom::MDP_SYSTEM_MD;
		}
	}
	else if (fmt == Rom::RFMT_MGD && header_size >= 0x200)
	{
		// MGD format check.
		if (header[0x100] == 0xF9)
		{
			if ((header[0x0082] == '3' && header[0x0083] == 'X') ||
			    (header[0x0207] == 'A' && header[0x0208] == 'S'))
			{
				// 32X ROM.
				return Rom::MDP_SYSTEM_32X;
			}
		}
		else
		{
			// Assume MD.
			return Rom::MDP_SYSTEM_MD;
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
				return Rom::MDP_SYSTEM_32X;
			}
		}
		else
		{
			// Assume MD.
			return Rom::MDP_SYSTEM_MD;
		}
	}
	
	// If all else fails, assume MD.
	// TODO: Add support for SMS, GG, Pico, etc.
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
 * Load the ROM header from the selected ROM file.
 * @param sysOverride System override.
 * @param fmtOverride Format override.
 * @return 0 on success; non-zero on error.
 */
int RomPrivate::loadRomHeader(Rom::MDP_SYSTEM_ID sysOverride, Rom::RomFormat fmtOverride)
{
	if (!z_entry_sel)
	{
		// No file selected!
		return -1;
	}
	
	// Save the override values as the detected system and ROM format.
	// If specified as UNKNOWN, they will be detected later.
	sysId = sysOverride;
	romFormat = fmtOverride;
	
	// Get the ROM size.
	// TODO: If it's an MD ROM over 6 MB, return an error.
	// TODO: Save the internal filename for multi-file archives.
	romSize = z_entry_sel->filesize;
	
	// Load the ROM header for detection purposes.
	uint8_t header[ROM_HEADER_SIZE];
	size_t header_size;
	int ret = decomp->getFile(z_entry_sel, header, sizeof(header), &header_size);
	if (ret != 0 || header_size == 0)
	{
		// File read error.
		// TODO: Error code constants.
		return -2;
	}
	
	// If the header size is smaller than the header buffer,
	// clear the rest of the header buffer.
	if (header_size < sizeof(header))
		memset(&header[header_size], 0x00, (sizeof(header) - header_size));
	
	if (romFormat == Rom::RFMT_UNKNOWN)
		romFormat = DetectFormat(header, header_size);
	if (sysId == Rom::MDP_SYSTEM_UNKNOWN)
		sysId = DetectSystem(header, header_size, romFormat);
	
	/** MD-only stuff here. **/
	
	// Load the ROM header information.
	readHeaderMD(header, header_size);
	
	// Check for ROM fixups.
	// TODO: CRC32 needs to be calculated here...
	// TODO: Move ROM fixup check to another function?
	romFixup = CheckRomFixupsMD(&m_mdHeader, rom_crc32);
	
	// Detect the EEPRom type from the ROM serial number and checksum.
	eprType = EEPRom::DetectEEPRomType(
			&m_mdHeader.serialNumber[3],
			(sizeof(m_mdHeader.serialNumber) - 3),
			m_mdHeader.checksum);
	
	// ROM header loaded.
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
		romNameJP = SpaceElim(header_utf8);
	} else {
		// Domestic ROM header name was not converted.
		// Use it as-is.
		// TODO: Remove characters with the high bit set?
		romNameJP = SpaceElim(string(m_mdHeader.romNameJP, sizeof(m_mdHeader.romNameJP)));
	}

	// Attempt to convert the Overseas ROM header name from Shift-JIS to UTF-8.
	// (Columns uses Shift-JIS for both fields.)
	header_sjis = string(m_mdHeader.romNameUS, sizeof(m_mdHeader.romNameUS));
	header_utf8 = LibGensText::SJIS_to_Utf8(header_sjis);

	if (!header_utf8.empty()) {
		// Overseas ROM header name has been converted from Shift-JIS to UTF-8.
		romNameUS = SpaceElim(header_utf8);
	} else {
		// Overseas ROM header name was not converted.
		// Use it as-is.
		// TODO: Remove characters with the high bit set?
		romNameUS = SpaceElim(string(m_mdHeader.romNameUS, sizeof(m_mdHeader.romNameUS)));
	}
}


/**
 * Check for ROM fixups. (Mega Drive)
 * @param mdRomHeader ROM header.
 * @param crc32 ROM CRC32.
 * @return Index in MD_RomFixups[], or -1 if no fixup is required.
 */
int RomPrivate::CheckRomFixupsMD(const RomPrivate::MD_RomHeader *mdRomHeader, uint32_t crc32)
{
	for (int i = 0; i < ARRAY_SIZE(MD_RomFixups); i++) {
		const MD_RomFixup *fixup = &MD_RomFixups[i];
		bool match = false;

		if (fixup->id.serial != nullptr) {
			// Compare the ROM serial number.
			if (strncmp(mdRomHeader->serialNumber, fixup->id.serial,
				sizeof(mdRomHeader->serialNumber)-3) != 0)
			{
				continue;
			}
			match = true;
		}

		if (fixup->id.crc32 != 0 && crc32 != 0) {
			// Compare the ROM CRC32.
			if (crc32 != fixup->id.crc32)
				continue;
			match = true;
		}

		if (fixup->id.checksum != 0) {
			// Compare the ROM checksum.
			if (mdRomHeader->checksum != fixup->id.checksum)
				continue;
			match = true;
		}

		// Found a fixup for this ROM.
		if (match)
			return i;
	}

	// No fixup found for this ROM.
	return -1;
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

Rom::~Rom()
	{ delete d; }

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
	
	// TODO; Move some of this to the SRam class?
	
	// SRam addresses.
	uint32_t start, end;
	
	// Check if the ROM header has SRam information.
	// Mask the SRam info value with 0xFFFF4000 and check
	// if it matches the Magic Number.
	// Magic Number: 0x52414000 ('R', 'A', 0x40, 0x00)
	if ((d->m_mdHeader.sramInfo & 0xFFFF4000) == 0x52414000)
	{
		// ROM header has SRam information. Use these addresses..
		// SRam starting position must be a multiple of 0xF80000.
		// TODO: Is that really necessary?
		start = d->m_mdHeader.sramStartAddr & 0xF80000;
		end = d->m_mdHeader.sramEndAddr & 0xFFFFFF;
	}
	else
	{
		// ROM header does not have SRam information.
		// Use default settings.
		start = 0x200000;
		end = 0x20FFFF;	// 64 KB
	}
	
	// Check for invalid SRam addresses.
	if ((start > end) || ((end - start) > 0xFFFF))
	{
		// Invalid ending address.
		// Set the end address to the start + 0xFFFF.
		end = start + 0xFFFF;
	}
	
	// Make sure SRam starts on an even byte and ends on an odd byte.
	start &= ~1;
	end |= 1;
	
	/**
	 * If the ROM is smaller than the SRam starting address, always enable SRam.
	 * Notes:
	 * - HardBall '95: SRAM is at $300000; ROM is 3 MB; cartridge does NOT have $A130F1 register.
	 *                 Need to enable SRAM initially; otherwise, an error appears on startup.
	 */
	const bool enableSRam = (d->romSize <= start);
	sram->setOn(enableSRam);
	sram->setWrite(enableSRam);
	
	// Check if a ROM fixup needs to be applied.
	if (d->romFixup >= 0)
	{
		// Apply a ROM fixup.
		const RomPrivate::MD_RomFixup *fixup = &RomPrivate::MD_RomFixups[d->romFixup];
		
		if (fixup->sram.force_off)
		{
			// Force SRAM off.
			sram->setOn(false);
			sram->setWrite(false);
			sram->setStart(1);
			sram->setEnd(0);
			return 0;
		}
		
		// Fix SRAM start/end addresses.
		if (fixup->sram.start_addr != 0)
			start = fixup->sram.start_addr;
		if (fixup->sram.end_addr != 0)
			end = fixup->sram.end_addr;
	}
	
	// Set the addresses.
	sram->setStart(start);
	sram->setEnd(end);
	
	// Load the SRam file.
	// TODO: Use internal filename for multi-file?
	sram->setFilename(d->filename);
	return sram->load();
}


/**
 * initEEPRom(): Initialize an EEPRom class using the ROM's header information.
 * @param eeprom Pointer to EEPRom class.
 * @return Positive value indicating EEPRom size on success; negative on error.
 */
int Rom::initEEPRom(EEPRom *eeprom) const
{
	// TODO: Load EEPRom from a file.
	// TODO: Should that be implemented here or in SRam.cpp?
	if (!isOpen())
		return -1;
	
	// Reset the EEPRom and set the type.
	eeprom->reset();
	eeprom->setEEPRomType(d->eprType);
	
	// Don't do anything if the ROM isn't in the EEPRom database.
	if (d->eprType < 0)
		return 0;
	
	// Load the EEProm file.
	// TODO: Use internal filename for multi-file?
	eeprom->setFilename(d->filename);
	return eeprom->load();
}


/**
 * Load the ROM image into a buffer.
 * @param buf Buffer.
 * @param siz Buffer size.
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

	if (d->romFormat != RFMT_BINARY) {
		// Unsupported ROM format.
		return -5;
	}

	// Load the ROM image.
	// TODO: Error handling.
	size_t ret_siz = 0;
	d->decomp->getFile(d->z_entry_sel, buf, siz, &ret_siz);
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
	{ return (d->file != nullptr); }

/**
 * Close the opened ROM file.
 */
void Rom::close(void)
{
	if (d->file)
	{
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
 * @return ROM filename (UTF-8), or nullptr on error.
 */
const utf8_str *Rom::filename(void) const
	{ return d->filename.c_str(); }

/**
 * Get the ROM filename. (basename, no extension)
 * @return ROM filename (UTF-8), or nullptr on error.
 */
const utf8_str *Rom::filenameBaseNoExt(void) const
	{ return d->filenameBaseNoExt.c_str(); }

/**
 * Get the Japanese (domestic) ROM name.
 * @return Japanese (domestic) ROM name (UTF-8), or nullptr on error.
 */
const utf8_str *Rom::romNameJP(void) const
	{ return d->romNameJP.c_str(); }

/**
 * Get the American (overseas) ROM name.
 * @return American (overseas) ROM name (UTF-8), or nullptr on error.
 */
const utf8_str *Rom::romNameUS(void) const
	{ return d->romNameUS.c_str(); }

/**
 * Get the ROM checksum.
 * TODO: This is MD only for now...
 * @return ROM checksum.
 */
uint16_t Rom::checksum(void) const
	{ return d->m_mdHeader.checksum; }

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
