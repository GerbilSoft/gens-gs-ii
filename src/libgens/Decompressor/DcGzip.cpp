/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DcGzip.cpp: GZip decompressor class.                                    *
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

#include "DcGzip.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// C++ includes.
#include <string>
using std::string;

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

namespace LibGens {

/**
 * Create a new DcGzip object.
 * @param f File pointer.
 * @param filename Filename.
 */
DcGzip::DcGzip(FILE *f, const char *filename)
	: Decompressor(f, filename)
	, m_gzFile(nullptr)
{
	// Seek to the beginning of the file and flush the stream.
	// Otherwise, zlib won't properly detect it as gzipped.
	fseeko(f, 0, SEEK_SET);
	fflush(f);

	// Duplicate the file handle for gzdopen.
	int fd = dup(fileno(f));
	if (fd == -1) {
		// dup() failed.
		m_file = nullptr;
		m_filename.clear();
		return;
	}

	// Open the file with zlib.
	m_gzFile = gzdopen(fd, "r");
	if (!m_gzFile) {
		// gzdopen() failed.
		m_file = nullptr;
		m_filename.clear();
		close(fd);
		return;
	}
}

/**
 * Delete the GZip decompressor object.
 */
DcGzip::~DcGzip()
{
	// Close the GZip file.
	if (m_gzFile)
		gzclose_r(m_gzFile);
}

/**
 * Detect if the file can be handled by this decompressor.
 * This function should be reimplemented by derived classes.
 * NOTE: Do NOT call this function like a virtual function!
 * @param f File pointer.
 * @return True if the file can be handled by this decompressor.
 */
bool DcGzip::DetectFormat(FILE *f)
{
	// Seek to the beginning of the file.
	fseeko(f, 0, SEEK_SET);

	// Read the "magic number".
	uint8_t header[2];
	size_t ret = fread(&header, 1, sizeof(header), f);
	if (ret < sizeof(header)) {
		// Error reading the "magic number".
		return false;
	}

	// "Magic number" should be 1F 8B.
	return (header[0] == 0x1F && header[1] == 0x8B);
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int DcGzip::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Make sure the file is open.
	if (!m_file || !m_gzFile)
		return -2;

	// Filesize.
	size_t filesize = 0;

	// gzseek() does not support SEEK_END.
	// Read through the file until we hit an EOF.
	uint8_t buf[4096];
	gzrewind(m_gzFile);
	while (!gzeof(m_gzFile)) {
		filesize += gzread(m_gzFile, buf, sizeof(buf));
	}

	// Allocate an mdp_z_entry_t.
	// NOTE: C-style malloc() is used because MDP is a C API.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

	// Set the elements of the list entry.
	z_entry->filesize = filesize;
	z_entry->filename = (!m_filename.empty() ? strdup(m_filename.c_str()) : nullptr);
	z_entry->next = nullptr;

	// Return the list.
	*z_entry_out = z_entry;
	return 0; // TODO: return MDP_ERR_OK;
}

/**
 * Get a file from the archive.
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract. [TODO]
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf.
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return MDP error code. [TODO]
 */
int DcGzip::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Make sure the file is open.
	if (!m_file || !m_gzFile)
		return -2; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Seek to the beginning of the file.
	gzrewind(m_gzFile);

	// Read the file into the buffer.
	*ret_siz = gzread(m_gzFile, buf, siz);
	return 0; // TODO: return MDP_ERR_OK;
}

}
