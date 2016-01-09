/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Sz.cpp: 7-Zip archive handler.                                          *
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
 * Based on 7zMain.c from the LZMA SDK 9.12.
 * http://www.7-zip.org/sdk.html
 */

#include "Sz.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>
// C++ includes.
#include <string>
using std::string;
using std::u16string;

// 7-Zip includes.
#include "lzma/7zAlloc.h"
#include "lzma/7zVersion.h"
#include "lzma/7zCrc.h"

// Character set conversion.
#include "libgenstext/Encoding.hpp"

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif

namespace LibGensFile {

// Set to true if the 7z CRC table has been initialized.
bool Sz::ms_CrcInit = false;

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Sz::Sz(const char *filename)
	: Archive(filename)
	, m_blockIndex(~0)
	, m_outBuffer(nullptr)
	, m_outBufferSize(0)
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

	// Check for 7-Zip magic first.
	// If it's not there, this isn't a 7-Zip archive.
	static const uint8_t _7z_magic[] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};
	int ret = checkMagic(_7z_magic, sizeof(_7z_magic));
	if (ret != 0) {
		// Not a 7-Zip archive.
		m_lastError = -ret;
		fclose(m_file);
		m_file = nullptr;
		return;
	}

	SRes res;
#ifdef _WIN32
	// Convert the filename from UTF-8 to UTF-16.
	u16string filenameW = LibGensText::Utf8_to_Utf16(filename, strlen(filename));
	if (filenameW.empty()) {
		// Error converting the filename to UTF-16.
		fclose(m_file);
		m_file = nullptr;
		m_lastError = EINVAL;
		return; // TODO: Figure out an MDP error code for this.
	}

	if (W32U_IsUnicode()) {
		res = InFile_OpenW(&m_archiveStream.file, (const wchar_t*)filenameW.c_str());
	} else {
		// System doesn't support Unicode.
		// Convert the filename from UTF-16 to ANSI.
		// FIXME: LibGensText doesn't support CP_ACP.
		// TODO: Use W32U_alloca?
		char *filenameA = W32U_UTF16_to_mbs((const wchar_t*)filenameW.c_str(), CP_ACP);
		if (!filenameA) {
			// Error converting the filename to ANSI.
			fclose(m_file);
			m_file = nullptr;
			m_lastError = EINVAL;
			return; // TODO: Figure out an MDP error code for this.
		}
		res = InFile_Open(&m_archiveStream.file, filenameA);
		free(filenameA);
	}
#else
	// Unix system. Use UTF-8 filenames directly.
	res = InFile_Open(&m_archiveStream.file, filename);
#endif

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
	if (!ms_CrcInit) {
		CrcGenerateTable();
		ms_CrcInit = true;
	}

	SzArEx_Init(&m_db);
	res = SzArEx_Open(&m_db, &m_lookStream.s, &m_allocImp, &m_allocTempImp);
	if (res != SZ_OK) {
		// Error opening the file.
		SzArEx_Free(&m_db, &m_allocImp);
		File_Close(&m_archiveStream.file);

		fclose(m_file);
		m_file = nullptr;
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// 7-Zip file is opened.
}

/**
 * Delete the Sz object.
 */
Sz::~Sz()
{
	// Free the output buffer, if it's allocated.
	if (m_outBuffer) {
		IAlloc_Free(&m_allocImp, m_outBuffer);
	}

	// Close the 7-Zip file.
	if (m_file) {
		SzArEx_Free(&m_db, &m_allocImp);
		File_Close(&m_archiveStream.file);
	}
}

/**
 * Close the archive file.
 */
void Sz::close(void)
{
	// Free the output buffer, if it's allocated.
	if (m_outBuffer) {
		IAlloc_Free(&m_allocImp, m_outBuffer);
		m_outBuffer = nullptr;
		m_outBufferSize = 0;
	}

	// Close the 7-Zip file.
	if (m_file) {
		SzArEx_Free(&m_db, &m_allocImp);
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
int Sz::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// List head and tail.
	mdp_z_entry_t *z_entry_head = nullptr;
	mdp_z_entry_t *z_entry_tail = nullptr;

	// Filename buffer. (UTF-16)
	char16_t *filenameW = nullptr;
	size_t filenameW_len = 0;

	// Read the filenames.
	for (unsigned int i = 0; i < m_db.NumFiles; i++) {
		// TODO: Verify this.
		if (m_db.IsDirs[i])
			continue;

		// Get the filename.
		size_t len = SzArEx_GetFileNameUtf16(&m_db, i, nullptr);
		if (len > filenameW_len) {
			// Increase the filenameW buffer.
			// TODO: Check for malloc() errors?
			free(filenameW);
			filenameW_len = len;
			filenameW = (char16_t*)malloc(filenameW_len * sizeof(*filenameW));
		}
		SzArEx_GetFileNameUtf16(&m_db, i, (uint16_t*)filenameW);

		// Convert the filename to UTF-8.
		string z_entry_filename = LibGensText::Utf16_to_Utf8(filenameW, filenameW_len);
		if (z_entry_filename.empty()) {
			// Error converting the filename to UTF-8.
			// We'll just mask each UTF-16 character by 0x7F for ASCII-compatible filenames.
			// TODO: Include our own UTF-16 to UTF-8 conversion function?
			z_entry_filename.resize(filenameW_len);
			for (unsigned int chr = 0; chr < len; chr++) {
				z_entry_filename[chr] = (filenameW[chr] & 0x7F);
			}
		}

		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

		// Store the ROM file information.
		// TODO: f->Size is 64-bit...
		z_entry_cur->filename = strdup(z_entry_filename.c_str());
		z_entry_cur->filesize = (size_t)SzArEx_GetFileSize(&m_db, i);
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
	}

	// If there are no files in the archive, return an error.
	if (!z_entry_head)
		return -1; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;

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
int Sz::readFile(const mdp_z_entry_t *z_entry,
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

	// Make sure the z_entry filename is valid.
	if (!z_entry->filename) {
		// Invalid filename.
		m_lastError = EINVAL;
		return -m_lastError; // TODO: Return an appropriate MDP error code.
	}

	// Convert the z_entry filename to UTF-16.
	u16string z_entry_filenameW = LibGensText::Utf8_to_Utf16(string(z_entry->filename));
	if (z_entry_filenameW.empty()) {
		// Error converting the filename to Unicode.
		m_lastError = EINVAL;
		return -m_lastError; // TODO: Return an appropriate MDP error code.
	}

	char16_t *filenameW = nullptr;
	size_t filenameW_len = 0;

	// Locate the file in the 7-Zip archive.
	unsigned int i = 0;
	SRes res;
	for (i = 0; i < m_db.NumFiles; i++) {
		// TODO: Verify this.
		if (m_db.IsDirs[i])
			continue;

		// Get the filename.
		size_t len = SzArEx_GetFileNameUtf16(&m_db, i, nullptr);
		if (len > filenameW_len) {
			// Increase the filenameW buffer.
			// TODO: Check for malloc() errors?
			free(filenameW);
			filenameW_len = len;
			filenameW = (char16_t*)malloc(filenameW_len * sizeof(*filenameW));
		}
		SzArEx_GetFileNameUtf16(&m_db, i, (uint16_t*)filenameW);

		// Compare the filename against the z_entry filename.
		if (LibGensText::Utf16_ncmp(z_entry_filenameW.c_str(), filenameW, filenameW_len) != 0) {
			// Not the correct file.
			continue;
		}

		// Found the file.

		// Extract the file into the buffer.
		size_t offset;
		size_t outSizeProcessed;
		res = SzArEx_Extract(&m_db, &m_lookStream.s, i,
				     &m_blockIndex, &m_outBuffer, &m_outBufferSize,
				     &offset, &outSizeProcessed,
				     &m_allocImp, &m_allocTempImp);

		if (res != SZ_OK) {
			// Error extracting the file.
			// TODO: Return an appropriate MDP error code.
			// For now, just break out of the loop.
			i = m_db.NumFiles;
			break;
		}

		// Copy the 7z buffer to the output buffer.
		// TODO: Handle start_pos.
		*ret_siz = (read_len < (int64_t)outSizeProcessed
				? read_len
				: (int64_t)outSizeProcessed);
		memcpy(buf, (m_outBuffer + offset), (size_t)(*ret_siz));

		// ROM processed.
		break;
	}
	
	// Free the temporary UTF-16 filename buffer.
	free(filenameW);

	if (i >= m_db.NumFiles) {
		// File not found.
		// TODO: Could also be an error in extracting the file.
		m_lastError = ENOENT;
		return -m_lastError; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}

	// File extracted successfully.
	return 0; // TODO: return MDP_ERR_OK;
}

}
