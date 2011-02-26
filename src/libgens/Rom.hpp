/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Rom.hpp: ROM loader.                                                    *
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

#ifndef __LIBGENS_ROM_HPP__
#define __LIBGENS_ROM_HPP__

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

// C++ includes.
#include <string>

// LibGens includes.
#include "Save/SRam.hpp"
#include "Save/EEPRom.hpp"
#include "macros/common.h"

// Decompressor subsystem.
#include "Decompressor/Decompressor.hpp"

/**
 * ROM_HEADER_SIZE: Number of bytes used for ROM type detection.
 */
#define ROM_HEADER_SIZE 65536

namespace LibGens
{

class Rom
{
	public:
		// TODO: Use MDP headers for system identifiers.
		enum MDP_SYSTEM_ID
		{
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
		
		enum RomFormat
		{
			RFMT_UNKNOWN = 0,
			
			RFMT_BINARY,		// Plain binary ROM image.
			RFMT_SMD,		// Interleaved ROM image from Super Magic Drive.
			RFMT_SMD_SPLIT,		// Multi-part SMD image. (Probably won't be supported.)
			RFMT_MGD,		// Interleaved ROM image from Multi-Game-Doctor.
			
			// TODO: CD-ROM image handling.
			RFMT_CD_CUE,		// CD-ROM image, CUE sheet.
			RFMT_CD_ISO_2048,	// CD-ROM image, ISO-9660 format. (2048-byte sectors)
			RFMT_CD_ISO_2352,	// CD-ROM image, ISO-9660 format. (2048-byte sectors)
			RFMT_CD_BIN_2048,	// CD-ROM image, BIN format. (2352-byte sectors)
			RFMT_CD_BIN_2352,	// CD-ROM image, BIN format. (2352-byte sectors)
		};
		
		Rom(const utf8_str *filename, MDP_SYSTEM_ID sysOverride = MDP_SYSTEM_UNKNOWN, RomFormat fmtOverride = RFMT_UNKNOWN);
		~Rom();
		
		bool isOpen(void) const { return (m_file != NULL); }
		void close(void) { fclose(m_file); m_file = NULL; }
		
		inline MDP_SYSTEM_ID sysId(void) const { return m_sysId; }
		inline RomFormat romFormat(void) const { return m_romFormat; }
		
		inline int romSize(void) const { return m_romSize; }
		
		int initSRam(SRam *sram) const;
		int initEEPRom(EEPRom *eeprom) const;
		
		/**
		 * loadRom(): Load the ROM image into a buffer.
		 * @param buf Buffer.
		 * @param siz Buffer size.
		 * @return Positive value indicating amount of data read on success; 0 or negative on error.
		 */
		int loadRom(void *buf, size_t siz);
		
		// ROM filename.
		const utf8_str *filename(void) const
			{ return m_filename.c_str(); }
		const utf8_str *filenameBaseNoExt(void) const
			{ return m_filenameBaseNoExt.c_str(); }
		
		/********************
		 * ROM header data. *
		 ********************/
		
		/// ROM names. (Obtained from the ROM headers.)
		const utf8_str *romNameJP(void) const
			{ return m_romNameJP.c_str(); }
		const utf8_str *romNameUS(void) const
			{ return m_romNameUS.c_str(); }
		
		/// ROM checksum.
		uint16_t checksum(void) const
			{ return m_mdHeader.checksum; }
		
		/// Region code. (MD hex format)
		int regionCode(void) const
			{ return m_regionCode; }
		
		/** Multi-file ROM archive support. **/
		
		/**
		 * isMultiFile(): Determine if the loaded ROM archive has multiple files.
		 * @return True if the ROM archive has multiple files; false if it doesn't.
		 */
		bool isMultiFile(void) const
			{ return (m_z_entry_list && m_z_entry_list->next); }
		
		const mdp_z_entry_t *get_z_entry_list(void) const
			{ return m_z_entry_list; }
		
		/**
		 * select_z_entry(): Select a file from a multi-file ROM archive to load.
		 * @param sel File to load.
		 * @return 0 on success; non-zero on error.
		 */
		int select_z_entry(const mdp_z_entry_t *sel);
		
		/**
		 * isRomSelected(): Determine if a ROM has been selected.
		 * NOTE: This is only correct if the ROM file hasn't been closed.
		 * @return True if a ROM has been selected.
		 */
		bool isRomSelected(void) const { return (m_z_entry_sel != NULL); }
	
	private:
		MDP_SYSTEM_ID m_sysId;
		RomFormat m_romFormat;
		
		FILE *m_file;
		Decompressor *m_decomp;
		mdp_z_entry_t *m_z_entry_list;
		const mdp_z_entry_t *m_z_entry_sel;
		
		// System overrides specified in the constructor.
		MDP_SYSTEM_ID m_sysId_override;
		RomFormat m_romFormat_override;
		
		/**
		 * loadRomHeader(): Load the ROM header from the selected ROM file.
		 * @param sysOverride System override.
		 * @param fmtOverride Format override.
		 * @return 0 on success; non-zero on error.
		*/
		int loadRomHeader(MDP_SYSTEM_ID sysOverride, RomFormat fmtOverride);
		
		static RomFormat DetectFormat(const uint8_t *header, size_t header_size);
		static MDP_SYSTEM_ID DetectSystem(const uint8_t *header, size_t header_size, RomFormat fmt);
		
		void readHeaderMD(const uint8_t *header, size_t header_size);
		
		// Space elimination algorithm.
		static inline FUNC_PURE bool IsGraphChar(char chr)
		{
			return (isgraph(chr) || (chr & 0x80));
		}
		static std::string SpaceElim(const char *src, size_t len);
		
		/** Variables. **/
		
		std::string m_filename;			// ROM filename.
		std::string m_filenameBaseNoExt;	// ROM filename. (basename; no extension)
		unsigned int m_romSize;			// ROM size.
		int m_eprType;				// EEPRom type.
		
		/**
		 * MD_RomHeader: ROM header. (MD-style)
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
		
		// ROM names.
		std::string m_romNameJP;
		std::string m_romNameUS;
		
		// Region code. Same format as the newer hex format used in MD ROMs.
		int m_regionCode;
		int detectRegionCodeMD(const char countryCodes[16]);
};

}

#endif /* __LIBGENS_ROM_HPP__ */
