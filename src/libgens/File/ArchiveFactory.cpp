/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * ArchiveFactory.cpp: Archive factory class.                              *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#include "ArchiveFactory.hpp"
#include "Archive.hpp"
#include "Gzip.hpp"
#include "Zip.hpp"

namespace LibGens { namespace File {

/**
 * Open the specified file using an Archive subclass.
 * @param filename Archive filename.
 * @return Opened archive, or nullptr on error. (TODO: Return an error code?)
 */
Archive *ArchiveFactory::openArchive(const char *filename)
{
	/**
	 * TODO:
	 * - Open the file here and pass a FILE* instead of
	 *   the filename so we can check for errors here?
	 * - Handle libarchive, MiniZip, and zlib.
	 *   Maybe skip MiniZip if libarchive is supported...
	 */
	Archive *archive;

	// Try Zip via MiniZip.
	archive = new Zip(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;

	// Try Gzip (zlib).
	// Note that zlib will handle uncompressed files
	// as well as compressed files, so this attempt
	// shouldn't fail.
	archive = new Gzip(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;

	// As a last resort, use the base Archive class.
	// No decompression will be performed, so only
	// uncompressed ROMs will work properly.
	archive = new Archive(filename);
	if (archive->isOpen())
		return archive;

	// Error opening the archive.
	// TODO: Error code?
	delete archive;
	return nullptr;
}

} }
