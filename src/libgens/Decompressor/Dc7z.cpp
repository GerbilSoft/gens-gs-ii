/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Dc7z.cpp: 7-Zip decompressor class.                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include <libgens/config.libgens.h>

/**
 * Based on 7zMain.c from the LZMA SDK 9.12.
 * http://www.7-zip.org/sdk.html
 */

#include "Dc7z.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// Byteswapping macros and functions.
#include "Util/byteswap.h"

// C++ includes.
#include <string>
using std::string;
using std::u16string;

// 7-Zip includes.
#include "lzma/7z/7zAlloc.h"
#include "lzma/7z/7zVersion.h"
#include "lzma/lzmabase/7zCrc.h"

// Character set conversion.
#include "libgenstext/Encoding.hpp"
#if defined(_WIN32)
#include "Win32/W32U_mini.h"
#endif

namespace LibGens
{

/** Static class variable initialization. **/

/**
 * Set to true if the 7z CRC table has been initialized.
 */
bool Dc7z::ms_CrcInit = false;

/**
 * Create a new Dc7z object.
 * @param f File pointer.
 * @param filename Filename.
 */
Dc7z::Dc7z(FILE *f, const utf8_str *filename)
	: Decompressor(f, filename)
{
	SRes res;

	/**
	 * NOTE: This initialization MUST be done before checking
	 * for a filename error!
	 */

	// Initialize the miscellaneous 7-Zip variables.
	m_blockIndex = ~0;
	m_outBuffer = nullptr;
	m_outBufferSize = 0;

	// Initialize the memory allocators.
	m_allocImp.Alloc = SzAlloc;
	m_allocImp.Free = SzFree;

	// Initialize the temporary memory allocators.
	m_allocTempImp.Alloc = SzAllocTemp;
	m_allocTempImp.Free = SzFreeTemp;

	/** End of basic Dc7z initialization. **/

	// LZMA SDK doesn't support opening files by fd.
	if (!f || !filename) {
		// No filename specified.
		m_file = nullptr;
		m_filename.clear();
		return;
	}

#ifdef _WIN32
	// Convert the filename from UTF-8 to UTF-16.
	// TODO: Use LibGensText::Utf8_to_Utf16?
	wchar_t *filenameW = W32U_mbs_to_UTF16(filename, CP_UTF8);
	if (!filenameW) {
		// Error converting the filename to UTF-16.
		m_file = nullptr;
		m_filename.clear();
		return; // TODO: Figure out an MDP error code for this.
	}

	if (W32U_IsUnicode) {
		res = InFile_OpenW(&m_archiveStream.file, filenameW);
	} else {
		// System doesn't support Unicode.
		// Convert the filename from UTF-16 to ANSI.
		// TODO: Use LibGensText::Utf16_to_[something]?
		char *filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		if (!filenameA) {
			// Error converting the filename to ANSI.
			m_file = nullptr;
			m_filename.clear();
			return; // TODO: Figure out an MDP error code for this.
		}
		res = InFile_Open(&m_archiveStream.file, filenameA);
		free(filenameA);
	}
	free(filenameW);
#else
	// Unix system. Use UTF-8 filenames directly.
	res = InFile_Open(&m_archiveStream.file, filename);
#endif

	if (res != 0) {
		// Error opening the file.
		m_file = nullptr,
		m_filename.clear();
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

		m_file = nullptr;
		m_filename.clear();
		return;
	}

	// 7-Zip file is opened.
}

/**
 * Delete the 7-Zip Decompressor object.
 */
Dc7z::~Dc7z()
{
	// Free the output buffer, if it's allocated.
	if (m_outBuffer) {
		IAlloc_Free(&m_allocImp, m_outBuffer);
		m_outBuffer = nullptr;
	}

	// Close the 7-Zip file.
	if (m_file) {
		SzArEx_Free(&m_db, &m_allocImp);
		File_Close(&m_archiveStream.file);
	}
}


/**
 * Detect if the file can be handled by this decompressor.
 * This function should be reimplemented by derived classes.
 * NOTE: Do NOT call this function like a virtual function!
 * @param f File pointer.
 * @return True if the file can be handled by this decompressor.
 */
bool Dc7z::DetectFormat(FILE *f)
{
	static const uint8_t _7z_magic[] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};

	// Seek to the beginning of the file.
	// TODO: fseeko()/fseeko64() support on Linux.
	fseek(f, 0, SEEK_SET);

	// Read the "magic number".
	uint8_t header[sizeof(_7z_magic)];
	size_t ret = fread(&header, 1, sizeof(header), f);
	if (ret < sizeof(header)) {
		// Error reading the "magic number".
		return false;
	}

	// Check the "magic number" and return true if it matches.
	return (!memcmp(header, _7z_magic, sizeof(header)));
}


/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int Dc7z::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Make sure the file is open.
	if (!m_file)
		return -2;

	// List head and tail.
	mdp_z_entry_t *z_entry_head = nullptr;
	mdp_z_entry_t *z_entry_tail = nullptr;

	// Filename buffer. (UTF-16)
	char16_t *filenameW = nullptr;
	size_t filenameW_len = 0;

	// Read the filenames.
	for (unsigned int i = 0; i < m_db.db.NumFiles; i++) {
		const CSzFileItem *f = (m_db.db.Files + i);
		if (f->IsDir)
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
		z_entry_cur->filesize = (size_t)f->Size;
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
 * Get a file from the archive.
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract. [TODO]
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf.
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return MDP error code. [TODO]
 */
int Dc7z::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	SRes res;

	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Make sure the file is open.
	if (!m_file)
		return -2; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Make sure the z_entry filename is valid.
	if (!z_entry->filename) {
		// Invalid filename.
		return -1; // TODO: Return an appropriate MDP error code.
	}

	// Convert the z_entry filename to UTF-16.
	u16string z_entry_filenameW = LibGensText::Utf8_to_Utf16(string(z_entry->filename));
	if (z_entry_filenameW.empty()) {
		// Error converting the filename to Unicode.
		return -4; // TODO: Return an appropriate MDP error code.
	}

	// Filename buffer. (UTF-16)
	char16_t *filenameW = nullptr;
	size_t filenameW_len = 0;

	// Locate the file in the 7-Zip archive.
	unsigned int i = 0;
	for (i = 0; i < m_db.db.NumFiles; i++) {
		const CSzFileItem *f = (m_db.db.Files + i);
		if (f->IsDir)
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
			i = m_db.db.NumFiles;
			break;
		}

		// Copy the 7z buffer to the output buffer.
		*ret_siz = (siz < outSizeProcessed ? siz : outSizeProcessed);
		memcpy(buf, (m_outBuffer + offset), *ret_siz);

		// ROM processed.
		break;
	}

	// Free the temporary UTF-16 filename buffer.
	free(filenameW);

	if (i >= m_db.db.NumFiles) {
		// File not found.
		// TODO: Could also be an error in extracting the file.
		printf("not found\n");
		return -1; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}

	// File extracted successfully.
	return 0; // TODO: return MDP_ERR_OK;
}

}
