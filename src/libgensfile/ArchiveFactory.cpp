/***************************************************************************
 * libgensfile: Gens file handling library.                                *
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

#include <config.libgensfile.h>

#include "ArchiveFactory.hpp"
#include "Archive.hpp"

#ifdef HAVE_ZLIB
#include "Gzip.hpp"
#ifdef HAVE_MINIZIP
#include "Zip.hpp"
#endif /* HAVE_MINIZIP */
#endif /* HAVE_ZLIB */

#ifdef HAVE_LZMA
#include "Sz.hpp"
#include "Xz.hpp"
#endif /* HAVE_LZMA */

// TODO: HAVE_UNRAR?
#include "Rar.hpp"

namespace LibGensFile {

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
	 * - Differentiate between "file not supported" and
	 *   "file supported, but broken". The latter case
	 *   should return immediately without trying any
	 *   other readers.
	 */
	Archive *archive;

	// TODO: Reorder based on overhead?

	// Try RAR via UnRAR.dll.
	archive = new Rar(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;

#ifdef HAVE_LZMA
	// Try 7-Zip via the LZMA SDK.
	archive = new Sz(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;

	// Try Xz via the LZMA SDK.
	archive = new Xz(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;
#endif /* HAVE_LZMA */

#ifdef HAVE_ZLIB
#ifdef HAVE_MINIZIP
	// Try Zip via MiniZip.
	archive = new Zip(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;
#endif /* HAVE_MINIZIP */

	// Try Gzip (zlib).
	// Note that zlib will handle uncompressed files
	// as well as compressed files, so this attempt
	// shouldn't fail.
	archive = new Gzip(filename);
	if (archive->isOpen())
		return archive;

	delete archive;
	archive = nullptr;
#endif /* HAVE_ZLIB */

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

}
