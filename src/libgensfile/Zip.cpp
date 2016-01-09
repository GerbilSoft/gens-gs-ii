/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Zip.cpp: Zip archive handler.                                           *
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

#include "Zip.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
// MiniZip Win32 UTF-8 compatibility library.
#include "libcompat/W32U/minizip_iowin32u.h"
#endif /* _WIN32 */

namespace LibGensFile {

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Zip::Zip(const char *filename)
	: Archive(filename)
	, m_unzFile(nullptr)
{
	if (!m_file)
		return;

	// Check for Zip magic first.
	// If it's not there, this isn't a Zip archive.
	static const uint8_t zip_magic[] = {'P', 'K', 0x03, 0x04};
	int ret = checkMagic(zip_magic, sizeof(zip_magic));
	if (ret != 0) {
		// Not a Zip archive.
		m_lastError = -ret;
		fclose(m_file);
		m_file = nullptr;
		return;
	}

	// Open the file using MiniZip.
	// This is done by filename, since MiniZip
	// doesn't support opening by fd.
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	m_unzFile = unzOpen2_64(filename, &ffunc);
#else
	m_unzFile = unzOpen(filename);
#endif

	if (!m_unzFile) {
		// Error opening the file.
		// TODO: Was it an I/O error, or is this
		// simply not a Zip file?
		fclose(m_file);
		m_file = nullptr;
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}
}

/**
 * Delete the Zip object.
 */
Zip::~Zip()
{
	// Close the Zip file.
	if (m_unzFile) {
		unzClose(m_unzFile);
	}
}

/**
 * Close the archive file.
 */
void Zip::close(void)
{
	if (m_unzFile) {
		unzClose(m_unzFile);
		m_unzFile = nullptr;
	}

	// Base class closes the FILE*.
	Archive::close();
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Zip::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file || !m_unzFile) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// List head and tail.
	mdp_z_entry_t *z_entry_head = nullptr;
	mdp_z_entry_t *z_entry_tail = nullptr;

	// MiniZip buffers.
	char rom_filename[4096];
	unz_file_info zinfo;

	// Find the first ROM file in the Zip archive.
	// TODO: Get UTF-8 filenames from the extra field.
	// TODO: Convert regular filename field from original encoding.
	int i = unzGoToFirstFile(m_unzFile);
	while (i == UNZ_OK) {
		// TODO: Support unzGetCurrentFileInfo64().
		unzGetCurrentFileInfo(m_unzFile, &zinfo,
			rom_filename, sizeof(rom_filename),
			nullptr, 0, nullptr, 0);
		rom_filename[sizeof(rom_filename)-1] = 0x00;

		if (zinfo.external_fa & 0x10) {
			// Directory. (0x10 == MS-DOS attribute for directory.)
			// Skip this file.
			i = unzGoToNextFile(m_unzFile);
			continue;
		}

		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

		// Store the ROM file information.
		z_entry_cur->filename = (rom_filename[0] != 0x00 ? strdup(rom_filename) : nullptr);
		z_entry_cur->filesize = zinfo.uncompressed_size;
		z_entry_cur->next = nullptr;

		if (!z_entry_head) {
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_tail = z_entry_cur;
		} else {
			// Append the mdp_z_entry to the end of the list.
			z_entry_tail->next = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}

		// Go to the next file.
		i = unzGoToNextFile(m_unzFile);
	}

	// If there are no files in the archive, return an error.
	if (!z_entry_head) {
		// TODO: Better error code?
		m_lastError = ENOENT;
		return -m_lastError; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	}

	// Return the list of files.
	*z_entry_out = z_entry_head;
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
int Zip::readFile(const mdp_z_entry_t *z_entry,
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
	} else if (!m_file || !m_unzFile) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Make sure the z_entry filename is valid.
	if (!z_entry->filename) {
		// Invalid filename.
		m_lastError = EINVAL;
		return -m_lastError; // TODO: Return an appropriate MDP error code.
	}

	// Locate the ROM in the Zip file.
	// TODO: unzLocateFile() might not work with UTF-8 filenames.
	if (unzLocateFile(m_unzFile, z_entry->filename, 1) != UNZ_OK) {
		// File not found.
		m_lastError = ENOENT;
		return -m_lastError; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}

	if (unzOpenCurrentFile(m_unzFile) != UNZ_OK) {
		// Error opening the file in the Zip archive.
		// TODO: Specific error code?
		m_lastError = ENOENT;
		return -m_lastError; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}

	int zResult = UNZ_OK;

	// NOTE: MiniZip doesn't support seeking within the compressed file.
	// If start_pos > 0, emulate it.
	if (start_pos > 0) {
		// Read data into a temporary buffer.
		// TODO: Verify that this works.
		// TODO: 64-bit MiniZip functions.
		void *tmp = malloc((size_t)start_pos);
		zResult = unzReadCurrentFile(m_unzFile, tmp, (unsigned int)start_pos);
		free(tmp);
		if (zResult == (int)start_pos) {
			// Correct amount of data read.
			zResult = UNZ_OK;
		}
	}

	if (zResult == UNZ_OK) {
		// Decompress the ROM data.
		// TODO: 64-bit MiniZip functions.
		zResult = unzReadCurrentFile(m_unzFile, buf, (unsigned int)read_len);
	}

	if (zResult <= 0) {
		// An error occurred...
		const char *zip_err;

		// TODO: Add MDP Z errors for these.
		switch (zResult) {
			case UNZ_ERRNO:
				zip_err = "Unknown...";
				break;
			case UNZ_EOF:
				zip_err = "Unexpected end of file.";
				break;
			case UNZ_PARAMERROR:
				zip_err = "Parameter error.";
				break;
			case UNZ_BADZIPFILE:
				zip_err = "Bad ZIP file.";
				break;
			case UNZ_INTERNALERROR:
				zip_err = "Internal error.";
				break;
			case UNZ_CRCERROR:
				zip_err = "CRC error.";
				break;
			default:
				zip_err = "Unknown error.";
				break;
		}

		// TODO: This originally used LOG_MSG, but since it's no longer
		// part of LibGens, we can't use that.
		// TODO: Error codes and/or message?
		fprintf(stderr, "Zip: Error extracting file '%s' from archive '%s': %s",
			z_entry->filename, m_filename.c_str(), zip_err);
		m_lastError = EIO;
		return -m_lastError; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}

	// File extracted successfully.
	*ret_siz = (file_offset_t)zResult;
	return 0; // TODO: return MDP_ERR_OK;
}

}
