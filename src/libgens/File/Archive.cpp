/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Archive.cpp: Archive file reader. (Base Class)                          *
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

/**
 * DUMMY archive handler. This is a base class intended to be
 * subclassed for actual archive handlers.
 *
 * Using this class directly will effectively result in a nop.
 */

#include "Archive.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif

namespace LibGens { namespace File {

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Archive::Archive(const char *filename)
	: m_filename(filename)
	, m_lastError(0)
{
	// Attempt to open the file.
	m_file = fopen(filename, "rb");
	if (!m_file) {
		// Error opening the file.
		m_lastError = errno;
	}

	// Subclasses should do further initialization.
}

/**
 * Delete the Archive object.
 */
Archive::~Archive()
{
	// Subclasses should have closed any other
	// references to the file here.
	if (m_file) {
		fclose(m_file);
	}
}

/**
 * Close the archive file.
 */
void Archive::close(void)
{
	// NOTE: Subclasses should reimplement close()
	// and close any other references to the file.
	if (m_file) {
		fclose(m_file);
		m_file = nullptr;
	}
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Archive::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// NOTE: Using int64_t instead of size_t or off_t.
	// sizeof(size_t) == sizeof(void*)
	// sizeof(off_t) == 4 on MSVC regardless of architecture.
	int64_t filesize;
	fseeko(m_file, 0, SEEK_END);
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
	z_entry->filesize = (size_t)filesize;
	z_entry->filename = (!m_filename.empty() ? strdup(m_filename.c_str()) : nullptr);
	z_entry->next = nullptr;

	// Return the list.
	*z_entry_out = z_entry;
	return 0;
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
int Archive::readFile(const mdp_z_entry_t *z_entry,
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
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Seek to the specified starting position.
	fseeko(m_file, start_pos, SEEK_SET);

	// Read the file into the buffer.
	// FIXME: 64-bit parameters for fread?
	*ret_siz = fread(buf, 1, read_len, m_file);
	if (*ret_siz != siz) {
		// Short read. Something went wrong.
		m_lastError = errno;
		return -m_lastError;
	}
	return 0; // TODO: return MDP_ERR_OK;
}


/**
 * Free an allocated mdp_z_entry_t list.
 * @param z_entry Pointer to the first entry in the list.
 */
void Archive::z_entry_t_free(mdp_z_entry_t *z_entry)
{
	// Delete the mdp_z_entry_t list.
	for (mdp_z_entry_t *next; z_entry != nullptr; z_entry = next) {
		// Save the next pointer.
		next = z_entry->next;

		// Free the filename and the mdp_z_entry_t.
		free(z_entry->filename);
		free(z_entry);
	}
}

} }
