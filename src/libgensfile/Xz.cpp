/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Xz.hpp: Xz archive handler.                                             *
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

#include "Xz.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>

// Byteswapping macros.
#include "libcompat/byteswap.h"

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif /* _WIN32 */

// 7-Zip includes.
#include "lzma/7zCrc.h"
#include "lzma/7zAlloc.h"
#include "lzma/Xz.h"
#include "lzma/XzCrc64.h"

namespace LibGensFile {

// Set to true if the 7z CRC table has been initialized.
bool Xz::ms_CrcInit = false;

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Xz::Xz(const char *filename)
	: Archive(filename)
{
	// NOTE: This initialization MUST be done before checking
	// for a filename error!

	// Initialize the memory allocators.
	m_allocImp.Alloc = SzAlloc;
	m_allocImp.Free = SzFree;

	// Initialize the temporary memory allocators.
	m_allocTempImp.Alloc = SzAllocTemp;
	m_allocTempImp.Free = SzFreeTemp;

	/** End of basic 7-Zip initialization. **/

	if (!m_file) {
		return;
	} else if (!filename) {
		fclose(m_file);
		m_file = nullptr;
		m_lastError = EINVAL;
		return;
	}

	// Check for Xz magic first.
	// If it's not there, this isn't an Xz archive.
	static const uint8_t xz_magic[] = {0xFD, '7', 'z', 'X', 'Z', 0x00};
	int ret = checkMagic(xz_magic, sizeof(xz_magic));
	if (ret != 0) {
		// Not an Xz archive.
		m_lastError = -ret;
		fclose(m_file);
		m_file = nullptr;
		return;
	}

	// TODO: This only works on Unix.
	// Combine the 7z one into a utility file for LZMA.
	// Unix system. Use UTF-8 filenames directly.
	SRes res = InFile_Open(&m_archiveStream.file, filename);
	if (res != 0) {
		// Error opening the file.
		fclose(m_file);
		m_file = nullptr,
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// Initialize VTables.
	FileInStream_CreateVTable(&m_archiveStream);
	LookToRead_CreateVTable(&m_lookStream, false);

	m_lookStream.realStream = &m_archiveStream.s;
	LookToRead_Init(&m_lookStream);

	// Generate the CRC table.
	// TODO: Shared with Sz?
	if (!ms_CrcInit) {
		CrcGenerateTable();
		Crc64GenerateTable();
		ms_CrcInit = true;
	}

	// TODO: Only read the first stream?
	Xzs_Construct(&m_xzs);

	int64_t startPosition;
	res = Xzs_ReadBackward(&m_xzs, &m_lookStream.s, &startPosition, nullptr, &m_allocImp);
	if (res != SZ_OK || startPosition != 0) {
		// Error seeking to the beginning of the file.
		Xzs_Free(&m_xzs, &m_allocImp);
		File_Close(&m_archiveStream.file);
		fclose(m_file);
		m_file = nullptr,
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// Xz file is opened.
}

/**
 * Delete the Xz object.
 */
Xz::~Xz()
{
	// Close the Xz file.
	if (m_file) {
		Xzs_Free(&m_xzs, &m_allocImp);
		File_Close(&m_archiveStream.file);
	}
}

/**
 * Close the archive file.
 */
void Xz::close(void)
{
	// Close the Xz file.
	if (m_file) {
		Xzs_Free(&m_xzs, &m_allocImp);
		File_Close(&m_archiveStream.file);
	}

	// Base class closes the FILE*.
	Archive::close();
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Xz::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Create a fake mdp_z_entry_t containing the uncompressed
	// filesize and the archive's filename.

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
	z_entry->filesize = (size_t)Xzs_GetUnpackSize(&m_xzs);
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
 * @param ret_siz	[out] Pointer to file_offset_t to store the number of bytes read.
 * @return 0 on success; negative POSIX error code on error.
 */
int Xz::readFile(const mdp_z_entry_t *z_entry,
		   file_offset_t start_pos, file_offset_t read_len,
		   void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
#if 0
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
#endif
}

}
