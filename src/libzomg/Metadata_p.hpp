/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Metadata.cpp: Metadata handler. (PRIVATE CLASS)                         *
 *                                                                         *
 * Copyright (c) 2013-2015 by David Korth.                                 *
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

#ifndef __LIBZOMG_METADATA_P_HPP__
#define __LIBZOMG_METADATA_P_HPP__

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <ctime>

// C++ includes.
#include <string>
#include <sstream>

#include "Metadata.hpp"
namespace LibZomg {

class MetadataPrivate
{
	public:
		MetadataPrivate();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		MetadataPrivate(const MetadataPrivate &);
		MetadataPrivate &operator=(const MetadataPrivate &);

	public:
		/**
		 * Get the current time.
		 * This function is implemented in OS-specific files.
		 */
		void init_ctime(void);

		/**
		 * Initialize system-specific metadata.
		 * This function is implemented in OS-specific files.
		 */
		static void InitSystemMetadata(void);

		// Useful functions.
		static void WriteValue(std::ostringstream& oss, const std::string &key, const std::string &value);
		static void WriteValue(std::ostringstream& oss, const std::string &key, uint32_t value, int width = 0, bool hex = false);

	public:
		// System-specific metadata.
		// Should be initialized at program startup.
		// TODO: If the program doesn't initialize it, initialize on first Metadata use?
		struct SysInfo_t {
			std::string osVersion;
			std::string username;
			// TODO: Custom username override.
			//std::string username_custom;
			std::string cpu;	// TODO
		};
		static SysInfo_t sysInfo;

		// Program-specific metadata.
		// Should be initialized at program startup.
		struct CreatorInfo_t {
			std::string creator;
			std::string creatorVersion;
			std::string creatorVcsVersion;
		};
		static CreatorInfo_t creatorInfo;

		// Default metadata export flags.
		static const int MetadataFlagsDefault =
			Metadata::MF_CreationTime |
			Metadata::MF_Emulator |
			Metadata::MF_OSandCPU |
			Metadata::MF_RomInfo;

		// NOTE: MSVC uses 64-bit time_t by default.
		// FIXME: 64-bit time_t for 32-bit Linux?
		// FIXME: What about MinGW?

		// Per-file metadata.
		struct {
			time_t seconds;
			uint32_t nano;
		} ctime;
		std::string systemId;
		std::string romFilename;
		uint32_t romCrc32;
		std::string region;
		std::string description;
		std::string extensions;
};

}

#endif /* __LIBZOMG_METADATA_P_HPP__ */
