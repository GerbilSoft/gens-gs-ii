/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Metadata.cpp: Metadata handler for savestates and screenshots.          *
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

#ifndef __LIBZOMG_METADATA_HPP__
#define __LIBZOMG_METADATA_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

namespace LibZomg {

class MetadataPrivate;
class Metadata
{
	public:
		Metadata();
		~Metadata();

	private:
		friend class MetadataPrivate;
		// NOTE: Non-const to allow clear() to work more efficiently.
		MetadataPrivate *d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		Metadata(const Metadata &);
		Metadata &operator=(const Metadata &);

	public:
		/**
		 * Initialize the system and program metadata.
		 * This function should only be run once at program startup.
		 * System information will be obtained by the Metadata class.
		 *
		 * @param creator		[in, opt] Emulator name.
		 * @param creatorVersion	[in, opt] Emulator version.
		 * @param creatorVcsVersion	[in, opt] Emulator's version control version, e.g. git tag.
		 */
		static void InitProgramMetadata(const char *creator,
						const char *creatorVersion,
						const char *creatorVcsVersion);

		/**
		 * Clear the loaded metadata.
		 */
		void clear(void);

		/**
		 * Export the metadata as ZOMG.ini.
		 * @return String representation of ZOMG.ini.
		 */
		std::string toZomgIni(void) const;

		/** Property get/set functions. **/
		// TODO: Use macros?

		// TODO: Use MDP system ID enumeration instead of a string.
		std::string systemId(void) const;
		void setSystemId(const std::string &systemId);

		std::string romFilename(void) const;
		void setRomFilename(const std::string &romFilename);

		uint32_t romCrc32(void) const;
		void setRomCrc32(uint32_t romCrc32);

		// TODO: Use numeric region codes?
		std::string region(void) const;
		void setRegion(const std::string &region);

		std::string description(void) const;
		void setDescription(const std::string &description);

		std::string extensions(void) const;
		void setExtensions(const std::string &extensions);
};

}

#endif /* __LIBZOMG_METADATA_HPP__ */
