/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Gzip.cpp: Gzip archive handler.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2016 by David Korth.                                 *
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

#include "Gzip.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>

// Byteswapping macros.
#include "libcompat/byteswap.h"

// gzclose_r() and gzclose_w() were introduced in zlib-1.2.4.
#if (ZLIB_VER_MAJOR > 1) || \
    (ZLIB_VER_MAJOR == 1 && ZLIB_VER_MINOR > 2) || \
    (ZLIB_VER_MAJOR == 1 && ZLIB_VER_MINOR == 2 && ZLIB_VER_REVISION >= 4)
// zlib-1.2.4 or later
#else
#define gzclose_r(file) gzclose(file)
#define gzclose_w(file) gzclose(file)
#endif

#ifdef _WIN32
// Win32 requires io.h for dup() and close().
#include <io.h>
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif /* _WIN32 */

namespace LibGens { namespace File {

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Gzip::Gzip(const char *filename)
	: Archive(filename)
	, m_gzFile(nullptr)
{
	/**
	 * NOTE: Previously, DcGzip used dup() to duplicate the file handle
	 * instead of reopening it. We're switching to a separate gzopen()
	 * on non-Windows platforms for two reasons:
	 * - Some platforms (e.g. Mac OS X) have problems where zlib
	 *   refuses to recognize the dup()'d file as gzipped, even
	 *   if we flush the stream and rewind it.
	 * - Some embedded platforms (e.g. RVL) don't have dup().
	 *
	 * For Windows, we'll still use dup() so we don't have to convert
	 * the filename from UTF-8 to UTF-16 or ANSI.
	 */
#ifdef _WIN32
	rewind(m_file);
	fflush(m_file);

	// Duplicate the file handle for gzdopen.
	int fd = dup(fileno(m_file));
	if (fd == -1) {
		// dup() failed.
		fclose(m_file);
		m_lastError = errno;
		m_file = nullptr;
		return;
	}

	// Open the file with zlib.
	m_gzFile = gzdopen(fd, "r");

	// NOTE: In order to actually make the dup() go through,
	// we have to force zlib to read the file. Otherwise,
	// when getFileInfo() uses fseeko()/ftello() to get the
	// file sizes, zlib will get confused and not recognize
	// that the file is gzipped.
	// TODO: Maybe this is what affected the other platforms...
	gzdirect(m_gzFile);

#else /* !_WIN32 */
	// Open the file with zlib directly.
	m_gzFile = gzopen(filename, "r");
#endif

	if (!m_gzFile) {
		// gzdopen() / gzopen() failed.
		m_file = nullptr;
		m_filename.clear();
#ifdef _WIN32
		::close(fd);
#endif /* _WIN32 */
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}
}

/**
 * Delete the Gzip object.
 */
Gzip::~Gzip()
{
	// Close the Gzip file.
	if (m_gzFile) {
		gzclose_r(m_gzFile);
	}
}

/**
 * Close the archive file.
 */
void Gzip::close(void)
{
	if (m_gzFile) {
		gzclose_r(m_gzFile);
		m_gzFile = nullptr;
	}

	// Base class closes the FILE*.
	Archive::close();
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Gzip::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file || !m_gzFile) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Gzipped files contain a 32-bit uncompressed size value at the end of the file.
	// Use that to improve performance.
	// References:
	// - http://insanecoding.blogspot.com/2007/03/file-descriptors-and-why-we-cant-use.html
	// - http://www.zlib.org/rfc-gzip.html
	uint32_t gzsize;
	fseeko(m_file, -4, SEEK_END);
	int ret = fread(&gzsize, 1, sizeof(gzsize), m_file);
	if (ret != sizeof(gzsize)) {
		// Error reading the uncompressed filesize.
		// TODO: Check for EOF?
		m_lastError = errno;
		return -m_lastError;
	}

	// Byteswap the Gzip size.
	gzsize = le32_to_cpu(gzsize);

	// Get the actual filesize.
	// Note that zlib works with uncompressed files as well,
	// so if this file isn't compressed, we should use the
	// regular filesize.
	int64_t filesize;
	filesize = ftello(m_file);
	if (filesize < 0) {
		m_lastError = errno;
		return -m_lastError;
	}

	// Allocate an mdp_z_entry_t.
	// NOTE: C-style malloc() is used because MDP is a C API.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
	if (!z_entry) {
		m_lastError = ENOMEM;
		return -m_lastError;
	}

	// Set the elements of the list entry.
	// FIXME: z_entry->filesize should be changed to int64_t.
	// For filesize, check if zlib is operating on a compressed file.
	// If it is, use gzsize; otherwise, use filesize.
	z_entry->filesize = (gzdirect(m_gzFile) ? (size_t)filesize : (size_t)gzsize);
	z_entry->filename = (!m_filename.empty() ? strdup(m_filename.c_str()) : nullptr);
	z_entry->next = nullptr;

	// Return the list.
	*z_entry_out = z_entry;
	return 0; // TODO: return MDP_ERR_OK;
}

/**
 * Read all or part of a file from the archive.
 * NOTE: This function is NOT optimized for random seeking.
 * The skip functionality is intended for skipping over headers,
 * e.g. for SMD-format ROMs.
 *
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
 * @param start_pos	[in]  Starting position within the file.
 * @param read_len	[in]  Number of bytes to read.
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf. (Must be >= read_len.)
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return 0 on success; negative POSIX error code on error.
 */
int Gzip::readFile(const mdp_z_entry_t *z_entry,
		   file_offset_t start_pos, file_offset_t read_len,
		   void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
	if (!z_entry || !buf ||
	    start_pos < 0 || start_pos >= z_entry->filesize ||
	    read_len < 0 || z_entry->filesize - read_len < start_pos ||
	    siz <= 0 || siz < read_len)
	{
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file || !m_gzFile) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Seek to the beginning of the file.
	gzrewind(m_gzFile);

	if (start_pos > 0) {
		// Starting position is set.
		// Seek to that position.
		// FIXME: Check Z_LARGE64, Z_SOLO, Z_WANT64, etc.
		// zconf.h, line 486
		// zlib.h, line 1700
		gzseek(m_gzFile, (z_off_t)start_pos, SEEK_SET);
	}

	// Read the file into the buffer.
	*ret_siz = gzread(m_gzFile, buf, (z_off_t)read_len);
	if (*ret_siz != read_len) {
		// Short read. Something went wrong.
		// TODO: gzerror() returns a string...
		// TODO: #include <errno.h> or <cerrno> in places?
		m_lastError = EIO;
		return -m_lastError;
	}
	return 0; // TODO: return MDP_ERR_OK;
}

} }
