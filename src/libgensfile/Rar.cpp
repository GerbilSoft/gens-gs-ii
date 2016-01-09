/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Rar.hpp: RAR archive handler.                                           *
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

#include "Rar.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// stat()
#include <sys/types.h>
#include <sys/stat.h>

// MSVC lacks some stat macros.
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & _S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & _S_IFMT) == S_IFREG)
#endif

#ifdef _WIN32
// Win32 Unicode Translation Laye#ifdef _WIN32
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"
#endif /* _WIN32 */

// C++ includes.
#include <string>
using std::string;

namespace LibGensFile {

/** Static class variable initialization. **/

// UnRAR.dll filename.
const char Rar::m_unrarDll_filename[] =
#ifdef _WIN32
#if defined(__amd64__) || defined(__x86_64__)
	"unrar64.dll";
#else
	"unrar.dll";
#endif
#else /* !_WIN32 */
	"libgensunrar.so";
#endif

/**
 * Open a file with this archive handler.
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 * @param filename Name of the file to open.
 */
Rar::Rar(const char *filename)
	: Archive(filename)
{
	if (!m_file)
		return;

	// Load UnRAR.dll.
	// TODO: Set an error flag if it can't be loaded.
	m_unrarDll.load(m_unrarDll_filename);
	if (!m_unrarDll.isLoaded()) {
		// Error loading UnRAR.dll.
		// TODO: Find the actual DLL error.
		fclose(m_file);
		m_file = nullptr;
		// TODO: Error code?
		m_lastError = EIO;
		return;
	}

	// Check if UnRAR.dll can open the file.
	// TODO: Separate function to open the file.
	// For now, just check for RAR magic.
	static const uint8_t rar_magic[] = {'R', 'a', 'r', '!'};
	uint8_t header[sizeof(rar_magic)];

	bool is_rar = false;
	fseeko(m_file, 0, SEEK_SET);
	size_t ret = fread(&header, 1, sizeof(header), m_file);
	if (ret == sizeof(header)) {
		if (!memcmp(header, rar_magic, sizeof(header))) {
			// Header matches.
			is_rar = true;
		} else {
			// TODO: Better error code?
			m_lastError = EIO;
		}
	} else {
		// Error reading from the file.
		m_lastError = errno;
		if (m_lastError == 0) {
			// Unknown error...
			m_lastError = EIO;
		}
	}

	if (!is_rar) {
		// Not a RAR archive.
		m_unrarDll.unload();
	}
}

/**
 * Delete the Rar object.
 */
Rar::~Rar()
{
	// Unload UnRAR.dll.
	m_unrarDll.unload();
}

/**
 * Close the archive file.
 */
void Rar::close(void)
{
	// Unload UnRAR.dll.
	m_unrarDll.unload();

	// Base class closes the FILE*.
	Archive::close();
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int Rar::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_file) {
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_unrarDll.isLoaded()) {
		// UnRAR.dll is not loaded.
		// TODO: Determine if it was a missing DLL or a missing symbol.
		m_lastError = ENOSYS;	// TODO: Better error?
		return -m_lastError; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}

	// TODO: Open the RAR file in the constructor.
	struct RAROpenArchiveDataEx rar_open;
	memset(&rar_open, 0, sizeof(rar_open));
	rar_open.OpenMode = RAR_OM_LIST;

#ifdef _WIN32
	std::auto_ptr<char> filenameA = nullptr;
	std::auto_ptr<wchar_t> filenameW = nullptr;

	// Convert the filename from UTF-8 to UTF-16 first.
	filenameW = W32U_mbs_to_UTF16(m_filename.c_str(), CP_UTF8);
	if (!filenameW)
		return -EINVAL; // TODO: Figure out an MDP error code for this.

	if (!W32U_IsUnicode()) {
		// System isn't using Unicode.
		// Convert the filename from UTF-16 to ANSI.
		filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		free(filenameW);
		filenameW = nullptr;

		if (!filenameA)
			return -9; // TODO: Figure out an MDP error code for this.
	}

	// Set the archive filename.
	if (W32U_IsUnicode()) {
		// Unicode mode.
		rar_open.ArcNameW = filenameW;
	} else {
		// ANSI mode.
		rar_open.ArcName = filenameA;
	}
#else
	// TODO: Is UTF-16 needed on Linux?
	// Assuming UnRAR.dll handles UTF-8 if the OS does.
	rar_open.ArcName = (char*)m_filename.c_str();
	
#endif

	// Open the RAR file.
	HANDLE hRar = m_unrarDll.pRAROpenArchiveEx(&rar_open);
	if (!hRar) {
		// Error opening the RAR file.
#ifdef _WIN32
		free(filenameA);
		free(filenameW);
#endif
		// TODO: Correct error code?
		m_lastError = EIO;
		return m_lastError; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}

	// List head and tail.
	mdp_z_entry_t *z_entry_head = nullptr;
	mdp_z_entry_t *z_entry_tail = nullptr;

	// Process the archive.
	struct RARHeaderDataEx rar_header;

	// Filenames in a RAR archive have a maximum length of 1,024 characters.
	char utf8_buf[1024*4];
	wchar_t wcs_buf[1024];

	while (m_unrarDll.pRARReadHeaderEx(hRar, &rar_header) == 0) {
		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

		// Store the ROM file information.
		// TODO: What do we do if rar_header.UnpSizeHigh is set (indicating >4 GB)?
		z_entry_cur->filesize = rar_header.UnpSize;
		z_entry_cur->next = nullptr;

		// TODO: Is the ANSI filename UTF-8 on Linux?
		z_entry_cur->filename = strdup(rar_header.FileName);
#if 0
		if (rar_header.FileNameW[0] != 0x00) {
			// Use the Unicode filename. (rar_header.FileNameW)
			// Convert UTF-16 to UTF-8.
			WideCharToMultiByte(CP_UTF8, 0, rar_header.FileNameW, -1,
						utf8_buf, sizeof(utf8_buf), nullptr, nullptr);
			z_entry_cur->filename = strdup(utf8_buf);
		} else {
			// Use the ANSI filename. (rar_header.FileName)
			// TODO: Proper codepage detection.
			// We're using CP_ACP for archives created on MS-DOS, OS/2, or Win32.
			// We're using UTF-8 for archives created on Unix
			if (rar_header.HostOS < 3) {
				// MS-DOS, OS/2, Win32.

				// Convert ANSI to UTF-16.
				MultiByteToWideChar(CP_ACP, 0, rar_header.FileName, -1,
							wcs_buf, ARRAY_SIZE(wcs_buf));
				// Convert UTF-16 to UTF-8.
				WideCharToMultiByte(CP_UTF8, 0, wcs_buf, -1,
							utf8_buf, sizeof(utf8_buf), nullptr, nullptr);

				// Save the filename.
				z_entry_cur->filename = strdup(utf8_buf);
			} else {
				// Unix. Assume the ANSI filename is UTF-8.
				z_entry_cur->filename = strdup(rar_header.FileName);
			}
		}
#endif

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
		if (m_unrarDll.pRARProcessFile(hRar, RAR_SKIP, nullptr, nullptr) != 0)
			break;
	}

	// Close the RAR file.
	m_unrarDll.pRARCloseArchive(hRar);

	// If there are no files in the archive, return an error.
	if (!z_entry_head) {
		// TODO: Better error?
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
int Rar::readFile(const mdp_z_entry_t *z_entry,
		  file_offset_t start_pos, file_offset_t read_len,
		  void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
#if 0
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Make sure the file is open.
	if (!m_file)
		return -2;

	// Check that UnRAR.dll is loaded.
	if (!m_unrarDll.isLoaded()) {
		// Error loading UnRAR.dll.
		// TODO: Determine if it was a missing DLL or a missing symbol.
		return -3; // TODO: return -MDP_ERR_Z_EXE_NOT_FOUND;
	}

	// TODO: Open the RAR file in the constructor.
	HANDLE hRar;
	char *filenameA = nullptr;
	wchar_t *filenameW = nullptr;

	// Convert the filename from UTF-8 to UTF-16 first.
	filenameW = W32U_mbs_to_UTF16(m_filename.c_str(), CP_UTF8);
	if (!filenameW)
		return -9; // TODO: Figure out an MDP error code for this.

	if (!W32U_IsUnicode()) {
		// System doesn't support Unicode.
		// Convert the filename from UTF-16 to ANSI.
		filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		free(filenameW);
		filenameW = nullptr;

		if (!filenameA)
			return -9; // TODO: Figure out an MDP error code for this.
	}

	// Open the RAR file.
	struct RAROpenArchiveDataEx rar_open;
	rar_open.OpenMode = RAR_OM_EXTRACT;
	rar_open.CmtBuf = nullptr;
	rar_open.CmtBufSize = 0;

	if (W32U_IsUnicode()) {
		// Unicode mode.
		rar_open.ArcName = nullptr;
		rar_open.ArcNameW = filenameW;
	} else {
		// ANSI mode.
		rar_open.ArcName = filenameA;
		rar_open.ArcNameW = nullptr;
	}

	hRar = m_unrarDll.pRAROpenArchiveEx(&rar_open);
	if (!hRar) {
		// Error opening the RAR file.
		free(filenameA);
		free(filenameW);
		return -4; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}

	// Convert z_entry->filename to UTF-16.
	// TODO: Move this to another function.
	char *z_filenameA = nullptr;
	wchar_t *z_filenameW = nullptr;

	// Convert the z_entry filename from UTF-8 to UTF-16 first.
	z_filenameW = W32U_mbs_to_UTF16(z_entry->filename, CP_UTF8);
	if (!z_filenameW)
		return -9; // TODO: Figure out an MDP error code for this.

	// Convert the filename from UTF-16 to ANSI in case files don't have a Unicode filename.
	z_filenameA = W32U_UTF16_to_mbs(z_filenameW, CP_ACP);
	if (!z_filenameA)
		return -9; // TODO: Figure out an MDP error code for this.

	// Search for the file.
	struct RARHeaderDataEx rar_header;
	*ret_siz = 0;
	int cmp;
	while (m_unrarDll.pRARReadHeaderEx(hRar, &rar_header) == 0) {
		if (rar_header.FileNameW[0] != 0x00) {
			// Use the Unicode filename.
			cmp = _wcsicmp(z_filenameW, rar_header.FileNameW);
		} else {
			// Use the ANSI filename. (rar_header.FileName)
			// TODO: Proper codepage detection.
			// We're using CP_ACP for archives created on MS-DOS, OS/2, or Win32.
			// We're using UTF-8 for archives created on Unix

			if (rar_header.HostOS < 3) {
				// MS-DOS, OS/2, Win32.
				// Use the ANSI codepage.
				cmp = _stricmp(z_filenameA, rar_header.FileName);
			} else {
				// Unix. Assume the ANSI filename is UTF-8.
				cmp = _stricmp(z_entry->filename, rar_header.FileName);
			}
		}

		if (cmp != 0) {
			// Not a match. Skip the file.
			if (m_unrarDll.pRARProcessFile(hRar, RAR_SKIP, nullptr, nullptr) != 0)
				break;
			continue;
		}

		// Found the file.

		// Create the RAR state.
		RarState_t rar_state;
		rar_state.buf = (uint8_t*)buf;
		rar_state.siz = siz;
		rar_state.pos = 0;
		rar_state.owner = this;

		// Set up the RAR callback.
		m_unrarDll.pRARSetCallback(hRar, &RarCallback, (LPARAM)&rar_state);

		// Process the file.
		// Possible errors:
		// - 0: Success.
		// - ERAR_UNKNOWN: Read the maximum amount of data for the ubuffer.
		// - Others: Read error; abort. (TODO: Show an error message.)
		int ret = m_unrarDll.pRARProcessFile(hRar, RAR_TEST, nullptr, nullptr);

		// TODO: DcRar.cpp returns the filesize processed on error.
		// This just returns 0.
		if (ret != 0 && ret != ERAR_UNKNOWN)
			break;

		// File processed.
		*ret_siz = rar_state.pos;
		break;
	}

	// Close the RAR file.
	m_unrarDll.pRARCloseArchive(hRar);
	free(filenameA);
	free(filenameW);
	free(z_filenameA);
	free(z_filenameW);

	// File extracted successfully.
	return 0; // TODO: return MDP_ERR_OK;
#endif
}

/**
 * Win32 UnRAR.dll callback function. [STATIC]
 */
int CALLBACK Rar::RarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
{
	RarState_t *pRarState = (RarState_t*)UserData;

	switch (msg) {
		default:
		case UCM_CHANGEVOLUME:
		case UCM_NEEDPASSWORD:
			// Unhandled message.
			// TODO: Support at least UCM_NEEDPASSWORD.
			return -1;

		case UCM_CHANGEVOLUMEW:
		case UCM_NEEDPASSWORDW:
			// Unicode versions of the above unhandled messages.
			// TODO: Support at least UCM_NEEDPASSWORDW.
			return 1;

		case UCM_PROCESSDATA: {
			// Process data.
			const uint8_t *buf = (const uint8_t*)P1;
			size_t siz = (size_t)P2;

			// FIXME: Use subtraction to check for overflow.
			if ((pRarState->pos + siz) > pRarState->siz) {
				// Overflow!
				size_t siz_diff = (pRarState->siz - pRarState->pos);
				memcpy(&pRarState->buf[pRarState->pos], buf, siz_diff);
				pRarState->pos += siz_diff;
				return -1;
			}

			// Copy the data.
			memcpy(&pRarState->buf[pRarState->pos], buf, siz);
			pRarState->pos += siz;
			break;
		}
	}

	return 0;
}

#if 0
// TODO: Port to Archive.

/**
 * Check if the specified external RAR program is usable.
 * @param extprg	[in] External RAR program filename.
 * @param prg_info	[out] If not nullptr, contains RAR/UnRAR version information.
 * @return Possible error codes:
 * -  0: Program is usable.
 * - -1: File not found.
 * - -2: File isn't executable
 * - -3: File isn't a regular file. (e.g. it's a directory)
 * - -4: Error calling stat().
 * - -5: Wrong DLL API version. (Win32 only)
 * - -6: Version information not found.
 * - -7: Not RAR, UnRAR, or UnRAR.dll.
 * TODO: Use MDP error code constants.
 */
uint32_t DcRar::CheckExtPrg(const char *extprg, ExtPrgInfo *prg_info)
{
	// Program information.
	ExtPrgInfo my_prg_info;
	memset(&my_prg_info, 0x00, sizeof(my_prg_info));
	if (prg_info)
		memset(prg_info, 0x00, sizeof(*prg_info));

	// Check that the UnRAR DLL is available.
	if (access(extprg, F_OK) != 0)
		return -1;

	// Make sure that this is a regular file.
	struct _stati64 st_buf;
	if (stat(extprg, &st_buf) != 0)
		return -4;
	if (!S_ISREG(st_buf.st_mode))
		return -3;

	// Attempt to load the DLL manually.
	// TODO: Use W32U?
	HINSTANCE hDll = LoadLibrary(extprg);
	if (!hDll)
		return -2;
	FreeLibrary(hDll);

	// Get the UnRAR DLL version information.
	DWORD dwLen;
	void *lpData, *lpVerInfo;
	unsigned int lenVerInfo;

	// TODO: Do we need Unicode translation for VerQueryValue?
	// Alternatively, just use the "A" functions.
	dwLen = GetFileVersionInfoSize(extprg, nullptr);
	if (dwLen == 0)
		return -6;
	lpData = malloc(dwLen);
	if (!GetFileVersionInfo(extprg, 0, dwLen, lpData)) {
		free(lpData);
		return -6;
	}
	if (!VerQueryValue(lpData, "\\", &lpVerInfo, &lenVerInfo)) {
		free(lpData);
		return -6;
	}

	// Get the file version.
	VS_FIXEDFILEINFO *lpFixedFileInfo = (VS_FIXEDFILEINFO*)lpVerInfo;
	my_prg_info.dll_major    = (lpFixedFileInfo->dwFileVersionMS >> 16);
	my_prg_info.dll_minor    = (lpFixedFileInfo->dwFileVersionMS & 0xFFFF);
	my_prg_info.dll_revision = (lpFixedFileInfo->dwFileVersionLS >> 16);
	my_prg_info.dll_build    = (lpFixedFileInfo->dwFileVersionLS & 0xFFFF);
	free(lpData);

	// Attempt to load the DLL.
	UnRAR_dll dll;
	dll.load(extprg);
	if (!dll.isLoaded()) {
		// Error loading the DLL.
		// TODO: Determine what the error is.
		// Assume that it's not UnRAR.dll for now.
		// (It could also be that the DLL is too old and is missing a function.)
		if (prg_info)
			memcpy(prg_info, &my_prg_info, sizeof(*prg_info));
		return -7;
	}

	// Get the UnRAR DLL API version.
	my_prg_info.api_version = dll.pRARGetDllVersion();
	if (my_prg_info.api_version < RAR_DLL_VERSION) {
		// DLL is too old.
		if (prg_info)
			memcpy(prg_info, &my_prg_info, sizeof(*prg_info));
		return -5;
	}

	// This is UnRAR.dll.
	my_prg_info.rar_type = ExtPrgInfo::RAR_ET_UNRAR_DLL;

	// RAR version obtained.
	if (prg_info)
		memcpy(prg_info, &my_prg_info, sizeof(*prg_info));

	// RAR is usable.
	return 0;
}
#endif

}
