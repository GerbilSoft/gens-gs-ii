/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * LzmaSdk.hpp: LZMA SDK common functions.                                 *
 *                                                                         *
 * Do NOT use this class directly. It's intended to be used by LZMA-based  *
 * archive handlers, since they share a lot of boilerplate code for        *
 * interfacing with the LZMA SDK.                                          *
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

#include "LzmaSdk.hpp"

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
#include "lzma/7zCrc.h"
#include "lzma/XzCrc64.h"

// Character set conversion.
#include "libgenstext/Encoding.hpp"

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif

namespace LibGensFile {

// Set to true if the LZMA SDK CRC tables have been initialized.
bool LzmaSdk::ms_CrcInit = false;

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
LzmaSdk::LzmaSdk(const char *filename)
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

	// Subclass MUST call lzmaInit() after checking magic.
}

/**
 * Initialize the LZMA SDK.
 * This function MUST be called by subclasses after
 * checking for file magic.
 *
 * @return 0 on success; negative POSIX error code on error.
 * If an error occurs, the file will NOT be closed;
 * the caller must handle this. In addition, m_lastError
 * is not set, since this is an internal function.
 */
int LzmaSdk::lzmaInit(void)
{
	SRes res;
#ifdef _WIN32
	// Convert the filename from UTF-8 to UTF-16.
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t should be 16-bit on Windows, but it isn't.");
	u16string filenameW = LibGensText::Utf8_to_Utf16(m_filename.data(), m_filename.size());
	if (filenameW.empty()) {
		// Error converting the filename to UTF-16.
		return -EINVAL; // TODO: Figure out an MDP error code for this.
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
			return -EINVAL; // TODO: Figure out an MDP error code for this.
		}
		res = InFile_Open(&m_archiveStream.file, filenameA);
		free(filenameA);
	}
#else
	// Unix system. Use UTF-8 filenames directly.
	res = InFile_Open(&m_archiveStream.file, m_filename.c_str());
#endif

	if (res != 0) {
		// Error opening the file.
		// TODO: Error code?
		return -EIO;
	}

	// Initialize VTables.
	FileInStream_CreateVTable(&m_archiveStream);
	LookToRead_CreateVTable(&m_lookStream, false);

	m_lookStream.realStream = &m_archiveStream.s;
	LookToRead_Init(&m_lookStream);

	// Generate the CRC tables.
	if (!ms_CrcInit) {
		CrcGenerateTable();
		Crc64GenerateTable();
		ms_CrcInit = true;
	}

	// LZMA SDK is initialized.
	// Subclass must open the archive using Sz, Xz, or LZMA functions.
	return 0;
}

/**
 * Delete the LzmaSdk object.
 */
LzmaSdk::~LzmaSdk()
{
	// Close the file.
	if (m_file) {
		File_Close(&m_archiveStream.file);
	}
}

/**
 * Close the archive file.
 */
void LzmaSdk::close(void)
{
	if (m_file) {
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
int LzmaSdk::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// NOTE: This MUST be implemented by the subclass.
	((void)z_entry_out);

	m_lastError = ENOSYS;
	return -m_lastError;	// TODO: MDP error code?
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
int LzmaSdk::readFile(const mdp_z_entry_t *z_entry,
		      file_offset_t start_pos, file_offset_t read_len,
		      void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
	// NOTE: This MUST be implemented by the subclass.
	((void)z_entry);
	((void)start_pos);
	((void)read_len);
	((void)buf);
	((void)siz);
	((void)ret_siz);

	m_lastError = ENOSYS;
	return -m_lastError;	// TODO: MDP error code?
}

}
