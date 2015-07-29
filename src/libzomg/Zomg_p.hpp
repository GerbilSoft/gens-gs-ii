/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg_p.h: Savestate handler. (Private class)                            *
 *                                                                         *
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

#ifndef __LIBZOMG_ZOMG_P_HPP__
#define __LIBZOMG_ZOMG_P_HPP__

// utf8_str
// TODO: Don't depend on LibGens.
#include "../libgens/macros/common.h"

// MiniZip
#include "minizip/zip.h"
#include "minizip/unzip.h"

namespace LibZomg {

class Zomg;
class ZomgPrivate
{
	public:
		ZomgPrivate(Zomg *q);
		~ZomgPrivate();

	protected:
		friend class Zomg;
		Zomg *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		ZomgPrivate(const ZomgPrivate &);
		ZomgPrivate &operator=(const ZomgPrivate &);

	public:
		// TODO: Combine into a union? Need to check all users...
		unzFile unz;
		zipFile zip;

		// Current time in Zip format.
		// NOTE: Only used when saving ZOMG files.
		zip_fileinfo zipfi;

		int initZomgLoad(const utf8_str *filename);
		int initZomgSave(const utf8_str *filename);

		/**
		 * File type.
		 * This maps directly to Zip internal file attributes.
		 */
		enum ZomgZipFileType_t {
			ZOMG_FILE_BINARY = 0,
			ZOMG_FILE_TEXT = 1,
		};

		int loadFromZomg(const utf8_str *filename, void *buf, int len);
		int saveToZomg(const utf8_str *filename, const void *buf, int len,
			       ZomgZipFileType_t fileType = ZOMG_FILE_BINARY);
};

}

#endif /* __LIBGENS_MD_VDP_P_HPP__ */
