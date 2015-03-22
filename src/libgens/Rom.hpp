/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Rom.hpp: ROM loader.                                                    *
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

#ifndef __LIBGENS_ROM_HPP__
#define __LIBGENS_ROM_HPP__

// C includes. (C++ namespace)
#include <stdint.h>

// C++ includes.
#include <string>

// Decompressor subsystem.
#include "Decompressor/Decompressor.hpp"

// SysVersion::RegionCode_t
// TODO: Move somewhere else.
#include "MD/SysVersion.hpp"

/**
 * ROM_HEADER_SIZE: Number of bytes used for ROM type detection.
 */
#define ROM_HEADER_SIZE 65536

namespace LibGens {

class RomPrivate;

class Rom
{
	public:
		// TODO: Use MDP headers for system identifiers.
		enum MDP_SYSTEM_ID {
			/*! BEGIN: MDP v1.0 system IDs. !*/
			MDP_SYSTEM_UNKNOWN = 0,
			MDP_SYSTEM_MD      = 1,
			MDP_SYSTEM_MCD     = 2,
			MDP_SYSTEM_32X     = 3,
			MDP_SYSTEM_MCD32X  = 4,
			MDP_SYSTEM_SMS     = 5,
			MDP_SYSTEM_GG      = 6,
			MDP_SYSTEM_SG1000  = 7,
			MDP_SYSTEM_PICO    = 8,
			/*! END: MDP v1.0 system IDs. !*/
			
			MDP_SYSTEM_MAX
		};

		enum RomFormat {
			RFMT_UNKNOWN = 0,

			RFMT_BINARY,		// Plain binary ROM image.
			RFMT_SMD,		// Interleaved ROM image from Super Magic Drive.
			RFMT_SMD_SPLIT,		// Multi-part SMD image. (Probably won't be supported.)
			RFMT_MGD,		// Interleaved ROM image from Multi Game Doctor.

			// TODO: CD-ROM image handling.
			RFMT_CD_CUE,		// CD-ROM image, CUE sheet.
			RFMT_CD_ISO_2048,	// CD-ROM image, ISO-9660 format. (2048-byte sectors)
			RFMT_CD_ISO_2352,	// CD-ROM image, ISO-9660 format. (2352-byte sectors)
			RFMT_CD_BIN_2048,	// CD-ROM image, BIN format. (2048-byte sectors)
			RFMT_CD_BIN_2352,	// CD-ROM image, BIN format. (2352-byte sectors)
		};

		Rom(const utf8_str *filename, MDP_SYSTEM_ID sysOverride = MDP_SYSTEM_UNKNOWN, RomFormat fmtOverride = RFMT_UNKNOWN);
		~Rom();

	private:
		friend class RomPrivate;
		RomPrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Rom(const Rom &);
		Rom &operator=(const Rom &);

	public:
		/**
		 * Check if the ROM file is open.
		 * @return True if the ROM file is open; false if not.
		 */
		bool isOpen(void) const;

		/**
		 * Close the opened ROM file.
		 */
		void close(void);

		/**
		 * Get the ROM system ID.
		 * @return ROM system ID.
		 */
		MDP_SYSTEM_ID sysId(void) const;

		/**
		 * Get the ROM format.
		 * @return ROM format.
		 */
		RomFormat romFormat(void) const;

		/**
		 * Get the ROM size.
		 * @return ROM size, or 0 on error.
		 */
		int romSize(void) const;

		/**
		 * Load the ROM image into a buffer.
		 * @param buf Buffer.
		 * @param siz Buffer size.
		 * @return Positive value indicating amount of data read on success; 0 or negative on error.
		 */
		int loadRom(void *buf, size_t siz);

		/**
		 * Get the ROM filename.
		 * @return ROM filename (UTF-8), or empty string on error.
		 */
		std::string filename(void) const;

		/**
		 * Get the ROM filename.
		 * (Basename, no extension)
		 * @return ROM filename (UTF-8), or empty string on error.
		 */
		std::string filenameBaseNoExt(void) const;

		/********************
		 * ROM header data. *
		 ********************/

		/**
		 * Get the Japanese (domestic) ROM name.
		 * @return Japanese (domestic) ROM name (UTF-8), or empty string on error.
		 */
		std::string romNameJP(void) const;

		/**
		 * Get the American (overseas) ROM name.
		 * @return American (overseas) ROM name (UTF-8), or empty string on error.
		 */
		std::string romNameUS(void) const;

		/**
		 * Get the ROM checksum.
		 * TODO: This is MD only for now...
		 * @return ROM checksum.
		 */
		uint16_t checksum(void) const;

		/**
		 * Get the ROM's CRC32.
		 * NOTE: loadRom() must be called before using this function;
		 * otherwise, it will return 0.
		 * @return ROM CRC32.
		 */
		uint32_t rom_crc32(void) const;

		/**
		 * Get the ROM's serial number.
		 * TODO: This is MD only for now...
		 * @return ROM serial number.
		 */
		std::string rom_serial(void) const;

		/**
		 * Get the ROM's SRAM information.
		 * @param sramInfo	[out, opt] SRAM info field.
		 * @param sramStartAddr	[out, opt] SRAM start address.
		 * @param sramEndAddr	[out, opt] SRAM end address.
		 * @return 0 on success; non-zero on error.
		 */
		int romSramInfo(uint32_t *sramInfo, uint32_t *sramStartAddr, uint32_t *sramEndAddr) const;

		/**
		 * Get the region code. (MD hex format)
		 * TODO: Change to uint8_t?
		 * @return ROM region code. (MD hex format)
		 */
		int regionCode(void) const;

		/** Multi-file ROM archive support. **/

		/**
		 * Determine if the loaded ROM archive has multiple files.
		 * @return True if the ROM archive has multiple files; false if it doesn't.
		 */
		bool isMultiFile(void) const;

		/**
		 * Get the list of files in the ROM archive.
		 * @return List of files in the ROM archive, or nullptr on error.
		 */
		const mdp_z_entry_t *get_z_entry_list(void) const;

		/**
		 * Select a file from a multi-file ROM archive to load.
		 * @param sel File to load.
		 * @return 0 on success; non-zero on error.
		 */
		int select_z_entry(const mdp_z_entry_t *sel);

		/**
		 * Determine if a ROM has been selected.
		 * NOTE: This is only correct if the ROM file hasn't been closed.
		 * @return True if a ROM has been selected.
		 */
		bool isRomSelected(void) const;

		/**
		 * Mappers.
		 */
		enum Mapper {
			/**
			 * Standard MD mapper.
			 * Can represent either of the following:
			 * - No mapper. (<= 4 MB)
			 * - Super Street Fighet 2 mapper. (> 4 MB)
			 * - Flat addressing. (> 4 MB, <= 10 MB)
			 */
			MAPPER_STANDARD = 0,

			/**
			 * Gamtec read-only copy protection mapper.
			 * Has read-only registers above $400000.
			 * Used in 777 Casino.
			 */
			MAPPER_GAMTEC_REG_RO = 1,

			/**
			 * Gamtec read-write copy protection mapper.
			 * Has writable registers above $400000.
			 */
			MAPPER_GAMTEC_REG_RW = 2,
		};
};

}

#endif /* __LIBGENS_ROM_HPP__ */
