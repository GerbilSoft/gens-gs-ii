/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * ZomgIni.hpp: ZOMG.ini handler.                                          *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __LIBZOMG_ZOMGINI_HPP__
#define __LIBZOMG_ZOMGINI_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

namespace LibZomg
{

class ZomgIniPrivate;

class ZomgIni
{
	public:
		ZomgIni();
		~ZomgIni();

	private:
		friend class CdDrivePrivate;
		// NOTE: Non-const to allow clear() to work more efficiently.
		ZomgIniPrivate *d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		ZomgIni(const ZomgIni &);
		ZomgIni &operator=(const ZomgIni &);

	public:
		/**
		 * Clear the loaded ZOMG.ini data.
		 */
		void clear(void);

		/**
		 * Save the ZOMG.ini file.
		 * @return String representation of ZOMG.ini.
		 */
		std::string save(void) const;

		/** Property get/set functions. **/

		// TODO: Use bitfield with system IDs instead of a string.
		std::string systemId(void);
		void setSystemId(std::string systemId);

		std::string creator(void);
		void setCreator(std::string creator);

		std::string creatorVersion(void);
		void setCreatorVersion(std::string creatorVersion);

		std::string creatorVcsVersion(void);
		void setCreatorVcsVersion(std::string creatorVcsVersion);

		std::string author(void);
		void setAuthor(std::string author);

		std::string romFilename(void);
		void setRomFilename(std::string romFilename);

		uint32_t romCrc32(void);
		void setRomCrc32(uint32_t romCrc32);

		// TODO: Use numeric region codes?
		std::string region(void);
		void setRegion(std::string region);

		std::string description(void);
		void setDescription(std::string description);

		std::string extensions(void);
		void setExtensions(std::string extensions);
};

}

#endif /* __LIBZOMG_ZOMGINI_HPP__ */
